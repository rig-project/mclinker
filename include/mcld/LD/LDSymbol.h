//===- LDSymbol.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_SYMBOL_H
#define MCLD_LD_SYMBOL_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <cassert>

#include <mcld/Config/Defines.h>
#include <mcld/ADT/Uncopyable.h>
#include <mcld/LD/ResolveInfo.h>
#include <mcld/Support/Allocators.h>

namespace llvm {

// forware declaration
template<class T> void* object_creator();

} // namespace of llvm

namespace mcld {

class FragmentRef;

/** \class LDSymbol
 *  \brief LDSymbol provides a consistent abstraction for different formats
 *  in different targets.
 */
class LDSymbol
{
public:
  // FIXME: use SizeTrait<32> or SizeTrait<64> instead of big type
  typedef ResolveInfo::SizeType SizeType;
  typedef uint64_t ValueType;

public:
  ~LDSymbol();

  // -----  factory method ----- //
  static LDSymbol* Create(ResolveInfo& pResolveInfo);

  static void Destroy(LDSymbol*& pSymbol);

  /// Clear - This function tells MCLinker to clear all created LDSymbols.
  static void Clear();

  /// NullSymbol() - This returns a reference to a LDSymbol that represents Null
  /// symbol.
  static LDSymbol* Null();

  // -----  observers  ----- //
  bool isNull() const;

  const char* name() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->name();
  }

  unsigned int nameSize() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->nameSize();
  }

  llvm::StringRef str() const {
    assert(NULL != m_pResolveInfo);
    return llvm::StringRef(m_pResolveInfo->name(), m_pResolveInfo->nameSize());
  }

  bool isDyn() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->isDyn();
  }

  unsigned int type() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->type();
  }
 unsigned int desc() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->desc();
  }
  unsigned int binding() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->binding();
  }

  uint8_t other() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->other();
  }

  uint8_t visibility() const {
    assert(NULL != m_pResolveInfo);
    return m_pResolveInfo->other();
  }

  ValueType value() const
  { return m_Value; }

  const FragmentRef* fragRef() const
  { return m_pFragRef; }

  SizeType size() const
  { return m_pResolveInfo->size(); }

  ResolveInfo* resolveInfo()
  { return m_pResolveInfo; }

  const ResolveInfo* resolveInfo() const 
  { return m_pResolveInfo; }

  bool hasFragRef() const;

  // -----  modifiers  ----- //
  void setSize(SizeType pSize) {
    assert(NULL != m_pResolveInfo);
    m_pResolveInfo->setSize(pSize);
  }

  void setValue(ValueType pValue)
  { m_Value = pValue; }
 
  void setFragmentRef(FragmentRef* pFragmentRef);

  void setResolveInfo(const ResolveInfo& pInfo);

private:
  friend class Chunk<LDSymbol, MCLD_SYMBOLS_PER_INPUT>;
  template<class T> friend void* llvm::object_creator();

  LDSymbol();
  LDSymbol(const LDSymbol& pCopy);
  LDSymbol& operator=(const LDSymbol& pCopy);

private:
  // -----  Symbol's fields  ----- //
  ResolveInfo* m_pResolveInfo;
  FragmentRef* m_pFragRef;
  ValueType m_Value;

};

} // namespace mcld

#endif

