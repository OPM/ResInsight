/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RiuSummaryMultiPlotPage.h"

#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuPlotWidget.h"
#include "RiuQwtLegendOverlayContentFrame.h"
#include "RiuQwtPlotLegend.h"

#include <QLabel>
#include <QWidget>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryMultiPlotPage::RiuSummaryMultiPlotPage( RimSummaryMultiPlot* plotDefinition, QWidget* parent )
    : RiuMultiPlotPage( plotDefinition, parent )
    , m_summaryMultiPlot( plotDefinition )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryMultiPlotPage::~RiuSummaryMultiPlotPage()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryMultiPlotPage::reinsertPlotWidgets()
{
    if ( m_gridLayout )
    {
        for ( int phIdx = 0; phIdx < m_placeholderWidgets.size(); phIdx++ )
        {
            m_gridLayout->removeWidget( m_placeholderWidgets[phIdx] );
            m_placeholderWidgets[phIdx]->hide();
        }
    }

    clearGridLayout();
    updateTitleFont();

    int cols   = m_summaryMultiPlot->columnCount();
    int rows   = m_summaryMultiPlot->rowsPerPage();
    int nPlots = visiblePlotWidgets().size();

    int nCells = cols * rows;
    reservePlaceholders( nCells - nPlots );

    QList<QPointer<QLabel>>                   subTitles    = subTitlesForVisiblePlots();
    QList<QPointer<RiuQwtPlotLegend>>         legends      = legendsForVisiblePlots();
    QList<QPointer<RiuDraggableOverlayFrame>> legendFrames = legendFramesForVisiblePlots();
    QList<QPointer<RiuPlotWidget>>            plotWidgets  = visiblePlotWidgets();

    m_visibleIndexToPositionMapping.clear();

    int visibleIndex = 0;
    int phIndex      = 0;

    for ( int row = 0; row < rows; row++ )
    {
        for ( int column = 0; column < cols; column++ )
        {
            if ( visibleIndex >= nPlots )
            {
                m_gridLayout->addWidget( m_placeholderWidgets[phIndex], row * 3 + 2, column );
                m_gridLayout->setRowStretch( row * 3 + 2, 1 );
                m_gridLayout->setColumnStretch( column, 6 );
                m_placeholderWidgets[phIndex]->show();
                phIndex++;
                continue;
            }

            auto plotWidget  = plotWidgets[visibleIndex];
            auto legend      = legends[visibleIndex];
            auto legendFrame = legendFrames[visibleIndex];
            auto subTitle    = subTitles[visibleIndex];

            int expectedColSpan = std::max( 1, plotWidget->colSpan() );
            int colSpan         = std::min( expectedColSpan, cols - column );
            int rowSpan         = 1;

            m_visibleIndexToPositionMapping[visibleIndex] = std::make_pair( row, column );

            reinsertPlotWidget( plotWidget, legend, legendFrame, subTitle, row, column, rowSpan, colSpan );

            auto summaryPlot = dynamic_cast<RimSummaryPlot*>( plotWidget->plotDefinition() );
            if ( summaryPlot ) m_summaryMultiPlot->setLayoutInfo( summaryPlot, row, column );

            visibleIndex++;
            column += colSpan - 1;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryMultiPlotPage::reservePlaceholders( int count )
{
    while ( m_placeholderWidgets.size() < count )
    {
        m_placeholderWidgets.push_back( new QWidget( this ) );
    }
}
