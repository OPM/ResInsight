//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafPdmUiItem.h"
#include "cafPdmUiOrdering.h"

namespace caf
{

//==================================================================================================
/// Class representing a group of fields communicated to the Gui
//==================================================================================================

class PdmUiGroup : public PdmUiItem, public PdmUiOrdering
{
public:
    PdmUiGroup();

    void        setKeyword(const QString& keyword);
    QString     keyword() const;

    bool        isUiGroup() const override;

    /// Set this group to be collapsed by default. When the user expands the group, the default no longer has any effect. 
    void        setCollapsedByDefault(bool doCollapse); 
    /// Forcifully set the collapsed state of the group, overriding the previous user actions and the default
    void        setCollapsed(bool doCollapse);
    void        setEnableFrame(bool enableFrame);

    // Pdm internal methods
    bool        isExpandedByDefault()    const; 
    bool        hasForcedExpandedState() const; 
    bool        forcedExpandedState()    const;
    bool        enableFrame() const;

private:
    bool    m_isCollapsedByDefault;
    bool    m_hasForcedExpandedState;
    bool    m_forcedCollapseState;
    bool    m_enableFrame;
    
    QString m_keyword;
};



} // End of namespace caf

