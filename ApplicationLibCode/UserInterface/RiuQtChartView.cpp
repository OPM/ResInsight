////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiuQtChartView.h"

#include "RimPlotWindow.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartView::RiuQtChartView( RimPlotWindow* plotWindow, QWidget* parent )
    : QtCharts::QChartView( parent )
    , m_plotWindow( plotWindow )
{
    setMouseTracking( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartView::~RiuQtChartView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuQtChartView::ownerViewWindow() const
{
    return m_plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartView::mousePressEvent( QMouseEvent* event )
{
    if ( event->buttons() & Qt::MiddleButton )
    {
        m_isPanning        = true;
        m_panStartPosition = event->pos();
        setCursor( Qt::ClosedHandCursor );
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartView::mouseReleaseEvent( QMouseEvent* event )
{
    if ( event->buttons() & Qt::MiddleButton )
    {
        m_isPanning = false;
        setCursor( Qt::ArrowCursor );
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartView::mouseMoveEvent( QMouseEvent* event )
{
    if ( event->buttons() & Qt::MiddleButton && m_isPanning )
    {
        QPoint  newPosition = event->pos();
        QPointF delta       = mapToScene( newPosition ) - mapToScene( m_panStartPosition );
        chart()->scroll( -delta.x(), delta.y() );
        m_panStartPosition = newPosition;
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
