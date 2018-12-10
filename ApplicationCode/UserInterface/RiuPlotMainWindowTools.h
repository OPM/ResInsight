/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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
}

class RiuPlotMainWindowTools
{
public:
    static void            showPlotMainWindow();
    static void            setActiveViewer(QWidget* subWindow) ;
    static void            setExpanded(const caf::PdmUiItem* uiItem, bool expanded = true);
    static void            selectAsCurrentItem(const caf::PdmObject* object, bool allowActiveViewChange = true);
    static void            refreshToolbars();
};

