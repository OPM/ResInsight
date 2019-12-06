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
#include "RiuAbstractOverlayContentFrame.h"

#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuAbstractOverlayContentFrame::RiuAbstractOverlayContentFrame( QWidget* parent /*= nullptr */ )
    : QFrame( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuAbstractOverlayContentFrame::~RiuAbstractOverlayContentFrame() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuTextOverlayContentFrame::RiuTextOverlayContentFrame( QWidget* parent /*= nullptr */ )
    : RiuAbstractOverlayContentFrame( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 8, 8, 8, 8 );
    m_textLabel = new QLabel;
    layout->addWidget( m_textLabel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextOverlayContentFrame::setText( const QString& text )
{
    m_textLabel->setText( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextOverlayContentFrame::renderTo( QPainter* painter, const QRect& targetRect )
{
    painter->save();

    painter->setFont( m_textLabel->font() );
    painter->drawText( targetRect, m_textLabel->text() );

    painter->restore();
}
