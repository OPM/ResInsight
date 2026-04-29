/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimDockWindowController.h"

#include "RiaGuiApplication.h"
#include "RimProject.h"
#include "RimViewWindow.h"
#include "RiuMainWindowBase.h"

#include "DockManager.h"
#include "DockWidget.h"

CAF_PDM_XML_SOURCE_INIT( RimDockWindowController, "DockWindowController" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDockWindowController::RimDockWindowController()
{
    CAF_PDM_InitField( &m_mainWindowID, "MainWindowID", -1, "" );
    CAF_PDM_InitField( &m_x, "xPos", -1, "" );
    CAF_PDM_InitField( &m_y, "yPos", -1, "" );
    CAF_PDM_InitField( &m_width, "Width", -1, "" );
    CAF_PDM_InitField( &m_height, "Height", -1, "" );
    CAF_PDM_InitField( &m_isMaximized, "IsMaximized", false, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDockWindowController::~RimDockWindowController()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::setWindowGeometry( const RimMdiWindowGeometry& windowGeometry )
{
    m_mainWindowID = windowGeometry.mainWindowID;
    m_x            = windowGeometry.x;
    m_y            = windowGeometry.y;
    m_width        = windowGeometry.width;
    m_height       = windowGeometry.height;
    m_isMaximized  = windowGeometry.isMaximized;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RimDockWindowController::windowGeometry()
{
    RimMdiWindowGeometry windowGeometry;

    windowGeometry.mainWindowID = m_mainWindowID;
    windowGeometry.x            = m_x;
    windowGeometry.y            = m_y;
    windowGeometry.width        = m_width;
    windowGeometry.height       = m_height;
    windowGeometry.isMaximized  = m_isMaximized;

    return windowGeometry;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::handleViewerDeletion()
{
    viewPdmObject()->m_showWindow = false;
    viewPdmObject()->updateConnectedEditors();
    viewPdmObject()->updateUiIconFromToggleField();
    uiCapability()->updateUiIconFromToggleField();
    removeWindowFromDock();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::removeWindowFromDock()
{
    RiuMainWindowBase* mainWin = getMainWindow();
    if ( mainWin && viewWidget() )
    {
        mainWin->removeViewer( viewWidget() );
        viewPdmObject()->deleteViewWidget();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RimDockWindowController::viewPdmObject()
{
    return firstAncestorOrThisOfType<RimViewWindow>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimDockWindowController::viewWidget()
{
    return viewPdmObject()->viewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase* RimDockWindowController::getMainWindow()
{
    if ( RiaGuiApplication::isRunning() )
    {
        return RiaGuiApplication::instance()->mainWindowByID( m_mainWindowID );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::setupBeforeSave()
{
    if ( viewWidget() && getMainWindow() )
    {
        setWindowGeometry( getMainWindow()->windowGeometryForViewer( viewWidget() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::updateViewerWidget()
{
    RiuMainWindowBase* mainWindow = getMainWindow();
    if ( !mainWindow ) return;

    if ( viewPdmObject()->isWindowVisible() )
    {
        if ( !viewWidget() )
        {
            ads::CDockWidget* viewWindow = mainWindow->createDockViewWindow();
            QWidget*          viewWidget = viewPdmObject()->createViewWidget( viewWindow );
            mainWindow->initializeViewer( viewWindow, viewWidget, windowGeometry() );

            viewPdmObject()->updateViewWidgetAfterCreation();
        }

        viewPdmObject()->updateMdiWindowTitle();
    }
    else
    {
        if ( viewWidget() )
        {
            setWindowGeometry( mainWindow->windowGeometryForViewer( viewWidget() ) );

            mainWindow->removeViewer( viewWidget() );

            viewPdmObject()->deleteViewWidget();
        }
    }
}
