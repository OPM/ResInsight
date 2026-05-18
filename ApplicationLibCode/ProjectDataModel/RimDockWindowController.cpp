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
#include "RiuViewer.h"

#include "DockManager.h"
#include "DockWidget.h"

CAF_PDM_XML_SOURCE_INIT( RimDockWindowController, "DockWindowController" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDockWindowController::RimDockWindowController()
{
    CAF_PDM_InitField( &m_mainWindowID, "MainWindowID", 0, "" );
    CAF_PDM_InitFieldNoDefault( &m_viewToControl, "ViewToControl", "" );
    m_viewToControl = nullptr;
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
    if ( mainWin && viewWidget() && viewPdmObject() )
    {
        viewPdmObject()->deleteDockWidget();
        viewPdmObject()->deleteViewWidget();

        mainWin->removeViewer( viewWidget() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RimDockWindowController::viewPdmObject()
{
    return m_viewToControl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimDockWindowController::viewWidget()
{
    if ( !viewPdmObject() ) return nullptr;
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::updateViewerWidget()
{
    RiuMainWindowBase* mainWindow = getMainWindow();
    if ( !mainWindow ) return;
    if ( !viewPdmObject() ) return;

    if ( viewPdmObject()->isWindowVisible() )
    {
        if ( !viewWidget() )
        {
            ads::CDockWidget* dockWidget = viewPdmObject()->createDockWidget();
            QWidget*          viewWidget = viewPdmObject()->createViewWidget( dockWidget );
            dockWidget->setWidget( viewWidget );
            dockWidget->setObjectName( viewPdmObject()->dockWindowName() );
            mainWindow->initializeViewer( dockWidget, viewWidget );

            mainWindow->connect( dockWidget, SIGNAL( closed() ), mainWindow, SLOT( slotDockViewerClosed() ) );
            mainWindow->connect( dockWidget, SIGNAL( visibilityChanged( bool ) ), mainWindow, SLOT( slotDockViewerVisibilityChanged( bool ) ) );

            viewPdmObject()->updateViewWidgetAfterCreation();
        }

        viewPdmObject()->updateWindowTitle();
    }
    else
    {
        if ( viewWidget() )
        {
            viewPdmObject()->deleteDockWidget();
            mainWindow->removeViewer( viewWidget() );

            viewPdmObject()->deleteViewWidget();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::setMainWindowId( int mainId )
{
    m_mainWindowID = mainId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::setViewToControl( RimViewWindow* view )
{
    m_viewToControl = view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDockWindowController::mainWindowId() const
{
    return m_mainWindowID;
}
