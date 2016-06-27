/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicTilePlotWindowsFeature.h"

#include "RiaApplication.h"
#include "RiuMainPlotWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicTilePlotWindowsFeature, "RicTilePlotWindowsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicTilePlotWindowsFeature::isCommandEnabled()
{
    RiuMainPlotWindow* wnd = RiaApplication::instance()->mainPlotWindow();
    if (wnd)
    {
        return wnd->isAnyMdiSubWindowVisible();
    }
    
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsFeature::onActionTriggered(bool isChecked)
{
    RiuMainPlotWindow* wnd = RiaApplication::instance()->mainPlotWindow();
    if (wnd)
    {
        wnd->tileWindows();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Tile Windows");
    actionToSetup->setIcon(QIcon(":/view-page-multi-24.png"));
}
