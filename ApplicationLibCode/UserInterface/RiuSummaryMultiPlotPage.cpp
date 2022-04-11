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

#include "RiuPlotWidget.h"
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

    QList<QPointer<QLabel>>           subTitles   = this->subTitlesForVisiblePlots();
    QList<QPointer<RiuQwtPlotLegend>> legends     = this->legendsForVisiblePlots();
    QList<QPointer<RiuPlotWidget>>    plotWidgets = this->visiblePlotWidgets();

    int visibleIndex = 0;
    int phIndex      = 0;

    for ( int row = 0; row < rows; row++ )
    {
        for ( int col = 0; col < cols; col++ )
        {
            if ( visibleIndex >= nPlots )
            {
                m_gridLayout->addWidget( m_placeholderWidgets[phIndex], row * 3 + 2, col );
                m_gridLayout->setRowStretch( row * 3 + 2, 1 );
                m_gridLayout->setColumnStretch( col, 6 );
                m_placeholderWidgets[phIndex]->show();
                phIndex++;
                continue;
            }

            int expectedColSpan = plotWidgets[visibleIndex]->colSpan();
            int colSpan         = std::min( expectedColSpan, cols - col );

            m_gridLayout->addWidget( subTitles[visibleIndex], 3 * row, col, 1, colSpan );
            if ( legends[visibleIndex] )
            {
                m_gridLayout->addWidget( legends[visibleIndex], 3 * row + 1, col, 1, colSpan, Qt::AlignHCenter | Qt::AlignBottom );
            }
            m_gridLayout->addWidget( plotWidgets[visibleIndex], 3 * row + 2, col, 1, colSpan );

            subTitles[visibleIndex]->setVisible( m_showSubTitles );
            QFont subTitleFont = subTitles[visibleIndex]->font();
            subTitleFont.setPixelSize( m_subTitleFontPixelSize );
            subTitles[visibleIndex]->setFont( subTitleFont );

            plotWidgets[visibleIndex]->setAxisLabelsAndTicksEnabled( RiuPlotAxis::defaultLeft(),
                                                                     showYAxis( row, col ),
                                                                     showYAxis( row, col ) );
            plotWidgets[visibleIndex]->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), showYAxis( row, col ) );
            plotWidgets[visibleIndex]->setAxesFontsAndAlignment( m_axisTitleFontSize, m_axisValueFontSize );

            // Adjust the space below a graph to make sure the heading of the row below is closest to the
            // corresponding graph
            auto margins = plotWidgets[visibleIndex]->contentsMargins();
            margins.setBottom( 40 );
            plotWidgets[visibleIndex]->setContentsMargins( margins );
            plotWidgets[visibleIndex]->show();

            if ( legends[visibleIndex] )
            {
                if ( m_plotDefinition->legendsVisible() )
                {
                    int legendColumns = 1;
                    if ( m_plotDefinition->legendsHorizontal() )
                    {
                        legendColumns = 0; // unlimited
                    }
                    legends[visibleIndex]->setMaxColumns( legendColumns );
                    QFont legendFont = legends[visibleIndex]->font();
                    legendFont.setPixelSize( m_legendFontPixelSize );
                    legends[visibleIndex]->setFont( legendFont );
                    legends[visibleIndex]->show();
                }
                else
                {
                    legends[visibleIndex]->hide();
                }
            }
            // Set basic row and column stretches
            m_gridLayout->setRowStretch( 3 * row + 2, 1 );
            for ( int c = col; c < col + colSpan; c++ )
            {
                int colStretch = 6; // Empirically chosen to try to counter the width of the axis on the first track
                if ( showYAxis( row, col ) ) colStretch += 1;
                m_gridLayout->setColumnStretch( c, std::max( colStretch, m_gridLayout->columnStretch( c ) ) );
            }

            visibleIndex++;
            col += colSpan - 1;
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
