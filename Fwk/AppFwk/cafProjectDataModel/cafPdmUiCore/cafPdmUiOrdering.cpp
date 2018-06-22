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
PdmUiGroup* PdmUiOrdering::addNewGroup(const QString& displayName)
{
    PdmUiGroup* group = new PdmUiGroup;
    group->setUiName(displayName);

    m_createdGroups.push_back(group);
    m_ordering.push_back(group);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::addNewGroupWithKeyword(const QString& displayName, const QString& keyword)
{
    PdmUiGroup* group = addNewGroup(displayName);

    group->setKeyword(keyword);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiOrdering::insertBeforeGroup(const QString& groupId, const PdmFieldHandle* field)
{
   PositionFound pos = findGroupPosition(groupId);
   if (pos.parent)
   {
       pos.parent->insert(pos.indexInParent, field);
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
bool PdmUiOrdering::insertBeforeItem(const PdmUiItem* item, const PdmFieldHandle* field)
{
    PositionFound pos = findItemPosition(item);
    if (pos.parent)
    {
        pos.parent->insert(pos.indexInParent, field);
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
caf::PdmUiGroup* PdmUiOrdering::createGroupBeforeGroup(const QString& groupId, const QString& displayName)
{
    return createGroupWithIdBeforeGroup(groupId, displayName, "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::createGroupBeforeItem(const PdmUiItem* item, const QString& displayName)
{
    return createGroupWithIdBeforeItem(item, displayName, "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::createGroupWithIdBeforeGroup(const QString& groupId, const QString& displayName, const QString& newGroupId)
{
    PositionFound pos = findGroupPosition(groupId);
    if (pos.parent)
    {
        return pos.parent->insertNewGroupWithKeyword(pos.indexInParent, displayName, newGroupId);
    }

    return nullptr;         
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::createGroupWithIdBeforeItem(const PdmUiItem* item, const QString& displayName, const QString& newGroupId)
{
    PositionFound pos = findItemPosition(item);
    if (pos.parent)
    {
        return pos.parent->insertNewGroupWithKeyword(pos.indexInParent, displayName, newGroupId);
    }

    return nullptr;         
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::findGroup(const QString& groupId)
{
    return findGroupPosition(groupId).group();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::insertNewGroupWithKeyword(size_t index, 
                                                          const QString& displayName, 
                                                          const QString& groupKeyword)
{
    PdmUiGroup* group = new PdmUiGroup;
    group->setUiName(displayName);

    m_createdGroups.push_back(group);

    m_ordering.insert(m_ordering.begin() + index, group);  

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
        if (m_ordering[i] == item) return { const_cast<PdmUiOrdering*>(this), i};
        if (m_ordering[i] && m_ordering[i]->isUiGroup())
        {
            PositionFound result =  static_cast<PdmUiGroup*>(m_ordering[i])->findItemPosition(item);
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
        if (m_ordering[i] && m_ordering[i]->isUiGroup())
        {
            if (static_cast<PdmUiGroup*>(m_ordering[i])->keyword() == groupKeyword)  return { const_cast<PdmUiOrdering*>(this), i};
            PositionFound result =  static_cast<PdmUiGroup*>(m_ordering[i])->findGroupPosition(groupKeyword);
            if (result.parent ) return result;
        }
    }
    return {nullptr,  size_t(-1)};
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::add(const PdmFieldHandle* field)
{
    PdmUiFieldHandle* uiItem = const_cast<PdmFieldHandle*>(field)->uiCapability();
    CAF_ASSERT(uiItem);
    CAF_ASSERT(!this->contains(uiItem));

    m_ordering.push_back(uiItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::add(const PdmObjectHandle* obj)
{
    PdmUiObjectHandle* uiItem = uiObj(const_cast<PdmObjectHandle*>(obj));
    CAF_ASSERT(uiItem);
    CAF_ASSERT(!this->contains(uiItem));

    m_ordering.push_back(uiItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiOrdering::insert(size_t index, const PdmFieldHandle* field)
{
    PdmUiFieldHandle* uiItem = const_cast<PdmFieldHandle*>(field)->uiCapability();
    CAF_ASSERT(uiItem);
    CAF_ASSERT(!this->contains(uiItem));

    m_ordering.insert(m_ordering.begin() + index, uiItem);  
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
const std::vector<PdmUiItem*>& PdmUiOrdering::uiItems() const
{
    return m_ordering;
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

