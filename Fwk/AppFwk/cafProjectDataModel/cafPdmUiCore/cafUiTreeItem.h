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

//#include <QAbstractItemModel>
#include <QList>
#include <iostream>

namespace caf
{
//==================================================================================================
/// Class used to build a tree item holding a data object
//==================================================================================================
template <typename T>
class UiTreeItem
{
public:
    // Create a new item at a specified position in parent child list. "-1" is understood as append.
    UiTreeItem( UiTreeItem* parent, int position, T dataObject )
    {
        m_parentItem = parent;

        if ( m_parentItem )
        {
            if ( position < 0 )
                m_parentItem->m_childItems.push_back( this );
            else
                m_parentItem->m_childItems.insert( position, this );
        }

        setDataObject( dataObject );
    }

    virtual ~UiTreeItem() { qDeleteAll( m_childItems ); }

    UiTreeItem* child( int row ) const
    {
        CAF_ASSERT( row < m_childItems.size() );
        return m_childItems.value( row );
    }

    bool hasGrandChildren() const
    {
        for ( auto child : m_childItems )
        {
            if ( child->childCount() != 0 )
            {
                return true;
            }
        }
        return false;
    }

    int childCount() const { return m_childItems.count(); }

    int columnCount() const { return 1; }

    int row() const
    {
        if ( m_parentItem ) return m_parentItem->m_childItems.indexOf( const_cast<UiTreeItem*>( this ) );

        return 0;
    }

    UiTreeItem* parent() { return m_parentItem; }

    T dataObject() const { return m_dataObject; }

    void setDataObject( const T& dataObject ) { m_dataObject = dataObject; }

    void appendChild( UiTreeItem* child )
    {
        m_childItems.append( child );
        child->m_parentItem = this;
    }

    void insertChild( int position, UiTreeItem* child )
    {
        m_childItems.insert( position, child );
        child->m_parentItem = this;
    }

    bool removeChildren( int position, int count )
    {
        if ( position < 0 || position + count > m_childItems.size() ) return false;

        for ( int row = 0; row < count; ++row )
        {
            UiTreeItem* uiItem = m_childItems.takeAt( position );

            delete uiItem;
        }

        return true;
    }

    bool removeChildrenNoDelete( int position, int count )
    {
        if ( position < 0 || position + count > m_childItems.size() ) return false;

        for ( int row = 0; row < count; ++row )
        {
            m_childItems.removeAt( position );
        }
        return true;
    }

    void removeAllChildrenNoDelete() { m_childItems.clear(); }

    // Returns the index of the first child with dataObject() == dataObject. -1 if not found
    int findChildItemIndex( const T& dataObject )
    {
        for ( int i = 0; i < m_childItems.size(); ++i )
        {
            if ( m_childItems[i]->dataObject() == dataObject )
            {
                return i;
            }
        }
        return -1;
    }

private:
    QList<UiTreeItem*> m_childItems;
    UiTreeItem*        m_parentItem;
    T                  m_dataObject;
};

} // End of namespace caf
