//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "gtest/gtest.h"

#include "cafUiTreeManager.h"

//--------------------------------------------------------------------------------------------------
/// Test basic tree creation and management
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, BasicTreeOperations )
{
    caf::UiTreeManager<int> treeManager;

    // Test empty tree
    EXPECT_EQ( -1, treeManager.getRootIndex() );
    EXPECT_FALSE( treeManager.isValidIndex( 0 ) );
    EXPECT_FALSE( treeManager.isValidIndex( -1 ) );

    // Create root node
    int rootIndex = treeManager.createRootNode( 100 );
    EXPECT_GE( rootIndex, 0 );
    EXPECT_EQ( rootIndex, treeManager.getRootIndex() );
    EXPECT_TRUE( treeManager.isValidIndex( rootIndex ) );

    // Test root node properties
    EXPECT_EQ( 100, treeManager.getNodeData( rootIndex ) );
    EXPECT_EQ( -1, treeManager.getParentIndex( rootIndex ) );
    EXPECT_EQ( 0, treeManager.getChildCount( rootIndex ) );
    EXPECT_EQ( 0, treeManager.getRow( rootIndex ) );

    // Create child nodes
    int child1 = treeManager.createNode( 200, rootIndex );
    int child2 = treeManager.createNode( 300, rootIndex );

    EXPECT_TRUE( treeManager.isValidIndex( child1 ) );
    EXPECT_TRUE( treeManager.isValidIndex( child2 ) );

    // Test parent-child relationships
    EXPECT_EQ( rootIndex, treeManager.getParentIndex( child1 ) );
    EXPECT_EQ( rootIndex, treeManager.getParentIndex( child2 ) );
    EXPECT_EQ( 2, treeManager.getChildCount( rootIndex ) );

    // Test child access
    EXPECT_EQ( child1, treeManager.getChildIndex( rootIndex, 0 ) );
    EXPECT_EQ( child2, treeManager.getChildIndex( rootIndex, 1 ) );
    EXPECT_EQ( -1, treeManager.getChildIndex( rootIndex, 2 ) ); // Out of bounds

    // Test row positions
    EXPECT_EQ( 0, treeManager.getRow( child1 ) );
    EXPECT_EQ( 1, treeManager.getRow( child2 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test tree navigation
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, TreeNavigation )
{
    caf::UiTreeManager<int> treeManager;

    // Build tree structure:
    //   root(0)
    //   ├── child1(10)
    //   │   ├── grandchild1(11)
    //   │   └── grandchild2(12)
    //   └── child2(20)
    //       └── grandchild3(21)

    int root        = treeManager.createRootNode( 0 );
    int child1      = treeManager.createNode( 10, root );
    int child2      = treeManager.createNode( 20, root );
    int grandchild1 = treeManager.createNode( 11, child1 );
    int grandchild2 = treeManager.createNode( 12, child1 );
    int grandchild3 = treeManager.createNode( 21, child2 );

    // Test data retrieval
    EXPECT_EQ( 0, treeManager.getNodeData( root ) );
    EXPECT_EQ( 10, treeManager.getNodeData( child1 ) );
    EXPECT_EQ( 20, treeManager.getNodeData( child2 ) );
    EXPECT_EQ( 11, treeManager.getNodeData( grandchild1 ) );
    EXPECT_EQ( 12, treeManager.getNodeData( grandchild2 ) );
    EXPECT_EQ( 21, treeManager.getNodeData( grandchild3 ) );

    // Test parent relationships
    EXPECT_EQ( -1, treeManager.getParentIndex( root ) );
    EXPECT_EQ( root, treeManager.getParentIndex( child1 ) );
    EXPECT_EQ( root, treeManager.getParentIndex( child2 ) );
    EXPECT_EQ( child1, treeManager.getParentIndex( grandchild1 ) );
    EXPECT_EQ( child1, treeManager.getParentIndex( grandchild2 ) );
    EXPECT_EQ( child2, treeManager.getParentIndex( grandchild3 ) );

    // Test child counts
    EXPECT_EQ( 2, treeManager.getChildCount( root ) );
    EXPECT_EQ( 2, treeManager.getChildCount( child1 ) );
    EXPECT_EQ( 1, treeManager.getChildCount( child2 ) );
    EXPECT_EQ( 0, treeManager.getChildCount( grandchild1 ) );
    EXPECT_EQ( 0, treeManager.getChildCount( grandchild2 ) );
    EXPECT_EQ( 0, treeManager.getChildCount( grandchild3 ) );

    // Test hasGrandChildren
    EXPECT_TRUE( treeManager.hasGrandChildren( root ) );
    EXPECT_FALSE( treeManager.hasGrandChildren( child1 ) ); // Children have no children
    EXPECT_FALSE( treeManager.hasGrandChildren( child2 ) ); // Children have no children
    EXPECT_FALSE( treeManager.hasGrandChildren( grandchild1 ) );

    // Test row positions
    EXPECT_EQ( 0, treeManager.getRow( child1 ) );
    EXPECT_EQ( 1, treeManager.getRow( child2 ) );
    EXPECT_EQ( 0, treeManager.getRow( grandchild1 ) );
    EXPECT_EQ( 1, treeManager.getRow( grandchild2 ) );
    EXPECT_EQ( 0, treeManager.getRow( grandchild3 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test data modification
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, DataModification )
{
    caf::UiTreeManager<int> treeManager;

    int root  = treeManager.createRootNode( 100 );
    int child = treeManager.createNode( 200, root );

    // Test data modification
    EXPECT_TRUE( treeManager.setNodeData( root, 999 ) );
    EXPECT_EQ( 999, treeManager.getNodeData( root ) );

    EXPECT_TRUE( treeManager.setNodeData( child, 888 ) );
    EXPECT_EQ( 888, treeManager.getNodeData( child ) );

    // Test invalid index modification
    EXPECT_FALSE( treeManager.setNodeData( -1, 777 ) );
    EXPECT_FALSE( treeManager.setNodeData( 999, 777 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test node removal
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, NodeRemoval )
{
    caf::UiTreeManager<int> treeManager;

    // Build tree structure:
    //   root(0)
    //   ├── child1(10)
    //   │   ├── grandchild1(11)
    //   │   └── grandchild2(12)
    //   └── child2(20)

    int root        = treeManager.createRootNode( 0 );
    int child1      = treeManager.createNode( 10, root );
    int child2      = treeManager.createNode( 20, root );
    int grandchild1 = treeManager.createNode( 11, child1 );
    int grandchild2 = treeManager.createNode( 12, child1 );

    EXPECT_EQ( 2, treeManager.getChildCount( root ) );
    EXPECT_EQ( 2, treeManager.getChildCount( child1 ) );

    // Remove child1 (should also remove grandchildren)
    EXPECT_TRUE( treeManager.removeNode( child1 ) );

    // Verify child1 and grandchildren are invalid
    EXPECT_FALSE( treeManager.isValidIndex( child1 ) );
    EXPECT_FALSE( treeManager.isValidIndex( grandchild1 ) );
    EXPECT_FALSE( treeManager.isValidIndex( grandchild2 ) );

    // Verify root and child2 are still valid
    EXPECT_TRUE( treeManager.isValidIndex( root ) );
    EXPECT_TRUE( treeManager.isValidIndex( child2 ) );
    EXPECT_EQ( 1, treeManager.getChildCount( root ) );
    EXPECT_EQ( child2, treeManager.getChildIndex( root, 0 ) );

    // Test removing root
    EXPECT_TRUE( treeManager.removeNode( root ) );
    EXPECT_FALSE( treeManager.isValidIndex( root ) );
    EXPECT_FALSE( treeManager.isValidIndex( child2 ) );
    EXPECT_EQ( -1, treeManager.getRootIndex() );
}

//--------------------------------------------------------------------------------------------------
/// Test clear operation
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, ClearOperation )
{
    caf::UiTreeManager<int> treeManager;

    int root   = treeManager.createRootNode( 0 );
    int child1 = treeManager.createNode( 10, root );
    int child2 = treeManager.createNode( 20, root );

    EXPECT_TRUE( treeManager.isValidIndex( root ) );
    EXPECT_TRUE( treeManager.isValidIndex( child1 ) );
    EXPECT_TRUE( treeManager.isValidIndex( child2 ) );

    treeManager.clear();

    EXPECT_EQ( -1, treeManager.getRootIndex() );
    EXPECT_FALSE( treeManager.isValidIndex( root ) );
    EXPECT_FALSE( treeManager.isValidIndex( child1 ) );
    EXPECT_FALSE( treeManager.isValidIndex( child2 ) );
    EXPECT_EQ( 0, treeManager.getChildCount( 0 ) ); // Should handle invalid index gracefully
}

//--------------------------------------------------------------------------------------------------
/// Test edge cases and error handling
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, EdgeCases )
{
    caf::UiTreeManager<int> treeManager;

    // Test operations on empty tree
    EXPECT_EQ( 0, treeManager.getNodeData( -1 ) ); // Should return default value
    EXPECT_EQ( 0, treeManager.getNodeData( 999 ) ); // Should return default value
    EXPECT_FALSE( treeManager.setNodeData( -1, 100 ) );
    EXPECT_FALSE( treeManager.setNodeData( 999, 100 ) );
    EXPECT_EQ( -1, treeManager.getParentIndex( -1 ) );
    EXPECT_EQ( -1, treeManager.getParentIndex( 999 ) );
    EXPECT_EQ( -1, treeManager.getChildIndex( -1, 0 ) );
    EXPECT_EQ( -1, treeManager.getChildIndex( 999, 0 ) );
    EXPECT_EQ( 0, treeManager.getChildCount( -1 ) );
    EXPECT_EQ( 0, treeManager.getChildCount( 999 ) );
    EXPECT_EQ( 0, treeManager.getRow( -1 ) );
    EXPECT_EQ( 0, treeManager.getRow( 999 ) );
    EXPECT_FALSE( treeManager.hasGrandChildren( -1 ) );
    EXPECT_FALSE( treeManager.hasGrandChildren( 999 ) );
    EXPECT_FALSE( treeManager.removeNode( -1 ) );
    EXPECT_FALSE( treeManager.removeNode( 999 ) );

    // Create tree and test boundary conditions
    int root  = treeManager.createRootNode( 0 );
    int child = treeManager.createNode( 10, root );

    // Test out-of-bounds child access
    EXPECT_EQ( -1, treeManager.getChildIndex( root, -1 ) );
    EXPECT_EQ( -1, treeManager.getChildIndex( root, 2 ) );
    EXPECT_EQ( -1, treeManager.getChildIndex( child, 0 ) ); // No children

    // Test creating node with invalid parent
    int orphan = treeManager.createNode( 99, 999 ); // Invalid parent index
    EXPECT_TRUE( treeManager.isValidIndex( orphan ) );
    EXPECT_EQ( -1, treeManager.getParentIndex( orphan ) ); // Should be orphaned
}

//--------------------------------------------------------------------------------------------------
/// Test memory reuse after node deletion
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, MemoryReuse )
{
    caf::UiTreeManager<int> treeManager;

    // Create and remove nodes to test slot reuse
    int root   = treeManager.createRootNode( 0 );
    int child1 = treeManager.createNode( 10, root );
    int child2 = treeManager.createNode( 20, root );

    EXPECT_EQ( 2, treeManager.getChildCount( root ) );

    // Remove child1
    EXPECT_TRUE( treeManager.removeNode( child1 ) );
    EXPECT_FALSE( treeManager.isValidIndex( child1 ) );
    EXPECT_EQ( 1, treeManager.getChildCount( root ) );

    // Create new node (should potentially reuse the slot)
    int child3 = treeManager.createNode( 30, root );
    EXPECT_TRUE( treeManager.isValidIndex( child3 ) );
    EXPECT_EQ( 30, treeManager.getNodeData( child3 ) );
    EXPECT_EQ( root, treeManager.getParentIndex( child3 ) );
    EXPECT_EQ( 2, treeManager.getChildCount( root ) );

    // Verify tree integrity
    EXPECT_TRUE( treeManager.isValidIndex( root ) );
    EXPECT_TRUE( treeManager.isValidIndex( child2 ) );
    EXPECT_TRUE( treeManager.isValidIndex( child3 ) );

    // Note: child1 index might be reused by child3, so we can't guarantee it's invalid
    // The important thing is that the tree structure is correct
}

//--------------------------------------------------------------------------------------------------
/// Test with string data type
//--------------------------------------------------------------------------------------------------
TEST( UiTreeManagerTest, StringDataType )
{
    caf::UiTreeManager<std::string> treeManager;

    int root   = treeManager.createRootNode( "root" );
    int child1 = treeManager.createNode( "child1", root );
    int child2 = treeManager.createNode( "child2", root );

    EXPECT_EQ( "root", treeManager.getNodeData( root ) );
    EXPECT_EQ( "child1", treeManager.getNodeData( child1 ) );
    EXPECT_EQ( "child2", treeManager.getNodeData( child2 ) );

    // Test data modification
    EXPECT_TRUE( treeManager.setNodeData( child1, "modified_child1" ) );
    EXPECT_EQ( "modified_child1", treeManager.getNodeData( child1 ) );

    // Test default return value for invalid index
    EXPECT_EQ( std::string(), treeManager.getNodeData( 999 ) ); // Should return empty string
}