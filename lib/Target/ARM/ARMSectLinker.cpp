//===- ARMSectLinker.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <llvm/ADT/Triple.h>
#include <mcld/Support/TargetRegistry.h>
#include "ARM.h"
#include "ARMAndroidSectLinker.h"
#include "ARMELFSectLinker.h"


using namespace mcld;

namespace mcld {
//===----------------------------------------------------------------------===//
/// createARMSectLinker - the help funtion to create corresponding ARMSectLinker
///
SectLinker* createARMSectLinker(const std::string &pTriple,
                                SectLinkerOption &pOption,
                                const llvm::cl::opt<std::string> &pInputFilename,
                                const std::string &pOutputFilename,
                                unsigned int pOutputLinkType,
                                mcld::TargetLDBackend &pLDBackend)
{
  Triple theTriple(pTriple);
  if (theTriple.isOSDarwin()) {
    assert(0 && "MachO linker has not supported yet");
  }
  if (theTriple.isOSWindows()) {
    assert(0 && "COFF linker has not supported yet");
  }

  // For now, use Android SectLinker directly
  return new ARMAndroidSectLinker(pOption,
                                  pInputFilename,
                                  pOutputFilename,
                                  pOutputLinkType,
                                  pLDBackend);
}

} // namespace of mcld

//==========================
// ARMSectLinker
extern "C" void LLVMInitializeARMSectLinker() {
  // Register the linker frontend
  mcld::TargetRegistry::RegisterSectLinker(TheARMTarget, createARMSectLinker);
}

