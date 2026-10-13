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
#include <QTimer>

#include <algorithm>
#include <limits>
#include <map>

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
    if ( m_overlayFrames.removeOne( overlayFrame ) )
    {
        // Rebuild the positions of the remaining frames, deferred to stay safe during widget destruction
        QTimer::singleShot( 0, this, [this]() { updateOverlayFrameLayout(); } );
    }
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
    using Corner = RiuDraggableOverlayFrame::AnchorCorner;

    const int spacing = 5;

    int xpos                 = spacing;
    int ypos                 = spacing;
    int widthOfCurrentColumn = 0;

    QSize canvasSize = getParentForOverlay()->size();
    QSize maxFrameSize( canvasSize.width() - 2 * m_overlayMargins, canvasSize.height() - 2 * m_overlayMargins );

    // Frames stacked in the same corner (other than top left) share the width of the widest one to align the stack
    std::map<Corner, int> stackWidths;
    for ( RiuDraggableOverlayFrame* frame : m_overlayFrames )
    {
        if ( frame && frame->anchorCorner() != Corner::TopLeft )
        {
            int& stackWidth = stackWidths[frame->anchorCorner()];
            stackWidth      = std::max( { stackWidth, frame->minimumSizeHint().width(), frame->sizeHint().width() } );
        }
    }

    // Offset of the next frame from the anchor corner. A stack wraps into a new column towards the plot center when full.
    struct StackPosition
    {
        int x;
        int y;
    };
    std::map<Corner, StackPosition> stackPositions;

    const QRect canvasRect = getParentForOverlay()->rect();

    auto placeInStack = [&]( RiuDraggableOverlayFrame* frame )
    {
        const Corner   corner   = frame->anchorCorner();
        StackPosition& position = stackPositions.try_emplace( corner, StackPosition{ spacing, spacing } ).first->second;
        if ( position.y + frame->height() + spacing > canvasRect.height() && position.y > spacing )
        {
            position.x += frame->width() + spacing;
            position.y = spacing;
        }

        const bool isRight  = corner == Corner::TopRight || corner == Corner::BottomRight;
        const bool isBottom = corner == Corner::BottomLeft || corner == Corner::BottomRight;
        const int  x        = isRight ? canvasRect.right() - position.x - frame->width() + 1 : position.x;
        const int  y        = isBottom ? canvasRect.bottom() - position.y - frame->height() + 1 : position.y;
        frame->move( x, y );
        position.y += frame->height() + spacing;
    };

    for ( RiuDraggableOverlayFrame* frame : m_overlayFrames )
    {
        if ( frame )
        {
            QSize minFrameSize     = frame->minimumSizeHint();
            QSize desiredFrameSize = frame->sizeHint();

            int width  = frame->anchorCorner() == Corner::TopLeft ? std::max( minFrameSize.width(), desiredFrameSize.width() )
                                                                  : stackWidths[frame->anchorCorner()];
            width      = std::min( width, maxFrameSize.width() );
            int height = std::min( std::max( minFrameSize.height(), desiredFrameSize.height() ), maxFrameSize.height() );

            frame->resize( width, height );

            if ( frame->anchorCorner() == Corner::TopLeft )
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
            else if ( frame->anchorCorner() == Corner::TopRight )
            {
                placeInStack( frame );
            }
            frame->show();
        }
    }

    // Bottom corners stack upwards from the bottom edge. Iterate in reverse so the first frame ends up topmost.
    for ( auto it = m_overlayFrames.rbegin(); it != m_overlayFrames.rend(); ++it )
    {
        RiuDraggableOverlayFrame* frame = *it;
        if ( frame && ( frame->anchorCorner() == Corner::BottomLeft || frame->anchorCorner() == Corner::BottomRight ) )
        {
            placeInStack( frame );
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
