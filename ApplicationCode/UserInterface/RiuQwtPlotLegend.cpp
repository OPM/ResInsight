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

#include <QDebug>
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlotLegend::sizeHint() const
{
    QSize fullSizeHint = QwtLegend::sizeHint();
    // Update width
    const QwtDynGridLayout* legendLayout = qobject_cast<QwtDynGridLayout*>( contentsWidget()->layout() );
    if ( legendLayout )
    {
        int rowCount = std::max( 1, (int)std::ceil( legendLayout->itemCount() / (double)std::max( m_columnCount, 1 ) ) );
        int maxHeight = 0;
        for ( unsigned int i = 0; i < legendLayout->itemCount(); ++i )
        {
            auto itemSize = legendLayout->itemAt( i )->sizeHint();
            int  width    = itemSize.width();
            fullSizeHint.setWidth( std::max( fullSizeHint.width(), width ) );
            maxHeight = std::max( maxHeight, itemSize.height() );
        }
        int totalSpacing = ( rowCount + 1 ) * legendLayout->spacing();
        fullSizeHint.setHeight(
            std::max( fullSizeHint.height(), static_cast<int>( maxHeight * rowCount + totalSpacing + 4 ) ) );
    }
    return fullSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotLegend::updateLegend( const QVariant& variant, const QList<QwtLegendData>& legendItems )
{
    QwtLegend::updateLegend( variant, legendItems );
    emit legendUpdated();
}
