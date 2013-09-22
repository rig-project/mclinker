//===- MipsELFDynamic.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <llvm/Support/ELF.h>
#include <mcld/LinkerConfig.h>
#include <mcld/LD/ELFFileFormat.h>
#include <mcld/LD/ELFSegment.h>
#include <mcld/LD/ELFSegmentFactory.h>
#include <mcld/Target/GNULDBackend.h>
#include "MipsELFDynamic.h"
#include "MipsLDBackend.h"

using namespace mcld;


// MIPS mandatory dynamic section entries
enum {
  DT_MIPS_RLD_VERSION  = 0x70000001,
  DT_MIPS_FLAGS        = 0x70000005,
  DT_MIPS_BASE_ADDRESS = 0x70000006,
  DT_MIPS_LOCAL_GOTNO  = 0x7000000a,
  DT_MIPS_SYMTABNO     = 0x70000011,
  DT_MIPS_GOTSYM       = 0x70000013,
  DT_MIPS_PLTGOT       = 0x70000032,
};

// Dynamic section MIPS flags
enum {
  RHF_NONE                   = 0x00000000,  // None
  RHF_QUICKSTART             = 0x00000001,  // Use shortcut pointers
  RHF_NOTPOT                 = 0x00000002,  // Hash size not power of two
  RHF_NO_LIBRARY_REPLACEMENT = 0x00000004   // Ignore LD_LIBRARY_PATH
};

MipsELFDynamic::MipsELFDynamic(const MipsGNULDBackend& pParent,
                               const LinkerConfig& pConfig)
  : ELFDynamic(pParent, pConfig),
    m_pParent(pParent),
    m_pConfig(pConfig)
{
}

void MipsELFDynamic::reserveTargetEntries(const ELFFileFormat& pFormat)
{
  if (pFormat.hasGOT())
    reserveOne(llvm::ELF::DT_PLTGOT);

  reserveOne(DT_MIPS_RLD_VERSION);
  reserveOne(DT_MIPS_FLAGS);
  reserveOne(DT_MIPS_BASE_ADDRESS);
  reserveOne(DT_MIPS_LOCAL_GOTNO);
  reserveOne(DT_MIPS_SYMTABNO);
  reserveOne(DT_MIPS_GOTSYM);

  if (pFormat.hasGOTPLT())
    reserveOne(DT_MIPS_PLTGOT);
}

void MipsELFDynamic::applyTargetEntries(const ELFFileFormat& pFormat)
{
  if (pFormat.hasGOT())
    applyOne(llvm::ELF::DT_PLTGOT, pFormat.getGOT().addr());

  applyOne(DT_MIPS_RLD_VERSION, 1);
  applyOne(DT_MIPS_FLAGS, RHF_NOTPOT);
  applyOne(DT_MIPS_BASE_ADDRESS, getBaseAddress());
  applyOne(DT_MIPS_LOCAL_GOTNO, getLocalGotNum(pFormat));
  applyOne(DT_MIPS_SYMTABNO, getSymTabNum(pFormat));
  applyOne(DT_MIPS_GOTSYM, getGotSym(pFormat));

  if (pFormat.hasGOTPLT())
    applyOne(DT_MIPS_PLTGOT, pFormat.getGOTPLT().addr());
}

size_t MipsELFDynamic::getSymTabNum(const ELFFileFormat& pFormat) const
{
  if (!pFormat.hasDynSymTab())
    return 0;

  const LDSection& dynsym = pFormat.getDynSymTab();
  return dynsym.size() / symbolSize();
}

size_t MipsELFDynamic::getGotSym(const ELFFileFormat& pFormat) const
{
  if (!pFormat.hasGOT())
    return 0;

  return getSymTabNum(pFormat) - m_pParent.getGOT().getGlobalNum();
}

size_t MipsELFDynamic::getLocalGotNum(const ELFFileFormat& pFormat) const
{
  if (!pFormat.hasGOT())
    return 0;

  return m_pParent.getGOT().getLocalNum();
}

uint64_t MipsELFDynamic::getBaseAddress()
{
  if (LinkerConfig::Exec != m_pConfig.codeGenType())
    return 0;

  ELFSegmentFactory::const_iterator baseSeg =
    m_pParent.elfSegmentTable().find(llvm::ELF::PT_LOAD, 0x0, 0x0);

  return m_pParent.elfSegmentTable().end() == baseSeg ? 0 : (*baseSeg)->vaddr();
}
