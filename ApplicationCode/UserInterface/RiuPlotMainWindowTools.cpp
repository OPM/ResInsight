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

#include "RiuPlotMainWindowTools.h"
#include "RiaApplication.h"
#include "RiuPlotMainWindow.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::showPlotMainWindow()
{
    RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::setActiveViewer(QWidget* subWindow)
{
    RiuPlotMainWindow* mpw = RiaApplication::instance()->mainPlotWindow();

    if (mpw) mpw->setActiveViewer(subWindow);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::setExpanded(const caf::PdmUiItem* uiItem, bool expanded /*= true*/)
{
    RiuPlotMainWindow* mpw = RiaApplication::instance()->mainPlotWindow();

    if (mpw) mpw->setExpanded(uiItem, expanded);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::selectAsCurrentItem(const caf::PdmObject* object, bool allowActiveViewChange /*= true*/)
{
    RiuPlotMainWindow* mpw = RiaApplication::instance()->mainPlotWindow();

    if (mpw) mpw->selectAsCurrentItem(object, allowActiveViewChange);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::refreshToolbars()
{
    RiuPlotMainWindow* mpw = RiaApplication::instance()->mainPlotWindow();

    if (mpw)
    {
        mpw->updateSummaryPlotToolBar();        
        mpw->updateWellLogPlotToolBar();
    }
}


