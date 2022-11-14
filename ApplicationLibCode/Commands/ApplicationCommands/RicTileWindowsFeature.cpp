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

#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuMdiArea.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QApplication>

CAF_CMD_SOURCE_INIT( RicTileWindowsFeature, "RicTileWindowsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTileWindowsFeature::applyTiling( RiuMainWindow* mainWindow, RiaDefines::WindowTileMode requestedTileMode )
{
    auto mode = requestedTileMode;

    // If requested mode is set, reset tiling mode to undefined
    if ( RimProject::current()->subWindowsTileMode3DWindow() == requestedTileMode )
        mode = RiaDefines::WindowTileMode::UNDEFINED;

    RimProject::current()->setSubWindowsTileMode3DWindow( mode );

    if ( mainWindow )
    {
        mainWindow->mdiArea()->applyTiling();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTileWindowsFeature::isCommandEnabled()
{
    auto* mainWindow = RiuMainWindow::instance();
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

    auto* mainWindow = RiuMainWindow::instance();
    applyTiling( mainWindow, RiaDefines::WindowTileMode::DEFAULT );
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
void RicTilePlotWindowsFeature::applyTiling( RiuPlotMainWindow* mainWindow, RiaDefines::WindowTileMode requestedTileMode )
{
    auto mode = requestedTileMode;

    // If requested mode is set, reset tiling mode to undefined
    if ( RimProject::current()->subWindowsTileModePlotWindow() == requestedTileMode )
        mode = RiaDefines::WindowTileMode::UNDEFINED;

    RimProject::current()->setSubWindowsTileModePlotWindow( mode );

    if ( mainWindow )
    {
        mainWindow->mdiArea()->applyTiling();
    }
}

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
    this->disableModelChangeContribution();

    auto* mainWindow = RiuPlotMainWindow::instance();
    RicTilePlotWindowsFeature::applyTiling( mainWindow, RiaDefines::WindowTileMode::DEFAULT );
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

    auto* mainWindow = RiuMainWindow::instance();
    RicTileWindowsFeature::applyTiling( mainWindow, RiaDefines::WindowTileMode::VERTICAL );
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

    auto* mainWindow = RiuMainWindow::instance();
    RicTileWindowsFeature::applyTiling( mainWindow, RiaDefines::WindowTileMode::HORIZONTAL );
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

//--------------------------------------------------------------------------------------------------
///
/// Main Plot window features
///
//--------------------------------------------------------------------------------------------------

CAF_CMD_SOURCE_INIT( RicTilePlotWindowsVerticallyFeature, "RicTilePlotWindowsVerticallyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTilePlotWindowsVerticallyFeature::isCommandEnabled()
{
    auto* mainWindow = RiuPlotMainWindow::instance();
    if ( mainWindow )
    {
        return mainWindow->isAnyMdiSubWindowVisible();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsVerticallyFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    auto* mainWindow = RiuPlotMainWindow::instance();
    RicTilePlotWindowsFeature::applyTiling( mainWindow, RiaDefines::WindowTileMode::VERTICAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsVerticallyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Tile Windows Vertically" );
    actionToSetup->setIcon( QIcon( ":/TileWindows.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTilePlotWindowsVerticallyFeature::isCommandChecked()
{
    return RimProject::current()->subWindowsTileModePlotWindow() == RiaDefines::WindowTileMode::VERTICAL;
}

CAF_CMD_SOURCE_INIT( RicTilePlotWindowsHorizontallyFeature, "RicTilePlotWindowsHorizontallyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTilePlotWindowsHorizontallyFeature::isCommandEnabled()
{
    auto* mainWindow = RiuPlotMainWindow::instance();
    if ( mainWindow )
    {
        return mainWindow->isAnyMdiSubWindowVisible();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsHorizontallyFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    auto* mainWindow = RiuPlotMainWindow::instance();
    RicTilePlotWindowsFeature::applyTiling( mainWindow, RiaDefines::WindowTileMode::HORIZONTAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTilePlotWindowsHorizontallyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Tile Windows Horizontally" );
    actionToSetup->setIcon( QIcon( ":/TileWindows.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTilePlotWindowsHorizontallyFeature::isCommandChecked()
{
    return RimProject::current()->subWindowsTileModePlotWindow() == RiaDefines::WindowTileMode::DEFAULT;
}
