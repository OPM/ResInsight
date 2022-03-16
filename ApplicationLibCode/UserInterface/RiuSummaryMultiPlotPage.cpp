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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryMultiPlotPage::RiuSummaryMultiPlotPage( RimSummaryMultiPlot* plotDefinition, QWidget* parent )
    : RiuMultiPlotPage( plotDefinition, parent )
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
    clearGridLayout();

    auto titleFont = m_plotTitle->font();
    titleFont.setPixelSize( m_titleFontPixelSize );
    m_plotTitle->setFont( titleFont );

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        if ( m_plotWidgets[tIdx] )
        {
            m_plotWidgets[tIdx]->hide();
        }
        if ( m_legends[tIdx] )
        {
            m_legends[tIdx]->hide();
        }
        if ( m_subTitles[tIdx] )
        {
            m_subTitles[tIdx]->hide();
        }
    }

    QList<QPointer<QLabel>>           subTitles   = this->subTitlesForVisiblePlots();
    QList<QPointer<RiuQwtPlotLegend>> legends     = this->legendsForVisiblePlots();
    QList<QPointer<RiuPlotWidget>>    plotWidgets = this->visiblePlotWidgets();

    if ( !plotWidgets.empty() )
    {
        auto [rowCount, columnCount] = this->rowAndColumnCount( plotWidgets.size() );

        int row    = 0;
        int column = 0;
        for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
        {
            int expectedColSpan = static_cast<int>( plotWidgets[visibleIndex]->colSpan() );
            int colSpan         = std::min( expectedColSpan, columnCount );
            int rowSpan         = plotWidgets[visibleIndex]->rowSpan();

            std::tie( row, column ) = findAvailableRowAndColumn( row, column, colSpan, columnCount );

            m_gridLayout->addWidget( subTitles[visibleIndex], 3 * row, column, 1, colSpan );
            if ( legends[visibleIndex] )
            {
                m_gridLayout->addWidget( legends[visibleIndex], 3 * row + 1, column, 1, colSpan, Qt::AlignHCenter | Qt::AlignBottom );
            }
            m_gridLayout->addWidget( plotWidgets[visibleIndex], 3 * row + 2, column, 1 + ( rowSpan - 1 ) * 3, colSpan );

            subTitles[visibleIndex]->setVisible( m_showSubTitles );
            QFont subTitleFont = subTitles[visibleIndex]->font();
            subTitleFont.setPixelSize( m_subTitleFontPixelSize );
            subTitles[visibleIndex]->setFont( subTitleFont );

            plotWidgets[visibleIndex]->setAxisLabelsAndTicksEnabled( RiuPlotAxis::defaultLeft(),
                                                                     showYAxis( row, column ),
                                                                     showYAxis( row, column ) );
            plotWidgets[visibleIndex]->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), showYAxis( row, column ) );
            plotWidgets[visibleIndex]->setAxesFontsAndAlignment( m_axisTitleFontSize, m_axisValueFontSize );

            {
                auto margins = plotWidgets[visibleIndex]->contentsMargins();
                margins.setBottom( 40 );

                // Adjust the space below a graph to make sure the heading of the row below is closest to the
                // corresponding graph
                plotWidgets[visibleIndex]->setContentsMargins( margins );
            }

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
            for ( int r = row; r < row + rowSpan; ++r )
            {
                m_gridLayout->setRowStretch( 3 * r + 2, 1 );
            }
            for ( int c = column; c < column + colSpan; ++c )
            {
                int colStretch = 6; // Empirically chosen to try to counter the width of the axis on the first track
                if ( showYAxis( row, column ) ) colStretch += 1;
                m_gridLayout->setColumnStretch( c, std::max( colStretch, m_gridLayout->columnStretch( c ) ) );
            }
        }
    }
}
