/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RiuComparisonViewMover.h"

#include "cafViewer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuComparisonViewMover::RiuComparisonViewMover( caf::Viewer* viewer )
    : QObject( viewer )
    , m_viewer( viewer )
    , m_dragState( NONE )
    , m_highlightHandle( NONE )
{
    viewer->installEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuComparisonViewMover::eventFilter( QObject* watched, QEvent* event )
{
    if ( !m_viewer->currentScene( true ) ) return false;

    switch ( event->type() )
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        {
            QMouseEvent* mEv = static_cast<QMouseEvent*>( event );

            if ( mEv->type() == QEvent::MouseButtonPress && mEv->button() == Qt::LeftButton )
            {
                DragState handle = findHandleUnderMouse( mEv->pos() );
                m_dragState      = handle;
            }
            else if ( mEv->type() == QEvent::MouseButtonRelease && mEv->button() == Qt::LeftButton )
            {
                m_dragState = NONE;
            }
            else if ( mEv->type() == QEvent::MouseMove )
            {
                m_highlightHandle = findHandleUnderMouse( mEv->pos() );

                if ( m_dragState == LEFT_EDGE )
                {
                    QPointF    mousePos     = mEv->windowPos();
                    QPointF    normMousePos = {mousePos.x() / m_viewer->width(), mousePos.y() / m_viewer->height()};
                    cvf::Rectf orgCompViewWindow = m_viewer->comparisonViewVisibleNormalizedRect();

                    m_viewer->setComparisonViewVisibleNormalizedRect(
                        cvf::Rectf( normMousePos.x(),
                                    orgCompViewWindow.min().y(),
                                    ( orgCompViewWindow.min().x() + orgCompViewWindow.width() ) -
                                        normMousePos.x(),
                                    orgCompViewWindow.height() ) );

                    return true;
                }
                else
                {
                    m_viewer->update();
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuComparisonViewMover::paintMoverHandles( QPainter* painter )
{
    if ( !m_viewer->currentScene( true ) ) return;

    const int  handleThickness          = 7;
    cvf::Rectf normalizedComparisonRect = m_viewer->comparisonViewVisibleNormalizedRect();
    int        viewerWidth              = m_viewer->width();
    int        viewerHeight             = m_viewer->height();

    int leftEdgePos     = viewerWidth * normalizedComparisonRect.min().x();
    int width           = viewerWidth * normalizedComparisonRect.width();
    int height          = viewerHeight * normalizedComparisonRect.height();
    int topEdgePosOgl   = viewerHeight * normalizedComparisonRect.max().y();
    int topEdgePosQt    = height - topEdgePosOgl;
    int bottomEdgePosQt = height - viewerHeight * normalizedComparisonRect.min().y();

    painter->setPen( QColor( 0, 0, 0, 30 ) );

    painter->drawRect( leftEdgePos, topEdgePosQt, width - 1, height - 1 );

    QColor handleColor( 0, 0, 0, 50 );
    if ( m_highlightHandle == LEFT_EDGE || m_dragState == LEFT_EDGE )
    {
        handleColor = QColor( 255, 255, 255, 50 );
    }

    painter->fillRect( leftEdgePos - handleThickness * 0.4,
                       bottomEdgePosQt - 8 * handleThickness,
                       handleThickness,
                       handleThickness * 6,
                       handleColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuComparisonViewMover::DragState RiuComparisonViewMover::findHandleUnderMouse( const QPoint& mousePos )
{
    const int  handleThickness          = 7;
    cvf::Rectf normalizedComparisonRect = m_viewer->comparisonViewVisibleNormalizedRect();
    int        viewerWidth              = m_viewer->width();
    int        viewerHeight             = m_viewer->height();

    int leftEdgePos     = viewerWidth * normalizedComparisonRect.min().x();
    int height          = viewerHeight * normalizedComparisonRect.height();
    int bottomEdgePosQt = height - viewerHeight * normalizedComparisonRect.min().y();

    if ( ( leftEdgePos - handleThickness * 0.4 ) < mousePos.x() &&
         mousePos.x() < ( leftEdgePos + handleThickness * 0.5 ) &&
         ( bottomEdgePosQt - 8 * handleThickness ) < mousePos.y() &&
         mousePos.y() < ( bottomEdgePosQt - 2 * handleThickness ) )
    {
        return LEFT_EDGE;
    }

    return NONE;
}
