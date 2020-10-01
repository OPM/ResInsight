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

#include "RicTileWindowsFeature.h"

#include "RiaGuiApplication.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QApplication>

CAF_CMD_SOURCE_INIT( RicTileWindowsFeature, "RicTileWindowsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTileWindowsFeature::isCommandEnabled()
{
    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if ( mainWindow )
    {
        return mainWindow->isAnyMdiSubWindowVisible();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTileWindowsFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if ( mainWindow )
    {
        if ( !mainWindow->subWindowsAreTiled() )
        {
            mainWindow->tileSubWindows();
        }
        else
        {
            mainWindow->clearWindowTiling();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTileWindowsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Tile Windows" );
    actionToSetup->setIcon( QIcon( ":/TileWindows.svg" ) );
    actionToSetup->setCheckable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTileWindowsFeature::isCommandChecked()
{
    if ( RiaGuiApplication::instance()->mainWindow() )
    {
        return RiaGuiApplication::instance()->mainWindow()->subWindowsAreTiled();
    }
    return false;
}

CAF_CMD_SOURCE_INIT( RicTilePlotWindowsFeature, "RicTilePlotWindowsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTilePlotWindowsFeature::isCommandEnabled()
{
    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    if ( mainPlotWindow )
    {
        return mainPlotWindow->isAnyMdiSubWindowVisible();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsFeature::onActionTriggered( bool isChecked )
{
    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    if ( mainPlotWindow )
    {
        if ( !mainPlotWindow->subWindowsAreTiled() )
        {
            mainPlotWindow->tileSubWindows();
        }
        else
        {
            mainPlotWindow->clearWindowTiling();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Tile Windows" );
    actionToSetup->setIcon( QIcon( ":/TileWindows.svg" ) );
    actionToSetup->setCheckable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTilePlotWindowsFeature::isCommandChecked()
{
    if ( RiaGuiApplication::instance()->mainPlotWindow() )
    {
        return RiaGuiApplication::instance()->mainPlotWindow()->subWindowsAreTiled();
    }
    return false;
}
