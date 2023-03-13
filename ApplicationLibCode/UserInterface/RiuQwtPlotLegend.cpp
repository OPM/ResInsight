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

#include "RiuQwtPlotWidget.h"

#include "qwt_dyngrid_layout.h"
#include "qwt_legend_label.h"
#include "qwt_plot.h"
#include "qwt_plot_item.h"

#include <QDebug>
#include <QResizeEvent>
#include <QVariant>

#include <utility>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotLegend::RiuQwtPlotLegend( QWidget* parent /*= nullptr */ )
    : QwtLegend( parent )

{
    auto* legendLayout = qobject_cast<QwtDynGridLayout*>( contentsWidget()->layout() );
    if ( legendLayout )
    {
        legendLayout->setExpandingDirections( Qt::Horizontal | Qt::Vertical );
        legendLayout->setSpacing( 1 );
    }
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotLegend::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent( event );

    QSize size = event->size();

    // Avoid updating geometry if height is very small
    if ( size.height() < 10 ) return;

    const QwtDynGridLayout* legendLayout = qobject_cast<QwtDynGridLayout*>( contentsWidget()->layout() );
    if ( legendLayout )
    {
        updateGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlotLegend::sizeHint() const
{
    QSize fullSizeHint = QwtLegend::sizeHint();

    const QwtDynGridLayout* legendLayout = qobject_cast<QwtDynGridLayout*>( contentsWidget()->layout() );
    if ( legendLayout )
    {
        QMargins margins = this->contentsMargins();

        auto widgetSize = size();

        int numColumns = std::max( 1, (int)legendLayout->columnsForWidth( widgetSize.width() - margins.left() - margins.right() ) );
        int numRows    = legendLayout->itemCount() / numColumns;
        if ( numRows == 0 )
        {
            return { 0, 0 };
        }

        if ( legendLayout->itemCount() % numColumns ) numRows++;

        int width = numColumns * legendLayout->maxItemWidth();

        int maxHeight = 0;
        for ( unsigned int i = 0; i < legendLayout->itemCount(); ++i )
        {
            auto itemSize = legendLayout->itemAt( i )->sizeHint();
            maxHeight     = std::max( maxHeight, itemSize.height() );
        }
        int totalSpacing = ( numRows + 2 ) * legendLayout->spacing() + margins.top() + margins.bottom();

        int height = maxHeight * numRows + totalSpacing;

        QSize layoutSize( width, height );
        QSize frameSize = layoutSize + QSize( 2 * frameWidth(), 2 * frameWidth() );

        fullSizeHint.setWidth( std::max( fullSizeHint.width(), frameSize.width() ) );
        fullSizeHint.setHeight( std::max( fullSizeHint.height(), frameSize.height() ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotLegend::updateLegend( const QList<QwtLegendData>& legendData )
{
    // Delete all existing widgets
    deleteAll();

    // Create legend widgets based on legendData
    updateLegend( QVariant(), legendData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotLegend::deleteAll()
{
    auto widgets = contentsWidget()->findChildren<QwtLegendLabel*>();
    for ( auto w : widgets )
    {
        w->hide();
        w->deleteLater();
    }
}
