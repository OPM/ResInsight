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
        m_createdGroups[i] = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiGroup* PdmUiOrdering::addNewGroup(QString displayName)
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
bool PdmUiOrdering::contains(const PdmUiItem* item)
{
    for (size_t i = 0; i < m_ordering.size(); ++i)
    {
        if (m_ordering[i] == item) return true;
        if (m_ordering[i] && m_ordering[i]->isUiGroup())
        {
            if (static_cast<PdmUiGroup*>(m_ordering[i])->contains(item)) return true;
        }
    }
    return false;
}

} //End of namespace caf

