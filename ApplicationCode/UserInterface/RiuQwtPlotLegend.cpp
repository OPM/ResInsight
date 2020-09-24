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
    , m_columnCount( 1 )
{
    QwtDynGridLayout* legendLayout = qobject_cast<QwtDynGridLayout*>( contentsWidget()->layout() );
    if ( legendLayout )
    {
        legendLayout->setExpandingDirections( Qt::Horizontal | Qt::Vertical );
    }
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );
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
        QMargins margins = this->contentsMargins();

        m_columnCount =
            std::max( 1, (int)legendLayout->columnsForWidth( size.width() - margins.left() - margins.right() ) );

        updateGeometry();
    }
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
        int numColumns = m_columnCount;
        int numRows    = legendLayout->itemCount() / numColumns;
        if ( numRows == 0 )
        {
            return QSize( 0, 0 );
        }
        else
        {
            if ( legendLayout->itemCount() % numColumns ) numRows++;

            int width = numColumns * legendLayout->maxItemWidth();

            int maxHeight = 0;
            for ( unsigned int i = 0; i < legendLayout->itemCount(); ++i )
            {
                auto itemSize = legendLayout->itemAt( i )->sizeHint();
                maxHeight     = std::max( maxHeight, itemSize.height() );
            }
            QMargins margins      = legendLayout->contentsMargins();
            int      totalSpacing = ( numRows + 2 ) * legendLayout->spacing() + margins.top() + margins.bottom();

            int height = maxHeight * numRows + totalSpacing;

            QSize layoutSize( width, height );
            QSize frameSize = layoutSize + QSize( 2 * frameWidth(), 2 * frameWidth() );

            fullSizeHint.setWidth( std::max( fullSizeHint.width(), frameSize.width() ) );
            fullSizeHint.setHeight( std::max( fullSizeHint.height(), frameSize.height() ) );
        }
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
