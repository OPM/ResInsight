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
#include "RiuSummaryMultiPlotBook.h"

#include "RimSummaryMultiPlot.h"

#include "RiuSummaryMultiPlotPage.h"

#include "RiuPlotWidget.h"

#include <QDebug>
#include <QList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryMultiPlotBook::RiuSummaryMultiPlotBook( RimSummaryMultiPlot* plotDefinition, QWidget* parent )
    : RiuMultiPlotBook( plotDefinition, parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryMultiPlotBook::~RiuSummaryMultiPlotBook()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryMultiPlotBook::createPages()
{
    CAF_ASSERT( m_plotDefinition );

    QList<QPointer<RiuPlotWidget>> plotWidgets = visiblePlotWidgets();

    int columns     = std::max( 1, m_plotDefinition->columnCount() );
    int rowsPerPage = m_plotDefinition->rowsPerPage();

    int row = 0;
    int col = 0;

    auto summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( m_plotDefinition.p() );
    if ( summaryMultiPlot ) summaryMultiPlot->clearLayoutInfo();

    RiuSummaryMultiPlotPage* page = createSummaryPage();

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int expectedColSpan = static_cast<int>( plotWidgets[visibleIndex]->colSpan() );
        int colSpan         = std::min( expectedColSpan, columns );

        if ( row >= rowsPerPage )
        {
            row  = 0;
            page = createSummaryPage();
        }

        page->addPlot( plotWidgets[visibleIndex] );

        col += colSpan;
        if ( col >= columns )
        {
            row++;
            col = 0;
        }
    }

    // Set page numbers in title when there's more than one page
    updatePageTitles();
    adjustBookFrame();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryMultiPlotPage* RiuSummaryMultiPlotBook::createSummaryPage()
{
    RimSummaryMultiPlot*     sumMultPlot = dynamic_cast<RimSummaryMultiPlot*>( m_plotDefinition.p() );
    RiuSummaryMultiPlotPage* page        = new RiuSummaryMultiPlotPage( sumMultPlot, this );

    applyPageSettings( page );

    m_pages.push_back( page );
    m_bookLayout->addWidget( page );

    page->setVisible( true );
    return page;
}
