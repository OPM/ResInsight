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
#include "Rim3dView.h"
#include "RimPlotWindow.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "RiuDockWidgetTools.h"
#include "RiuMainWindow.h"
#include "RiuMainWindowBase.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuViewer.h"

#include "DockManager.h"
#include "DockWidget.h"

CAF_PDM_XML_SOURCE_INIT( RimDockWindowController, "DockWindowController", "MdiWindowController" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDockWindowController::RimDockWindowController()
{
    CAF_PDM_InitField( &m_mainWindowID, "MainWindowID", RimDockWindowController::MAIN_WINDOW_ID_UNASSIGNED, "" );
    CAF_PDM_InitFieldNoDefault( &m_viewToControl, "ViewToControl", "" );
    m_viewToControl = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDockWindowController::~RimDockWindowController()
{
    m_viewToControl = nullptr;
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
    if ( mainWin && viewPdmObject() )
    {
        viewPdmObject()->deleteDockWidget();
        viewPdmObject()->deleteViewWidget();

        mainWin->onViewerRemoved();
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
            QWidget*          viewWidget = viewPdmObject()->createViewWidget();
            dockWidget->setWidget( viewWidget );
            dockWidget->setObjectName( viewPdmObject()->dockWindowName() );
            viewWidget->setObjectName( viewPdmObject()->dockWindowName() );
            mainWindow->initializeViewer( dockWidget, viewWidget );

            RiuMainWindowBase::connect( dockWidget, SIGNAL( closed() ), mainWindow, SLOT( slotDockViewerClosed() ) );
            RiuMainWindowBase::connect( dockWidget,
                                        SIGNAL( visibilityChanged( bool ) ),
                                        mainWindow,
                                        SLOT( slotDockViewerVisibilityChanged( bool ) ) );

            viewPdmObject()->updateViewWidgetAfterCreation();
        }

        viewPdmObject()->updateWindowTitle();
    }
    else
    {
        viewPdmObject()->deleteDockWidget();
        if ( viewWidget() )
        {
            viewPdmObject()->deleteViewWidget();
            mainWindow->onViewerRemoved();
            setViewToControl( nullptr );
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
    return m_mainWindowID();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDockWindowController::setAsActiveViewer()
{
    if ( getMainWindow() == nullptr ) return;

    if ( auto pdmView = viewPdmObject() )
    {
        auto view3d   = dynamic_cast<Rim3dView*>( pdmView );
        auto viewPlot = dynamic_cast<RimPlotWindow*>( pdmView );

        RiuMainWindowBase* mainWindow = RiaGuiApplication::instance()->mainWindow();
        if ( viewPlot ) mainWindow = RiaGuiApplication::instance()->mainPlotWindow();
        if ( !mainWindow ) return;

        for ( auto view : mainWindow->viewWindows() )
        {
            if ( view->isActive() )
            {
                view->setActive( false );
                view->updateWindowTitle();
            }
        }

        pdmView->setActive( true );
        pdmView->updateWindowTitle();

        if ( view3d )
        {
            RiaApplication::instance()->setActiveReservoirView( view3d );
            if ( auto mainWin = dynamic_cast<RiuMainWindow*>( mainWindow ) )
            {
                mainWin->refreshViewActions();
                mainWin->refreshAnimationActions();
                mainWin->refreshDrawStyleActions();
            }
        }
        else if ( viewPlot )
        {
            RiuPlotMainWindowTools::selectAsCurrentItem( viewPlot );
            RiuPlotMainWindowTools::refreshToolbars();
        }
    }
}
