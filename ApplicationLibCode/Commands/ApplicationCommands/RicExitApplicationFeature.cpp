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
bool RicExitApplicationFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExitApplicationFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RiaGuiApplication* app = RiaGuiApplication::instance();
    if ( !app->askUserToSaveModifiedProject() ) return;

    if ( app->mainPlotWindow() )
    {
        app->mainPlotWindow()->saveWinGeoAndDockToolBarLayout();
    }

    if ( app->mainWindow() )
    {
        app->mainWindow()->saveWinGeoAndDockToolBarLayout();
    }

    // Hide all windows first to make sure they get closed properly
    for ( QWidget* topLevelWidget : app->topLevelWidgets() )
    {
        topLevelWidget->hide();
    }
    // Close just the main window, it'll take care of closing the plot window
    app->mainWindow()->close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExitApplicationFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "E&xit" );

    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Quit );
}
