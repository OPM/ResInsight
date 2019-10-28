/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicActivateCurveFilterInToolbarFeature.h"

#include "RiaGuiApplication.h"
#include "RiuPlotMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicActivateCurveFilterInToolbarFeature, "RicActivateCurveFilterInToolbarFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicActivateCurveFilterInToolbarFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicActivateCurveFilterInToolbarFeature::onActionTriggered( bool isChecked )
{
    if ( RiaGuiApplication::isRunning() )
    {
        auto plotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        if ( plotWindow )
        {
            plotWindow->setFocusToLineEditInSummaryToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicActivateCurveFilterInToolbarFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Activate Summary Curve Filter Editor" );

    applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+F" ) ) );
}
