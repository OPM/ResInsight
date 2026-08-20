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

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QTextDocument>
#include <QTimer>
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSpinnerOverlayContentFrame::RiuSpinnerOverlayContentFrame( QWidget* parent /*= nullptr */ )
    : RiuAbstractOverlayContentFrame( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    // Room for the spinner, which is painted rather than laid out: it has no content of its own to size it,
    // and reserving the space keeps it from ending up under the text.
    layout->setContentsMargins( 4 + spinnerSize() + spinnerMargin(), 4, 4, 4 );

    m_textLabel = new QLabel;
    layout->addWidget( m_textLabel );

    m_animationTimer = new QTimer( this );
    m_animationTimer->setInterval( 50 );

    QObject::connect( m_animationTimer,
                      &QTimer::timeout,
                      this,
                      [this]()
                      {
                          m_angleDegrees = ( m_angleDegrees + 30 ) % 360;
                          update();
                      } );

    updateLabelFont();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSpinnerOverlayContentFrame::setText( const QString& text )
{
    m_textLabel->setText( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuSpinnerOverlayContentFrame::spinnerSize()
{
    return 14;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuSpinnerOverlayContentFrame::spinnerMargin()
{
    return 6;
}

//--------------------------------------------------------------------------------------------------
/// Animate only while on screen. A frame taken off a plot keeps its timer, and a timer left running would
/// wake the application up several times a second for something nobody is looking at.
//--------------------------------------------------------------------------------------------------
void RiuSpinnerOverlayContentFrame::showEvent( QShowEvent* event )
{
    RiuAbstractOverlayContentFrame::showEvent( event );

    m_animationTimer->start();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSpinnerOverlayContentFrame::hideEvent( QHideEvent* event )
{
    RiuAbstractOverlayContentFrame::hideEvent( event );

    m_animationTimer->stop();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSpinnerOverlayContentFrame::paintEvent( QPaintEvent* event )
{
    RiuAbstractOverlayContentFrame::paintEvent( event );

    QPainter painter( this );
    drawSpinner( &painter, QPoint( 4, ( height() - spinnerSize() ) / 2 ) );
}

//--------------------------------------------------------------------------------------------------
/// An arc left open at one end, turned a step further on every tick. The gap is what makes the turning
/// visible: a full circle would look the same at every angle.
//--------------------------------------------------------------------------------------------------
void RiuSpinnerOverlayContentFrame::drawSpinner( QPainter* painter, const QPoint& topLeft ) const
{
    painter->save();
    painter->setRenderHint( QPainter::Antialiasing );

    QPen pen( palette().color( QPalette::WindowText ), 2.0, Qt::SolidLine, Qt::RoundCap );
    painter->setPen( pen );
    painter->setBrush( Qt::NoBrush );

    // Qt angles are in sixteenths of a degree and turn counterclockwise, so the sign makes the arc turn the
    // way a clock does. Inset by the pen width, otherwise the stroke is drawn half outside the rectangle.
    const QRect arcRect( topLeft.x() + 1, topLeft.y() + 1, spinnerSize() - 2, spinnerSize() - 2 );
    painter->drawArc( arcRect, -m_angleDegrees * 16, 300 * 16 );

    painter->restore();
}

//--------------------------------------------------------------------------------------------------
/// Drawn into snapshots as it looks at this moment. There is no animation in a still image, but leaving the
/// spinner out would make a plot that was still loading look finished.
//--------------------------------------------------------------------------------------------------
void RiuSpinnerOverlayContentFrame::renderTo( QPainter* painter, const QRect& targetRect )
{
    updateLabelFont();

    painter->save();
    painter->translate( targetRect.topLeft() );

    drawSpinner( painter, QPoint( 4, ( targetRect.height() - spinnerSize() ) / 2 ) );

    painter->translate( contentsMargins().left(), contentsMargins().top() );
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
void RiuSpinnerOverlayContentFrame::updateLabelFont()
{
    QFont font = m_textLabel->font();
    font.setPointSize( caf::FontTools::pointSizeFromEnum( RiaPreferences::current()->defaultPlotFontSize() ) );
    m_textLabel->setFont( font );
}
