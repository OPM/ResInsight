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

    QList<QPointer<RiuPlotWidget>> plotWidgets = this->visiblePlotWidgets();
    auto [rowCount, columnCount]               = this->rowAndColumnCount( plotWidgets.size() );

    int rowsPerPage = m_plotDefinition->rowsPerPage();
    int row         = 0;
    int column      = 0;

    // Make sure we always add a page. For empty multi-plots we'll have an empty page with a drop target.
    RiuMultiPlotPage* page = createPage();

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int expectedColSpan = static_cast<int>( plotWidgets[visibleIndex]->colSpan() );
        int colSpan         = std::min( expectedColSpan, columnCount );

        // std::tie( row, column ) = page->findAvailableRowAndColumn( row, column, colSpan, columnCount );
        // if ( row >= rowsPerPage )
        //{
        //    page   = createPage();
        //    row    = 0;
        //    column = 0;
        //}
        // CAF_ASSERT( plotWidgets[visibleIndex] );
        // page->addPlot( plotWidgets[visibleIndex] );
        // page->performUpdate( RiaDefines::MultiPlotPageUpdateType::ALL );
    }

    // Set page numbers in title when there's more than one page
    if ( m_pages.size() > 1 )
    {
        for ( int i = 0; i < m_pages.size(); ++i )
        {
            int pageNumber = i + 1;
            m_pages[i]->setPlotTitle( QString( "%1 %2/%3" ).arg( m_plotTitle ).arg( pageNumber ).arg( m_pages.size() ) );
        }
    }
    m_book->adjustSize();
}
