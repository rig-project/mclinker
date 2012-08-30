//===- LDSection.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <mcld/LD/LDSection.h>

using namespace mcld;

//===----------------------------------------------------------------------===//
// LDSection
//===----------------------------------------------------------------------===//
LDSection::LDSection()
  : m_Name(),
    m_Kind(LDFileFormat::Ignore),
    m_Type(0x0),
    m_Flag(0x0),
    m_Size(0),
    m_Offset(0),
    m_Addr(0x0),
    m_Align(0),
    m_Info(0),
    m_pLink(NULL),
    m_pSectionData(NULL),
    m_Index(0) {
}

LDSection::LDSection(const std::string& pName,
                     LDFileFormat::Kind pKind,
                     uint32_t pType,
                     uint32_t pFlag,
                     uint64_t pSize,
                     uint64_t pOffset,
                     uint64_t pAddr)
  : m_Name(pName),
    m_Kind(pKind),
    m_Type(pType),
    m_Flag(pFlag),
    m_Size(pSize),
    m_Offset(pOffset),
    m_Addr(pAddr),
    m_Align(0),
    m_Info(0),
    m_pLink(NULL),
    m_pSectionData(NULL),
    m_Index(0) {
}

LDSection::~LDSection()
{
}

