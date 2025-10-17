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

#include "RicExitApplicationFeature.h"

#include "RiaGuiApplication.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QDebug>

CAF_CMD_SOURCE_INIT( RicExitApplicationFeature, "RicExitApplicationFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExitApplicationFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    RiaGuiApplication* app = RiaGuiApplication::instance();

    if ( !app->checkWithUserBeforeClose() ) return;

    if ( app->mainPlotWindow() )
    {
        app->mainPlotWindow()->saveWinGeoAndDockToolBarLayout();
    }

    if ( app->mainWindow() )
    {
        app->mainWindow()->saveWinGeoAndDockToolBarLayout();
    }

    // Hide all windows first to make sure they get closed properly
    for ( QWidget* topLevelWidget : RiaGuiApplication::topLevelWidgets() )
    {
        topLevelWidget->hide();
    }

    if ( app->mainWindow() )
    {
        app->mainWindow()->close();
    }

    if ( app->mainPlotWindow() )
    {
        app->mainPlotWindow()->close();
    }

    // This was required after moving to Qt6, causing the application not to shut down properly, and ghost processes remains after a forced
    // shutdown. The slot onLastWindowClosed() configured in RiaGuiApplication::RiaGuiApplication is never called. Testing with
    // processEvents() had no effect.
    app->closeProject();
    RiaGuiApplication::quit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExitApplicationFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "E&xit" );

    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Quit );
}
