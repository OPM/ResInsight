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
caf::PdmUiGroup* PdmUiOrdering::insertNewGroup(size_t index, const QString& displayName)
{
    PdmUiGroup* group = new PdmUiGroup;
    group->setUiName(displayName);

    m_createdGroups.push_back(group);

    m_ordering.insert(m_ordering.begin() + index, group);  
    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* PdmUiOrdering::insertNewGroupWithKeyword(size_t index, 
                                                          const QString& displayName, 
                                                          const QString& groupKeyword)
{
    PdmUiGroup* group = insertNewGroup(index, displayName);
    group->setKeyword(groupKeyword);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiOrdering::contains(const PdmUiItem* item) const
{
   return this->findItem(item).parent != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiOrdering::FindResult PdmUiOrdering::findItem(const PdmUiItem* item) const
{
    for (size_t i = 0; i < m_ordering.size(); ++i)
    {
        if (m_ordering[i] == item) return { const_cast<PdmUiOrdering*>(this), i};
        if (m_ordering[i] && m_ordering[i]->isUiGroup())
        {
            FindResult result =  static_cast<PdmUiGroup*>(m_ordering[i])->findItem(item);
            if (result.parent ) return result;
        }
    }
    return {nullptr, size_t(-1)};
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiOrdering::FindResult PdmUiOrdering::findGroup(const QString& groupKeyword) const
{
    for (size_t i = 0; i < m_ordering.size(); ++i)
    {
        if (m_ordering[i] && m_ordering[i]->isUiGroup())
        {
            if (static_cast<PdmUiGroup*>(m_ordering[i])->keyword() == groupKeyword)  return { const_cast<PdmUiOrdering*>(this), i};
            FindResult result =  static_cast<PdmUiGroup*>(m_ordering[i])->findGroup(groupKeyword);
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
caf::PdmUiItem* PdmUiOrdering::FindResult::item()
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
caf::PdmUiGroup* PdmUiOrdering::FindResult::group()
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

