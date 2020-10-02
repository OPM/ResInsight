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

#include "RiuMdiSubWindow.h"

#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimSummaryPlot.h"
#include "RimWellLogPlot.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuViewer.h"

#include <QWindowStateChangeEvent>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMdiSubWindow::RiuMdiSubWindow( QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/ )
    : QMdiSubWindow( parent, flags )
    , m_normalWindowGeometry( QRect() )
    , m_blockTilingChanges( false )
{
    setWindowIcon( QIcon( ":/Window16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMdiSubWindow::~RiuMdiSubWindow()
{
    RiuMainWindow::instance()->slotRefreshViewActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RiuMdiSubWindow::windowGeometry() const
{
    RimMdiWindowGeometry geo;

    int mainWinID = 0;
    if ( window() == RiaGuiApplication::instance()->mainPlotWindow() )
    {
        mainWinID = 1;
    }

    geo.mainWindowID = mainWinID;
    geo.isMaximized  = isMaximized();

    // Save normal/non-maximized size and position so this can be restored
    QRect currentGeometry = frameGeometry();
    if ( isMaximized() && !m_normalWindowGeometry.isNull() )
    {
        currentGeometry = m_normalWindowGeometry;
    }

    geo.x      = currentGeometry.topLeft().x();
    geo.y      = currentGeometry.topLeft().y();
    geo.width  = currentGeometry.width();
    geo.height = currentGeometry.height();

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiSubWindow::blockTilingChanges( bool block )
{
    m_blockTilingChanges = block;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiSubWindow::closeEvent( QCloseEvent* event )
{
    QWidget* mainWidget = widget();

    RimViewWindow* viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget( mainWidget );
    if ( !viewWindow )
    {
        RiuViewer* viewer = mainWidget->findChild<RiuViewer*>();
        if ( viewer )
        {
            viewWindow = viewer->ownerViewWindow();
        }
    }

    if ( viewWindow )
    {
        viewWindow->setMdiWindowGeometry( windowGeometry() );
        viewWindow->handleMdiWindowClosed();
        event->accept();
    }
    else
    {
        QMdiSubWindow::closeEvent( event );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiSubWindow::resizeEvent( QResizeEvent* resizeEvent )
{
    if ( !isMaximized() )
    {
        m_normalWindowGeometry = frameGeometry();
    }

    if ( !m_blockTilingChanges )
    {
        if ( window() == RiaGuiApplication::instance()->mainWindow() )
        {
            RiaGuiApplication::instance()->mainWindow()->storeSubWindowTiling( false );
        }
        else if ( window() == RiaGuiApplication::instance()->mainPlotWindow() )
        {
            RiaGuiApplication::instance()->mainPlotWindow()->storeSubWindowTiling( false );
        }
    }

    QMdiSubWindow::resizeEvent( resizeEvent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiSubWindow::moveEvent( QMoveEvent* moveEvent )
{
    if ( !isMaximized() )
    {
        m_normalWindowGeometry = frameGeometry();
    }

    if ( !m_blockTilingChanges )
    {
        if ( window() == RiaGuiApplication::instance()->mainWindow() )
        {
            RiaGuiApplication::instance()->mainWindow()->storeSubWindowTiling( false );
        }
        else if ( window() == RiaGuiApplication::instance()->mainPlotWindow() )
        {
            RiaGuiApplication::instance()->mainPlotWindow()->storeSubWindowTiling( false );
        }
    }

    QMdiSubWindow::moveEvent( moveEvent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiSubWindow::showEvent( QShowEvent* event )
{
    QMdiSubWindow::showEvent( event );
    resize( size() + QSize( 1, 1 ) );
    update();
    QApplication::processEvents();
    resize( size() + QSize( -1, -1 ) );
    update();
    QApplication::processEvents();
}
