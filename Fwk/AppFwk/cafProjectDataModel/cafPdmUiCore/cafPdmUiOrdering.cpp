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

#include "cafPdmUiOrdering.h"

#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiOrdering::~PdmUiOrdering()
{
    for (size_t i = 0; i < m_createdGroups.size(); ++i)
    {
        delete m_createdGroups[i];
        m_createdGroups[i] = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiGroup* PdmUiOrdering::addNewGroup(const QString& displayName, LayoutOptions layout)
{
    PdmUiGroup* group = new PdmUiGroup;
    group->setUiName(displayName);

    m_createdGroups.push_back(group);
    m_ordering.push_back(std::make_pair(group, layout));

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::addNewGroupWithKeyword(const QString& displayName, const QString& keyword, LayoutOptions layout)
{
    PdmUiGroup* group = addNewGroup(displayName, layout);

    group->setKeyword(keyword);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiOrdering::insertBeforeGroup(const QString& groupId, const PdmFieldHandle* field, LayoutOptions layout)
{
   PositionFound pos = findGroupPosition(groupId);
   if (pos.parent)
   {
       pos.parent->insert(pos.indexInParent, field, layout);
       return true;
   }  
   else
   {
        return false;
   }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiOrdering::insertBeforeItem(const PdmUiItem* item, const PdmFieldHandle* field, LayoutOptions layout)
{
    PositionFound pos = findItemPosition(item);
    if (pos.parent)
    {
        pos.parent->insert(pos.indexInParent, field, layout);
        return true;
    }  
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::createGroupBeforeGroup(const QString& groupId, const QString& displayName, LayoutOptions layout)
{
    return createGroupWithIdBeforeGroup(groupId, displayName, "", layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::createGroupBeforeItem(const PdmUiItem* item, const QString& displayName, LayoutOptions layout)
{
    return createGroupWithIdBeforeItem(item, displayName, "", layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::createGroupWithIdBeforeGroup(const QString& groupId, const QString& displayName, const QString& newGroupId, LayoutOptions layout)
{
    PositionFound pos = findGroupPosition(groupId);
    if (pos.parent)
    {
        return pos.parent->insertNewGroupWithKeyword(pos.indexInParent, displayName, newGroupId, layout);
    }

    return nullptr;         
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::createGroupWithIdBeforeItem(const PdmUiItem* item, const QString& displayName, const QString& newGroupId, LayoutOptions layout)
{
    PositionFound pos = findItemPosition(item);
    if (pos.parent)
    {
        return pos.parent->insertNewGroupWithKeyword(pos.indexInParent, displayName, newGroupId, layout);
    }

    return nullptr;         
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::findGroup(const QString& groupId) const
{
    return findGroupPosition(groupId).group();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::insertNewGroupWithKeyword(size_t index, 
                                                          const QString& displayName, 
                                                          const QString& groupKeyword,
                                                          LayoutOptions layout)
{
    PdmUiGroup* group = new PdmUiGroup;
    group->setUiName(displayName);

    m_createdGroups.push_back(group);

    m_ordering.insert(m_ordering.begin() + index, std::make_pair(group, layout));  

    group->setKeyword(groupKeyword);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiOrdering::contains(const PdmUiItem* item) const
{
   return this->findItemPosition(item).parent != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiOrdering::PositionFound PdmUiOrdering::findItemPosition(const PdmUiItem* item) const
{
    for (size_t i = 0; i < m_ordering.size(); ++i)
    {
        if (m_ordering[i].first == item) return { const_cast<PdmUiOrdering*>(this), i};
        if (m_ordering[i].first && m_ordering[i].first->isUiGroup())
        {
            PositionFound result =  static_cast<PdmUiGroup*>(m_ordering[i].first)->findItemPosition(item);
            if (result.parent ) return result;
        }
    }
    return {nullptr, size_t(-1)};
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiOrdering::PositionFound PdmUiOrdering::findGroupPosition(const QString& groupKeyword) const
{
    for (size_t i = 0; i < m_ordering.size(); ++i)
    {
        if (m_ordering[i].first && m_ordering[i].first->isUiGroup())
        {
            if (static_cast<PdmUiGroup*>(m_ordering[i].first)->keyword() == groupKeyword)  return { const_cast<PdmUiOrdering*>(this), i};
            PositionFound result =  static_cast<PdmUiGroup*>(m_ordering[i].first)->findGroupPosition(groupKeyword);
            if (result.parent ) return result;
        }
    }
    return {nullptr,  size_t(-1)};
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::add(const PdmFieldHandle* field, LayoutOptions layout)
{
    PdmUiFieldHandle* uiItem = const_cast<PdmFieldHandle*>(field)->uiCapability();
    CAF_ASSERT(uiItem);
    CAF_ASSERT(!this->contains(uiItem));

    m_ordering.push_back(std::make_pair(uiItem, layout));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::add(const PdmObjectHandle* obj, LayoutOptions layout)
{
    PdmUiObjectHandle* uiItem = uiObj(const_cast<PdmObjectHandle*>(obj));
    CAF_ASSERT(uiItem);
    CAF_ASSERT(!this->contains(uiItem));
    m_ordering.push_back(std::make_pair(uiItem, layout));
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::insert(size_t index, const PdmFieldHandle* field, LayoutOptions layout)
{
    PdmUiFieldHandle* uiItem = const_cast<PdmFieldHandle*>(field)->uiCapability();
    CAF_ASSERT(uiItem);
    CAF_ASSERT(!this->contains(uiItem));

    m_ordering.insert(m_ordering.begin() + index, std::make_pair(uiItem, layout));  
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiOrdering::isIncludingRemainingFields() const
{
    return !m_skipRemainingFields;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::skipRemainingFields(bool doSkip /*= true*/)
{
    m_skipRemainingFields = doSkip;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<PdmUiItem*> PdmUiOrdering::uiItems() const
{
    std::vector<PdmUiItem*> justUiItems;
    justUiItems.reserve(m_ordering.size());
    for (const FieldAndLayout& itemAndLayout : m_ordering)
    {
        justUiItems.push_back(itemAndLayout.first);
    }
    return justUiItems;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<PdmUiOrdering::FieldAndLayout>& PdmUiOrdering::uiItemsWithLayout() const
{
    return m_ordering;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiOrdering::TableLayout PdmUiOrdering::calculateTableLayout(const QString& uiConfigName) const
{
    TableLayout tableLayout;

    for (size_t i = 0; i < m_ordering.size(); ++i)
    {
        if (m_ordering[i].first->isUiHidden(uiConfigName)) continue;

        if (m_ordering[i].second.newRow || i == 0u)
        {
            tableLayout.push_back(RowLayout());
        }
        tableLayout.back().push_back(m_ordering[i]);
    }
    return tableLayout;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiOrdering::nrOfColumns(const TableLayout& tableLayout) const
{
    int maxNrOfColumns = 0;

    for (const auto& rowContent : tableLayout)
    {
        maxNrOfColumns = std::max(maxNrOfColumns, nrOfRequiredColumnsInRow(rowContent));
    }

    return maxNrOfColumns;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiOrdering::nrOfRequiredColumnsInRow(const RowLayout& rowItems) const
{
    int totalColumns = 0;
    for (const FieldAndLayout& item : rowItems)
    {
        int totalItemColumns = 0, labelItemColumns = 0, fieldItemColumns = 0;
        nrOfColumnsRequiredForItem(item, &totalItemColumns, &labelItemColumns, &fieldItemColumns);
        totalColumns += totalItemColumns;
    }
    return totalColumns;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiOrdering::nrOfExpandingItemsInRow(const RowLayout& rowItems) const
{
    int nrOfExpandingItems = 0;
    for (const FieldAndLayout& item : rowItems)
    {
        if (item.second.totalColumnSpan == LayoutOptions::MAX_COLUMN_SPAN)
            nrOfExpandingItems++;
    }
    return nrOfExpandingItems;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::nrOfColumnsRequiredForItem(const FieldAndLayout& fieldAndLayout,
                                               int*                  totalColumnsRequired,
                                               int*                  labelColumnsRequired,
                                               int*                  fieldColumnsRequired) const
{
    const PdmUiItem* uiItem = fieldAndLayout.first;
    CAF_ASSERT(uiItem && totalColumnsRequired && labelColumnsRequired && fieldColumnsRequired);

    LayoutOptions layoutOption = fieldAndLayout.second;

    if (uiItem->isUiGroup())
    {
        *totalColumnsRequired = 1;
        *labelColumnsRequired = 0;
        *fieldColumnsRequired = 0;
    }
    else
    {
        *fieldColumnsRequired = 1;
        *labelColumnsRequired = 0;
        if (uiItem->uiLabelPosition() == PdmUiItemInfo::LEFT)
        {
            *labelColumnsRequired = 1;
            if (layoutOption.leftLabelColumnSpan != LayoutOptions::MAX_COLUMN_SPAN)
            {
                *labelColumnsRequired = layoutOption.leftLabelColumnSpan;
            }
        }
        *totalColumnsRequired = *labelColumnsRequired + *fieldColumnsRequired;
    }

    if (layoutOption.totalColumnSpan != LayoutOptions::MAX_COLUMN_SPAN)
    {
        *totalColumnsRequired = layoutOption.totalColumnSpan;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiItem* PdmUiOrdering::PositionFound::item()
{
    if ( parent ) 
    {
        return parent->uiItems()[indexInParent];
    } 
    else 
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::PositionFound::group()
{
    PdmUiItem* g = item();
    if ( g && g->isUiGroup() )
    {
        return static_cast<PdmUiGroup*>(g);
    }
    else
    {
        return nullptr;
    }
}

} //End of namespace caf

