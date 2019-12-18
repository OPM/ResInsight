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

#include "RiuWidgetDragger.h"
#include <QEvent>
#include <QMouseEvent>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWidgetDragger::RiuWidgetDragger( QWidget* widgetToMove, int snapMargins /*= 5*/ )
    : QObject( widgetToMove )
    , m_widgetToMove( widgetToMove )
    , m_snapMargins( snapMargins )
    , m_startPos( 0, 0 )
{
    addWidget( m_widgetToMove );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWidgetDragger::addWidget( QWidget* widget )
{
    widget->installEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWidgetDragger::eventFilter( QObject* watched, QEvent* event )
{
    if ( event->type() == QEvent::MouseMove )
    {
        QMouseEvent* mMoveEv = static_cast<QMouseEvent*>( event );
        if ( mMoveEv->buttons() & Qt::LeftButton )
        {
            QPoint relativeMove = mMoveEv->pos() - m_startPos;
            QRect  newFrameRect = m_widgetToMove->frameGeometry().translated( relativeMove );
            QRect  snapToRect   = m_widgetToMove->parentWidget()->rect();

            {
                QPoint snapToTopLeft = snapToRect.topLeft();
                QPoint widgetTopLeft = newFrameRect.topLeft();
                QPoint diff          = snapToTopLeft - widgetTopLeft;
                if ( std::abs( diff.x() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveLeft( snapToTopLeft.x() + m_snapMargins );
                }
                if ( std::abs( diff.y() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveTop( snapToTopLeft.y() + m_snapMargins );
                }
            }
            {
                QPoint snapToBottomLeft = snapToRect.bottomLeft();
                QPoint widgetBottomLeft = newFrameRect.bottomLeft();
                QPoint diff             = snapToBottomLeft - widgetBottomLeft;
                if ( std::abs( diff.x() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveLeft( snapToBottomLeft.x() + m_snapMargins );
                }
                if ( std::abs( diff.y() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveBottom( snapToBottomLeft.y() - m_snapMargins );
                }
            }
            {
                QPoint snapToTopRight = snapToRect.topRight();
                QPoint widgetTopRight = newFrameRect.topRight();
                QPoint diff           = snapToTopRight - widgetTopRight;
                if ( std::abs( diff.x() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveRight( snapToTopRight.x() - m_snapMargins );
                }
                if ( std::abs( diff.y() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveTop( snapToTopRight.y() + m_snapMargins );
                }
            }
            {
                QPoint snapToBottomRight = snapToRect.bottomRight();
                QPoint widgetBottomRight = newFrameRect.bottomRight();
                QPoint diff              = snapToBottomRight - widgetBottomRight;
                if ( std::abs( diff.x() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveRight( snapToBottomRight.x() - m_snapMargins );
                }
                if ( std::abs( diff.y() ) < 4 * m_snapMargins )
                {
                    newFrameRect.moveBottom( snapToBottomRight.y() - m_snapMargins );
                }
            }
            m_widgetToMove->move( newFrameRect.topLeft() );
            return true;
        }
    }
    else if ( event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* mEv = static_cast<QMouseEvent*>( event );
        m_startPos       = mEv->pos();
        return true;
    }

    return false;
}
