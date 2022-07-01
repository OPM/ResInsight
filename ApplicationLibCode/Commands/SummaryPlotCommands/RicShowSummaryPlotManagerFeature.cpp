////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicShowSummaryPlotManagerFeature.h"

#include "RiaGuiApplication.h"
#include "RiuPlotMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowSummaryPlotManagerFeature, "RicShowSummaryPlotManagerFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowSummaryPlotManagerFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowSummaryPlotManagerFeature::onActionTriggered( bool isChecked )
{
    RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();
    if ( mpw )
    {
        mpw->showAndSetKeyboardFocusToSummaryPlotManager();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowSummaryPlotManagerFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Show Summary Plot Manager" );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+K" ) ) );
}
