//===- Linker.cpp ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <mcld/Linker.h>
#include <mcld/LinkerConfig.h>
#include <mcld/Module.h>
#include <mcld/IRBuilder.h>

#include <mcld/Support/MsgHandling.h>
#include <mcld/Support/TargetRegistry.h>
#include <mcld/Support/FileHandle.h>
#include <mcld/Support/MemoryArea.h>
#include <mcld/Support/raw_ostream.h>

#include <mcld/Object/ObjectLinker.h>
#include <mcld/MC/InputBuilder.h>
#include <mcld/Target/TargetLDBackend.h>
#include <mcld/LD/LDSection.h>
#include <mcld/LD/LDSymbol.h>
#include <mcld/LD/SectionData.h>
#include <mcld/LD/RelocData.h>
#include <mcld/Fragment/Relocation.h>
#include <mcld/Fragment/FragmentRef.h>

#include <cassert>

using namespace mcld;

Linker::Linker()
  : m_pConfig(NULL), m_pIRBuilder(NULL),
    m_pTarget(NULL), m_pBackend(NULL), m_pObjLinker(NULL) {
}

Linker::~Linker()
{
  reset();
}

bool Linker::config(LinkerConfig& pConfig)
{
  m_pConfig = &pConfig;

  if (!initTarget())
    return false;

  if (!initBackend())
    return false;

  m_pObjLinker = new ObjectLinker(*m_pConfig,
                                  *m_pBackend);

  if (!initEmulator())
    return false;

  if (!initOStream())
    return false;

  return true;
}

bool Linker::link(Module& pModule, IRBuilder& pBuilder)
{
  if (!resolve(pModule, pBuilder))
    return false;

  return layout();
}

bool Linker::resolve(Module& pModule, IRBuilder& pBuilder)
{
  assert(NULL != m_pConfig);

  m_pIRBuilder = &pBuilder;
  assert(m_pObjLinker!=NULL);
  m_pObjLinker->setup(pModule, pBuilder);

  // 2. - initialize FragmentLinker
  if (!m_pObjLinker->initFragmentLinker())
    return false;

  // 3. - initialize output's standard sections
  if (!m_pObjLinker->initStdSections())
    return false;

  if (!Diagnose())
    return false;

  // 4. - normalize the input tree
  //   => read out sections and symbol/string tables (from the files) and set them in Module.
  //   => Symbol resolution: when reading out the symbol, resolve them immediately and set their ResolveInfo
  m_pObjLinker->normalize();

  if (m_pConfig->options().trace()) {
    static int counter = 0;
    mcld::outs() << "** name\ttype\tpath\tsize (" << pModule.getInputTree().size() << ")\n";
    InputTree::const_dfs_iterator input, inEnd = pModule.getInputTree().dfs_end();
    for (input=pModule.getInputTree().dfs_begin(); input!=inEnd; ++input) {
      mcld::outs() << counter++ << " *  " << (*input)->name();
      switch((*input)->type()) {
      case Input::Archive:
        mcld::outs() << "\tarchive\t(";
        break;
      case Input::Object:
        mcld::outs() << "\tobject\t(";
        break;
      case Input::DynObj:
        mcld::outs() << "\tshared\t(";
        break;
      case Input::Script:
        mcld::outs() << "\tscript\t(";
        break;
      case Input::External:
        mcld::outs() << "\textern\t(";
        break;
      default:
        unreachable(diag::err_cannot_trace_file) << (*input)->type()
                                                 << (*input)->name()
                                                 << (*input)->path();
      }
      mcld::outs() << (*input)->path() << ")\n";
    }
  }

  if (!m_pObjLinker->linkable())
    return Diagnose();

  // 5. - set up code position
  if (LinkerConfig::DynObj == m_pConfig->codeGenType() ||
      m_pConfig->options().isPIE()) {
    m_pConfig->setCodePosition(LinkerConfig::Independent);
  }
  else if (m_pConfig->options().nmagic() || m_pConfig->options().omagic()) {
    // --nmagic and --omagic options lead to static executable program.
    // These options turn off page alignment of sections. Because the
    // sections are not aligned to pages, these sections can not contain any
    // exported functions. Also, because the two options disable linking
    // against shared libraries, the output absolutely does not call outside
    // functions.
    m_pConfig->setCodePosition(LinkerConfig::StaticDependent);
  }
  else if (pModule.getLibraryList().empty()) {
    // If the output is dependent on its loaded address, and it does not need
    // to call outside functions, then we can treat the output static dependent
    // and perform better optimizations.
    m_pConfig->setCodePosition(LinkerConfig::StaticDependent);
  }
  else {
    m_pConfig->setCodePosition(LinkerConfig::DynamicDependent);
  }

  // 6. - read all relocation entries from input files
  //   => for all relocation sections of each input file (in the tree), read out reloc entry info
  //    from the object file and accordingly init their reloc entries in SectOrRelocData of LDSection
  m_pObjLinker->readRelocations();

  // 7. - merge all sections
  //   => push sections into Module's SectionTable. Merge sections that have the same name.
  //     Maintain them as fragments in the section
  if (!m_pObjLinker->mergeSections())
    return false;

  // 8. - allocateCommonSymbols
  //   => allocate fragments for common symbols to the corresponding sections
  if (!m_pObjLinker->allocateCommonSymbols())
    return false;
  return true;
}

bool Linker::layout()
{
  assert(NULL != m_pConfig && NULL != m_pObjLinker);

  // 9. - add standard symbols and target-dependent symbols
  // m_pObjLinker->addUndefSymbols();
  if (!m_pObjLinker->addStandardSymbols() ||
      !m_pObjLinker->addTargetSymbols())
    return false;

  // 10. - scan all relocation entries by output symbols.
  //   reserve GOT space for layout.
  //   the space info is needed by pre-layout to compute the section size
  m_pObjLinker->scanRelocations();

  // 11.a - init relaxation stuff.
  m_pObjLinker->initStubs();

  // 11.b - pre-layout
  m_pObjLinker->prelayout();

  // 11.c - linear layout
  //   Decide which sections will be left in. Sort the sections according to
  //   a given order. Then, create program header accordingly.
  //   Finally, set the offset for sections (@ref LDSection)
  //   according to the new order.
  m_pObjLinker->layout();

  // 11.d - post-layout (create segment, instruction relaxing)
  m_pObjLinker->postlayout();

  // 12. - finalize symbol value
  m_pObjLinker->finalizeSymbolValue();

  // 13. - apply relocations
  m_pObjLinker->relocation();

  if (!Diagnose())
    return false;
  return true;
}

bool Linker::emit(MemoryArea& pOutput)
{
  // 13. - write out output
  m_pObjLinker->emitOutput(pOutput);

  // 14. - post processing
  m_pObjLinker->postProcessing(pOutput);

  if (!Diagnose())
    return false;

  return true;
}

bool Linker::emit(const std::string& pPath)
{
  FileHandle file;
  FileHandle::Permission perm = 0755;
  if (!file.open(pPath,
            FileHandle::ReadWrite | FileHandle::Truncate | FileHandle::Create,
            perm)) {
    error(diag::err_cannot_open_output_file) << "Linker::emit()" << pPath;
    return false;
  }

  MemoryArea* output = new MemoryArea(file);

  bool result = emit(*output);

  delete output;
  file.close();
  return result;
}

bool Linker::emit(int pFileDescriptor)
{
  FileHandle file;
  file.delegate(pFileDescriptor);
  MemoryArea* output = new MemoryArea(file);

  bool result = emit(*output);

  delete output;
  file.close();
  return result;
}

bool Linker::reset()
{
  m_pConfig = NULL;
  m_pIRBuilder = NULL;
  m_pTarget = NULL;

  // Because llvm::iplist will touch the removed node, we must clear
  // RelocData before deleting target backend.
  RelocData::Clear();
  SectionData::Clear();
  EhFrame::Clear();

  delete m_pBackend;
  m_pBackend = NULL;

  delete m_pObjLinker;
  m_pObjLinker = NULL;

  LDSection::Clear();
  LDSymbol::Clear();
  FragmentRef::Clear();
  Relocation::Clear();
  return true;
}

bool Linker::initTarget()
{
  assert(NULL != m_pConfig);

  std::string error;
  m_pTarget = mcld::TargetRegistry::lookupTarget(m_pConfig->targets().triple().str(), error);
  if (NULL == m_pTarget) {
    fatal(diag::fatal_cannot_init_target) << m_pConfig->targets().triple().str() << error;
    return false;
  }
  return true;
}

bool Linker::initBackend()
{
  assert(NULL != m_pTarget);
  m_pBackend = m_pTarget->createLDBackend(*m_pConfig);
  if (NULL == m_pBackend) {
    fatal(diag::fatal_cannot_init_backend) << m_pConfig->targets().triple().str();
    return false;
  }
  return true;
}

bool Linker::initEmulator()
{
  assert(NULL != m_pTarget && NULL != m_pConfig);
  bool result = m_pTarget->emulate(m_pConfig->targets().triple().str(),
                                   *m_pConfig);

  // Relocation should be set up after emulation.
  Relocation::SetUp(*m_pConfig);
  return result;
}

bool Linker::initOStream()
{
  assert(NULL != m_pConfig);

  mcld::outs().setColor(m_pConfig->options().color());
  mcld::errs().setColor(m_pConfig->options().color());

  return true;
}

