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
#include <vector>
#include <QString>

#include "cafPdmUiItem.h"

namespace caf 
{

class PdmUiGroup;

//==================================================================================================
/// Class storing the order and grouping of fields and groups of fields etc. to be used in the Gui
//==================================================================================================

class PdmUiOrdering
{
public:
    PdmUiOrdering(): m_forgetRemainingFields(false) { };
    virtual ~PdmUiOrdering();

    PdmUiGroup*                    addNewGroup(QString displayName);
    void                           add(PdmUiItem* item)               { m_ordering.push_back(item); }

    /// HACK constness of this class and functions must be revisited
    void                           add(const PdmUiItem* item)               { m_ordering.push_back(const_cast<PdmUiItem*>(item)); }

    bool                           forgetRemainingFields() const      { return m_forgetRemainingFields; }
    void                           setForgetRemainingFields(bool val) { m_forgetRemainingFields = val; }

    const std::vector<PdmUiItem*>& uiItems() const                    { return m_ordering; }
    bool                           contains(const PdmUiItem* item);

private:
    // Private copy constructor and assignment to prevent this. (The vectors below will make trouble)
    PdmUiOrdering(const PdmUiOrdering& other)                                 { }
    PdmUiOrdering&       operator= (const PdmUiOrdering& other)               { }

    std::vector<PdmUiItem*>     m_ordering;      ///< The order of groups and fields
    std::vector<PdmUiGroup*>    m_createdGroups; ///< Owned PdmUiGroups, for mem management
    bool                        m_forgetRemainingFields;
};

//==================================================================================================
/// Class representing a group of fields
//==================================================================================================

class PdmUiGroup : public PdmUiItem, public PdmUiOrdering
{
    virtual bool isUiGroup() { return true; }
};



} // End of namespace caf

