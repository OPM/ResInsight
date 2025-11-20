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

#pragma once

#include <algorithm>
#include <vector>

namespace caf
{
//==================================================================================================
/// Index-based tree implementation
//==================================================================================================
template <typename T>
class UiTreeManager
{
public:
    struct TreeNode
    {
        T                data;
        int              parentIndex = -1;
        std::vector<int> childIndices;

        TreeNode() = default;
        TreeNode( T nodeData, int parent = -1 )
            : data( nodeData )
            , parentIndex( parent )
        {
        }

        bool isValid() const { return parentIndex >= -1; }
        bool isRoot() const { return parentIndex == -1; }
        int  childCount() const { return static_cast<int>( childIndices.size() ); }
    };

    UiTreeManager()
        : m_rootNodeIndex( -1 )
    {
    }

    ~UiTreeManager() { clear(); }

    // Tree management
    void clear();
    int  createNode( T data, int parentIndex = -1 );
    int  createRootNode( T data );
    bool removeNode( int nodeIndex );

    // Node data access
    T    getNodeData( int nodeIndex ) const;
    bool setNodeData( int nodeIndex, T data );
    int  getRootIndex() const { return m_rootNodeIndex; }

    // Tree navigation
    int getParentIndex( int nodeIndex ) const;
    int getChildIndex( int nodeIndex, int childRow ) const;
    int getChildCount( int nodeIndex ) const;
    int getRow( int nodeIndex ) const;

    // Tree queries
    bool hasGrandChildren( int nodeIndex ) const;

    // Validation
    bool isValidIndex( int nodeIndex ) const;

private:
    std::vector<TreeNode> m_nodes;
    int                   m_rootNodeIndex;
    std::vector<int>      m_freeIndices; // For reusing deleted node slots
};

//==================================================================================================
/// Template method implementations for UiTreeManager
//==================================================================================================

template <typename T>
void UiTreeManager<T>::clear()
{
    m_nodes.clear();
    m_freeIndices.clear();
    m_rootNodeIndex = -1;
}

template <typename T>
int UiTreeManager<T>::createNode( T data, int parentIndex )
{
    int nodeIndex;

    // Validate parent index - set to -1 if invalid
    int validParentIndex = ( parentIndex >= 0 && isValidIndex( parentIndex ) ) ? parentIndex : -1;

    // Reuse free slot if available
    if ( !m_freeIndices.empty() )
    {
        nodeIndex = m_freeIndices.back();
        m_freeIndices.pop_back();
        m_nodes[nodeIndex] = TreeNode( data, validParentIndex );
    }
    else
    {
        nodeIndex = static_cast<int>( m_nodes.size() );
        m_nodes.emplace_back( data, validParentIndex );
    }

    // Add to parent's children if parent exists and is valid
    if ( validParentIndex >= 0 )
    {
        m_nodes[validParentIndex].childIndices.push_back( nodeIndex );
    }

    return nodeIndex;
}

template <typename T>
int UiTreeManager<T>::createRootNode( T data )
{
    m_rootNodeIndex = createNode( data, -1 );
    return m_rootNodeIndex;
}

template <typename T>
bool UiTreeManager<T>::removeNode( int nodeIndex )
{
    if ( !isValidIndex( nodeIndex ) ) return false;

    TreeNode& node = m_nodes[nodeIndex];

    // Remove from parent's children
    if ( node.parentIndex >= 0 && isValidIndex( node.parentIndex ) )
    {
        auto& parentChildren = m_nodes[node.parentIndex].childIndices;
        parentChildren.erase( std::remove( parentChildren.begin(), parentChildren.end(), nodeIndex ),
                              parentChildren.end() );
    }

    // Recursively remove all children
    for ( int childIndex : node.childIndices )
    {
        removeNode( childIndex );
    }

    // Mark slot as free
    node             = TreeNode(); // Reset to invalid state
    node.parentIndex = -2; // Mark as deleted
    m_freeIndices.push_back( nodeIndex );

    if ( nodeIndex == m_rootNodeIndex )
    {
        m_rootNodeIndex = -1;
    }

    return true;
}

template <typename T>
T UiTreeManager<T>::getNodeData( int nodeIndex ) const
{
    if ( isValidIndex( nodeIndex ) )
    {
        return m_nodes[nodeIndex].data;
    }
    return T{}; // Return default value if invalid
}

template <typename T>
bool UiTreeManager<T>::setNodeData( int nodeIndex, T data )
{
    if ( isValidIndex( nodeIndex ) )
    {
        m_nodes[nodeIndex].data = data;
        return true;
    }
    return false;
}

template <typename T>
int UiTreeManager<T>::getParentIndex( int nodeIndex ) const
{
    if ( isValidIndex( nodeIndex ) )
    {
        return m_nodes[nodeIndex].parentIndex;
    }
    return -1;
}

template <typename T>
int UiTreeManager<T>::getChildIndex( int nodeIndex, int childRow ) const
{
    if ( isValidIndex( nodeIndex ) && childRow >= 0 )
    {
        const auto& childIndices = m_nodes[nodeIndex].childIndices;
        if ( childRow < static_cast<int>( childIndices.size() ) )
        {
            return childIndices[childRow];
        }
    }
    return -1;
}

template <typename T>
int UiTreeManager<T>::getChildCount( int nodeIndex ) const
{
    if ( isValidIndex( nodeIndex ) )
    {
        return static_cast<int>( m_nodes[nodeIndex].childIndices.size() );
    }
    return 0;
}

template <typename T>
int UiTreeManager<T>::getRow( int nodeIndex ) const
{
    if ( !isValidIndex( nodeIndex ) ) return 0;

    int parentIndex = m_nodes[nodeIndex].parentIndex;
    if ( parentIndex < 0 ) return 0;

    if ( !isValidIndex( parentIndex ) ) return 0;

    const auto& parentChildren = m_nodes[parentIndex].childIndices;
    auto        it             = std::find( parentChildren.begin(), parentChildren.end(), nodeIndex );
    return it != parentChildren.end() ? static_cast<int>( std::distance( parentChildren.begin(), it ) ) : 0;
}

template <typename T>
bool UiTreeManager<T>::hasGrandChildren( int nodeIndex ) const
{
    if ( !isValidIndex( nodeIndex ) ) return false;

    const auto& childIndices = m_nodes[nodeIndex].childIndices;
    for ( int childIndex : childIndices )
    {
        if ( getChildCount( childIndex ) > 0 )
        {
            return true;
        }
    }
    return false;
}

template <typename T>
bool UiTreeManager<T>::isValidIndex( int nodeIndex ) const
{
    return nodeIndex >= 0 && nodeIndex < static_cast<int>( m_nodes.size() ) &&
           m_nodes[nodeIndex].parentIndex >= -1; // -2 means deleted
}

} // End of namespace caf
