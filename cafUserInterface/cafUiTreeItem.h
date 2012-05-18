//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include <QAbstractItemModel>
#include <assert.h>


namespace caf
{

//==================================================================================================
/// Class used to build a tree item holding a data object
/// Todo: Needs destructor !!!
//==================================================================================================
template <typename T>
class UiTreeItem
{
public:
    // Create a new item at a specified position in parent child list. "-1" is understood as append.
    UiTreeItem(UiTreeItem* parent, int position, T dataObject)
    {
        m_parentItem = parent;

        if (m_parentItem)
        {
            if (position < 0) m_parentItem->m_childItems.push_back( this);
            else              m_parentItem->m_childItems.insert(position, this);
        }

        setDataObject(dataObject);
    }

    ~UiTreeItem()
    {
        qDeleteAll(m_childItems);
    }

    UiTreeItem* child(int row)
    {
        assert(row < m_childItems.size());
        return m_childItems.value(row);
    }

    int childCount() const
    {
        return m_childItems.count();
    }

    int columnCount() const
    {
        return 1;
    }

    int row() const
    {
        if (m_parentItem)
            return m_parentItem->m_childItems.indexOf(const_cast<UiTreeItem*>(this));

        return 0;
    }

    UiTreeItem* parent()
    {
        return m_parentItem;
    }

    T dataObject()
    {
        return m_dataObject;
    }

    void setDataObject(const T& dataObject)
    {
        m_dataObject = dataObject;
    }

    void appendChild( UiTreeItem* child)
    {
        m_childItems.append(child);
        child->m_parentItem = this;
    }

    bool insertChildren(int position, int count)
    {
        if (position < 0 || position > m_childItems.size())
            return false;

        for (int row = 0; row < count; ++row)
        {
            createChild(this, position + row);
        }

        return true;
    }

    // Create a new Ui tree item at given position pointing to a NULL data object
    virtual UiTreeItem* createChild(UiTreeItem* parent, int position)
    {
        return new UiTreeItem(this, position, NULL);
    }

    bool removeChildren(int position, int count)
    {
        if (position < 0 || position + count > m_childItems.size())
            return false;

        for (int row = 0; row < count; ++row)
        {
            UiTreeItem* uiItem = m_childItems.takeAt(position);

            delete uiItem;
        }

        return true;
    }

    void removeAllChildrenNoDelete()
    {
        m_childItems.clear();
    }

    UiTreeItem* findUiItem(const T& dataObject)
    {
        if (m_dataObject == dataObject) return this;
        int i;
        for (i = 0; i < m_childItems.size(); ++i)
        {
            UiTreeItem* itemFound = m_childItems[i]->findUiItem(dataObject);
            if (itemFound != NULL) return itemFound;
        }
        return NULL;
    }

private:
    QList<UiTreeItem*>  m_childItems;
    UiTreeItem*         m_parentItem;
    T                  m_dataObject;
};



} // End of namespace caf
