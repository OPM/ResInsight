/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiuDraggableOverlayFrame.h"
#include "RiuWidgetDragger.h"

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDraggableOverlayFrame::RiuDraggableOverlayFrame( QWidget* parent, const int snapMargins, const QColor& backgroundColor )
    : QFrame( parent )
    , m_anchorCorner( AnchorCorner::TopLeft )
{
    m_widgetDragger = new RiuWidgetDragger( this, snapMargins );

    setFrameShape( QFrame::Box );

    QGraphicsDropShadowEffect* dropShadowEffect = new QGraphicsDropShadowEffect( this );
    dropShadowEffect->setOffset( 1.0, 1.0 );
    dropShadowEffect->setBlurRadius( 3.0 );
    dropShadowEffect->setColor( QColor( 100, 100, 100, 100 ) );
    setGraphicsEffect( dropShadowEffect );
    this->setContentsMargins( 1, 1, 1, 1 );
    m_layout = new QVBoxLayout( this );
    m_layout->setContentsMargins( 0, 0, 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuAbstractOverlayContentFrame* RiuDraggableOverlayFrame::contentFrame()
{
    return m_contentFrame;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDraggableOverlayFrame::setContentFrame( RiuAbstractOverlayContentFrame* contentFrame )
{
    if ( m_contentFrame )
    {
        m_layout->removeWidget( m_contentFrame );
        m_contentFrame->setParent( nullptr ); // TODO: check if both removeWidget and setParent is necessary
        delete m_contentFrame;
        m_contentFrame = nullptr;
    }

    m_contentFrame = contentFrame;
    m_layout->addWidget( m_contentFrame );
    this->adjustSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDraggableOverlayFrame::renderTo( QPainter* painter, const QRect& targetRect )
{
    if ( m_contentFrame )
    {
        painter->save();
        painter->fillRect( targetRect, this->palette().color( QWidget::backgroundRole() ) );
        QRect contentRect = targetRect;
        contentRect.adjust( -1, -1, -1, -1 );
        m_contentFrame->renderTo( painter, contentRect );
        painter->drawRect( targetRect );
        painter->restore();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDraggableOverlayFrame::setAnchorCorner( AnchorCorner corner )
{
    m_anchorCorner = corner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDraggableOverlayFrame::AnchorCorner RiuDraggableOverlayFrame::anchorCorner() const
{
    return m_anchorCorner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuDraggableOverlayFrame::sizeHint() const
{
    if ( m_contentFrame )
    {
        QSize contentSize = m_contentFrame->sizeHint();
        return QSize( contentSize.width() + 2, contentSize.height() + 2 );
    }
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuDraggableOverlayFrame::minimumSizeHint() const
{
    if ( m_contentFrame )
    {
        QSize contentSize = m_contentFrame->minimumSizeHint();
        return QSize( contentSize.width() + 2, contentSize.height() + 2 );
    }
    return QSize( 0, 0 );
}
