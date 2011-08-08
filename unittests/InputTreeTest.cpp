/*****************************************************************************
 *   Test Suite of The MCLinker Project,                                     *
 *                                                                           *
 *   Copyright (C), 2011 -                                                   *
 *   Embedded and Web Computing Lab, National Taiwan University              *
 *   MediaTek, Inc.                                                          *
 *                                                                           *
 *   MCLDFile <pinronglu@gmail.com>                                          *
 ****************************************************************************/
#include <mcld/MC/MCLDInputTree.h>
#include <InputTreeTest.h>

using namespace mcld;
using namespace mcldtest;


// Constructor can do set-up work for all test here.
InputTreeTest::InputTreeTest()
{
	// create testee. modify it if need
	m_pTestee = new InputTree();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
InputTreeTest::~InputTreeTest()
{
	delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void InputTreeTest::SetUp()
{
}

// TearDown() will be called immediately after each test.
void InputTreeTest::TearDown()
{
}

//==========================================================================//
// Testcases
//



TEST_F( InputTreeTest, Basic_operation ) {
  InputTree::iterator node = m_pTestee->root();

  m_pTestee->insert(node, InputTree::Input, "FileSpec", "path1", InputTree::Downward);

  InputTree::const_iterator const_node = node;

  ASSERT_TRUE(isGroup(node));
  ASSERT_TRUE(isGroup(const_node));
  ASSERT_TRUE(m_pTestee->hasFiles());
  ASSERT_EQ(1, m_pTestee->fileSize());

  --node;

  m_pTestee->enterGroup(node, InputTree::Downward);

  InputTree::const_iterator const_node2 = node;

  ASSERT_FALSE(node.isRoot());

  ASSERT_FALSE(isGroup(node));
  ASSERT_FALSE(isGroup(const_node2));
  ASSERT_TRUE(m_pTestee->hasFiles());
  ASSERT_FALSE(m_pTestee->fileSize()==0);

  ASSERT_TRUE(m_pTestee->size()==2);
}

TEST_F( InputTreeTest, forLoop_TEST ) {
  InputTree::iterator node = m_pTestee->root();

  
  m_pTestee->insert(node, InputTree::Input, "FileSpec", "path1", InputTree::Downward);
  InputTree::const_iterator const_node = node;
  --node;

  for(int i=0 ; i<100 ; ++i) 
  {
    m_pTestee->insert(node, InputTree::Input, "FileSpec", "path1", InputTree::Downward);
    ++node;
  }

  m_pTestee->enterGroup(node, InputTree::Downward);
  --node;

  ASSERT_FALSE(node.isRoot());
  ASSERT_TRUE(isGroup(node));
  ASSERT_TRUE(m_pTestee->hasFiles());
  ASSERT_FALSE(m_pTestee->fileSize()==100);

  ASSERT_TRUE(m_pTestee->size()==102);
}

TEST_F( InputTreeTest, Nesting_Case ) {
  InputTree::iterator node = m_pTestee->root(); 

  for(int i=0 ; i<50 ; ++i) 
  {
    m_pTestee->enterGroup(node, InputTree::Downward);
    --node;

    m_pTestee->insert(node, InputTree::Input, "FileSpec", "path1", InputTree::Afterward);
    ++node;
  }
  
  ASSERT_FALSE(node.isRoot());
  ASSERT_FALSE(isGroup(node));
  ASSERT_TRUE(m_pTestee->hasFiles());
  ASSERT_TRUE(m_pTestee->fileSize()==50);
  ASSERT_TRUE(m_pTestee->size()==100);
}


