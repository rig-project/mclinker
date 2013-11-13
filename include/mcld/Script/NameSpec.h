//===- NameSpec.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_NAMESPEC_TOKEN_INTERFACE_H
#define MCLD_SCRIPT_NAMESPEC_TOKEN_INTERFACE_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <mcld/Script/InputToken.h>
#include <mcld/Support/Allocators.h>
#include <mcld/Config/Defines.h>

namespace mcld
{

/** \class NameSpec
 *  \brief This class defines the interfaces to a namespec in INPUT/GROUP
 *         command.
 */

class NameSpec : public InputToken
{
private:
  friend class Chunk<NameSpec, MCLD_SYMBOLS_PER_INPUT>;
  NameSpec();
  NameSpec(const std::string& pName, bool pAsNeeded);

public:
  ~NameSpec();

  static bool classof(const InputToken* pToken)
  {
    return pToken->type() == InputToken::NameSpec;
  }

  /* factory method */
  static NameSpec* create(const std::string& pName, bool pAsNeeded);
  static void destroy(NameSpec*& pToken);
  static void clear();
};

} // namepsace of mcld

#endif
