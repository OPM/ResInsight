/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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
#include "RiuQwtLegendOverlayContentFrame.h"

#include <QPainter>
#include <QTextDocument>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtLegendOverlayContentFrame::RiuQwtLegendOverlayContentFrame( QWidget* parent )
    : RiuAbstractOverlayContentFrame( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 4, 4, 4, 4 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtLegendOverlayContentFrame::setLegend( QwtLegend* legend )
{
    m_legend = legend;
    layout()->addWidget( legend );
    legend->adjustSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtLegendOverlayContentFrame::renderTo( QPainter* painter, const QRect& targetRect )
{
    painter->save();
    painter->translate( targetRect.topLeft() + QPoint( this->contentsMargins().left(), this->contentsMargins().top() ) );

    QRegion sourceRegion = visibleRegion();
    render( painter, QPoint(), sourceRegion );

    painter->restore();
}
