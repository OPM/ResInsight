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

#include "RiaApplication.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"

#include <QLabel>
#include <QPainter>
#include <QTextDocument>
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
RiuAbstractOverlayContentFrame::~RiuAbstractOverlayContentFrame()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuAbstractOverlayContentFrame::updateFontSize()
{
    QFont font = this->font();
    font.setPointSize( caf::FontTools::pointSizeFromEnum( RiaPreferences::current()->defaultPlotFontSize() ) );
    setFont( font );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuTextOverlayContentFrame::RiuTextOverlayContentFrame( QWidget* parent /*= nullptr */ )
    : RiuAbstractOverlayContentFrame( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 4, 4, 4, 4 );
    m_textLabel = new QLabel;
    layout->addWidget( m_textLabel );

    updateLabelFont();
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
    updateLabelFont();

    painter->save();
    painter->translate( targetRect.topLeft() + QPoint( contentsMargins().left(), contentsMargins().top() ) );
    painter->setFont( m_textLabel->font() );

    QTextDocument td;
    td.setDefaultFont( m_textLabel->font() );
    td.setHtml( m_textLabel->text() );
    td.drawContents( painter );

    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTextOverlayContentFrame::updateLabelFont()
{
    QFont font = m_textLabel->font();
    font.setPointSize( caf::FontTools::pointSizeFromEnum( RiaPreferences::current()->defaultPlotFontSize() ) );
    m_textLabel->setFont( font );
}
