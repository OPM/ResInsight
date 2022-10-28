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
#include "RiuMdiArea.h"
#include "RiuPlotMainWindow.h"

#include "RimProject.h"
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

    auto mode = RiaDefines::WindowTileMode::DEFAULT;
    if ( RimProject::current()->subWindowsTileMode3DWindow() == RiaDefines::WindowTileMode::DEFAULT )
        mode = RiaDefines::WindowTileMode::UNDEFINED;

    RimProject::current()->setSubWindowsTileMode3DWindow( mode );

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if ( mainWindow )
    {
        mainWindow->mdiArea()->updateTiling();
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
    return RimProject::current()->subWindowsTileMode3DWindow() == RiaDefines::WindowTileMode::DEFAULT;
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
    //     auto mode = RimProject::current()->subWindowsTileModePlotWindow();
    //     if ( mode == RiaDefines::WindowTileMode::DEFAULT ) mode = RiaDefines::WindowTileMode::UNDEFINED;
    //
    //     RimProject::current()->setSubWindowsTileModePlotWindow( mode );
    //
    //     auto* mainWindow = RiuPlotMainWindow::instance();
    //     if ( mainWindow )
    //     {
    //         mainWindow->mdiArea()->updateTiling();
    //     }
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
    return RimProject::current()->subWindowsTileMode3DWindow() == RiaDefines::WindowTileMode::DEFAULT;
}

CAF_CMD_SOURCE_INIT( RicTileWindowsVerticallyFeature, "RicTileWindowsVerticallyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTileWindowsVerticallyFeature::isCommandEnabled()
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
void RicTileWindowsVerticallyFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    auto mode = RiaDefines::WindowTileMode::VERTICAL;
    if ( RimProject::current()->subWindowsTileMode3DWindow() == RiaDefines::WindowTileMode::VERTICAL )
        mode = RiaDefines::WindowTileMode::UNDEFINED;

    RimProject::current()->setSubWindowsTileMode3DWindow( mode );

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if ( mainWindow )
    {
        mainWindow->mdiArea()->updateTiling();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTileWindowsVerticallyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Tile Windows Vertically" );
    actionToSetup->setIcon( QIcon( ":/TileWindows.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTileWindowsVerticallyFeature::isCommandChecked()
{
    return RimProject::current()->subWindowsTileMode3DWindow() == RiaDefines::WindowTileMode::VERTICAL;
}

CAF_CMD_SOURCE_INIT( RicTileWindowsHorizontallyFeature, "RicTileWindowsHorizontallyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTileWindowsHorizontallyFeature::isCommandEnabled()
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
void RicTileWindowsHorizontallyFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    auto mode = RiaDefines::WindowTileMode::HORIZONTAL;
    if ( RimProject::current()->subWindowsTileMode3DWindow() == RiaDefines::WindowTileMode::HORIZONTAL )
        mode = RiaDefines::WindowTileMode::UNDEFINED;

    RimProject::current()->setSubWindowsTileMode3DWindow( mode );

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if ( mainWindow )
    {
        mainWindow->mdiArea()->updateTiling();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTileWindowsHorizontallyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Tile Windows Horizontally" );
    actionToSetup->setIcon( QIcon( ":/TileWindows.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTileWindowsHorizontallyFeature::isCommandChecked()
{
    return RimProject::current()->subWindowsTileMode3DWindow() == RiaDefines::WindowTileMode::DEFAULT;
}
