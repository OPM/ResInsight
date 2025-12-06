////////////////////////////////////////////////////////////////////////////////
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

#include "RiuQwtPlotWidget.h"

#include "RiaGuiApplication.h"
#include "RiaPlotDefines.h"
#include "RiaPlotWindowRedrawScheduler.h"

#include "RimMimeData.h"
#include "RimPlot.h"
#include "RimProject.h"

#include "RiuDragDrop.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuPlotMainWindow.h"

#include "cafAssert.h"

#include <QDragEnterEvent>
#include <QGraphicsSceneEvent>

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget::RiuPlotWidget( RimPlot* plotDefinition, QWidget* parent )
    : QWidget( parent )
    , m_plotDefinition( plotDefinition )
    , m_overlayMargins( 5 )
    , m_plotTitleText( "" )
    , m_plotTitleEnabled( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget::~RiuPlotWidget()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RiuPlotWidget::plotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotWidget::isChecked() const
{
    if ( m_plotDefinition )
    {
        return m_plotDefinition->showWindow();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::colSpan() const
{
    if ( m_plotDefinition )
    {
        return m_plotDefinition->colSpan();
    }
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::rowSpan() const
{
    if ( m_plotDefinition )
    {
        return m_plotDefinition->rowSpan();
    }
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiuPlotWidget::plotTitle() const
{
    return m_plotTitleText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::setPlotTitleEnabled( bool enabled )
{
    m_plotTitleEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotWidget::plotTitleEnabled() const
{
    return m_plotTitleEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPoint RiuPlotWidget::dragStartPosition() const
{
    return m_clickPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::scheduleReplot()
{
    RiaPlotWindowRedrawScheduler::instance()->schedulePlotWidgetReplot( this );
}

//--------------------------------------------------------------------------------------------------
/// Adds an overlay frame. The overlay frame becomes the responsibility of the plot widget
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::addOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    if ( std::find( m_overlayFrames.begin(), m_overlayFrames.end(), overlayFrame ) == m_overlayFrames.end() )
    {
        overlayFrame->setParent( getParentForOverlay() );
        m_overlayFrames.push_back( overlayFrame );
        updateLayout();
    }
}

//--------------------------------------------------------------------------------------------------
/// Remove the overlay widget. The frame becomes the responsibility of the caller
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::removeOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    CAF_ASSERT( overlayFrame );

    overlayFrame->hide();
    overlayFrame->setParent( nullptr );
    m_overlayFrames.removeOne( overlayFrame );
}

//--------------------------------------------------------------------------------------------------
/// Remove overlay widgets matching a given criteria
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::clearOverlayFrames( std::function<bool( RiuDraggableOverlayFrame* )> matcher )
{
    // Collect matching frames first to avoid iterator invalidation
    QList<QPointer<RiuDraggableOverlayFrame>> framesToRemove;

    for ( auto p : m_overlayFrames )
    {
        // Check if the QPointer is valid before calling matcher
        if ( p && matcher( p ) )
        {
            framesToRemove.append( p );
        }
    }

    // Now safely remove the collected frames
    for ( auto p : framesToRemove )
    {
        if ( p ) // Check again in case the object was deleted
        {
            removeOverlayFrame( p );
            p->deleteLater();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::removeEventFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::updateOverlayFrameLayout()
{
    const int spacing = 5;

    int xpos                 = spacing;
    int ypos                 = spacing;
    int widthOfCurrentColumn = 0;

    QSize canvasSize = getParentForOverlay()->size();
    QSize maxFrameSize( canvasSize.width() - 2 * m_overlayMargins, canvasSize.height() - 2 * m_overlayMargins );

    for ( RiuDraggableOverlayFrame* frame : m_overlayFrames )
    {
        if ( frame )
        {
            QSize minFrameSize     = frame->minimumSizeHint();
            QSize desiredFrameSize = frame->sizeHint();

            int width  = std::min( std::max( minFrameSize.width(), desiredFrameSize.width() ), maxFrameSize.width() );
            int height = std::min( std::max( minFrameSize.height(), desiredFrameSize.height() ), maxFrameSize.height() );

            frame->resize( width, height );

            if ( frame->anchorCorner() == RiuDraggableOverlayFrame::AnchorCorner::TopLeft )
            {
                if ( ypos + frame->height() + spacing > getParentForOverlay()->height() && widthOfCurrentColumn > 0 )
                {
                    xpos += spacing + widthOfCurrentColumn;
                    ypos                 = spacing;
                    widthOfCurrentColumn = 0;
                }
                frame->move( xpos, ypos );
                ypos += frame->height() + spacing;
                widthOfCurrentColumn = std::max( widthOfCurrentColumn, frame->width() );
            }
            else if ( frame->anchorCorner() == RiuDraggableOverlayFrame::AnchorCorner::TopRight )
            {
                QRect  frameRect      = frame->frameGeometry();
                QRect  canvasRect     = getParentForOverlay()->rect();
                QPoint canvasTopRight = canvasRect.topRight();
                frameRect.moveTopRight( QPoint( canvasTopRight.x() - spacing, canvasTopRight.y() + spacing ) );
                frame->move( frameRect.topLeft() );
            }
            frame->show();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotWidget::handleDragDropEvent( QEvent* event )
{
    if ( !event ) return false;

    if ( event->type() == QEvent::DragEnter )
    {
        auto dragEnterEvent = dynamic_cast<QDragEnterEvent*>( event );
        if ( dragEnterEvent )
        {
            dragEnterEvent->acceptProposedAction();

            return true;
        }
    }

    std::vector<caf::PdmObjectHandle*> objects;

    if ( RiuDragDrop::handleGenericDropEvent( event, objects ) )
    {
        if ( m_plotDefinition ) m_plotDefinition->handleDroppedObjects( objects );

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        if ( mainPlotWindow ) mainPlotWindow->updateMultiPlotToolBar();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::overlayMargins() const
{
    return m_overlayMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuPlotWidget::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::defaultMinimumWidth()
{
    return 80;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::updateZoomDependentCurveProperties()
{
}
