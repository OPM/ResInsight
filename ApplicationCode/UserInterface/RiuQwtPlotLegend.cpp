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
#include "RiuQwtPlotLegend.h"

#include "qwt_dyngrid_layout.h"

#include <QResizeEvent>

#include <utility>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotLegend::RiuQwtPlotLegend( QWidget* parent /*= nullptr */ )
    : QwtLegend( parent )
    , m_columnCount( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotLegend::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent( event );
    QSize                   size         = event->size();
    const QwtDynGridLayout* legendLayout = qobject_cast<QwtDynGridLayout*>( contentsWidget()->layout() );
    if ( legendLayout )
    {
        m_columnCount = legendLayout->columnsForWidth( size.width() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotLegend::columnCount() const
{
    return m_columnCount;
}
