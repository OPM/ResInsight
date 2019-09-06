/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace caf
{
class PdmUiItem;
class PdmUiTreeOrdering;
class PdmUiTreeView;
}; // namespace caf

//==================================================================================================
///
//==================================================================================================
class RicToggleItemsFeatureImpl
{
public:
    enum SelectionToggleType
    {
        TOGGLE_ON,
        TOGGLE_OFF,
        TOGGLE_SUBITEMS,
        TOGGLE,
        TOGGLE_UNDEFINED
    };

    static bool isToggleCommandsAvailable();
    static bool isToggleCommandsForSubItems();
    static void setObjectToggleStateForSelection( SelectionToggleType state );

private:
    static caf::PdmUiTreeView*     findTreeView( const caf::PdmUiItem* uiItem );
    static caf::PdmUiTreeOrdering* findTreeItemFromSelectedUiItem( const caf::PdmUiItem* uiItem );
};
