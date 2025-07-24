/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

class QWidget;

namespace caf
{
class PdmUiItem;
class PdmObject;
} // namespace caf

class RiuPlotMainWindowTools
{
public:
    static void showPlotMainWindow();
    static void setActiveViewer( QWidget* subWindow );
    static void setExpanded( const caf::PdmUiItem* uiItem );
    static void selectAsCurrentItem( const caf::PdmObject* object );
    static void selectOrToggleObject( const caf::PdmObject* object, bool toggle );
    static void refreshToolbars();

    // Returns the first visible ancestor of the object, or the object itself if it is visible.
    static const caf::PdmObject* firstVisibleAncestorOrThis( const caf::PdmObject* object );

    // Use this function to select (and expand) an object in the project tree and update tool bars. Use the second
    // parameter to expand a different object than the object to be selected.
    static void onObjectAppended( const caf::PdmObject* objectToSelect, const caf::PdmObject* objectToExpand = nullptr );

private:
    static void toggleItemInSelection( const caf::PdmObject* object );
};
