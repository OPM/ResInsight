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

#include "RiaPlotWindowRedrawScheduler.h"

#include "RiuMultiPlotBook.h"
#include "RiuMultiPlotPage.h"
#include "RiuPlotWidget.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPlotWindowRedrawScheduler* RiaPlotWindowRedrawScheduler::instance()
{
    static RiaPlotWindowRedrawScheduler theInstance;

    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::scheduleMultiPlotBookUpdate( RiuMultiPlotBook* plotBook, RiaDefines::MultiPlotPageUpdateType updateType )
{
    if ( m_plotBooksToUpdate.count( plotBook ) == 0 )
    {
        m_plotBooksToUpdate[plotBook] = updateType;
    }
    else
    {
        m_plotBooksToUpdate[plotBook] = m_plotBooksToUpdate[plotBook] | updateType;
    }

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::scheduleMultiPlotPageUpdate( RiuMultiPlotPage* plotPage, RiaDefines::MultiPlotPageUpdateType updateType )
{
    if ( m_plotPagesToUpdate.count( plotPage ) == 0 )
    {
        m_plotPagesToUpdate[plotPage] = updateType;
    }
    else
    {
        m_plotPagesToUpdate[plotPage] = m_plotPagesToUpdate[plotPage] | updateType;
    }

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::schedulePlotWidgetReplot( RiuPlotWidget* plotWidget )
{
    m_plotWidgetsToReplot.insert( plotWidget );

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::clearAllScheduledUpdates()
{
    m_plotWidgetsToReplot.clear();
    m_plotPagesToUpdate.clear();
    m_plotBooksToUpdate.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::performScheduledUpdates()
{
    std::map<QPointer<RiuMultiPlotBook>, RiaDefines::MultiPlotPageUpdateType> plotBooksToUpdate;
    std::map<QPointer<RiuMultiPlotPage>, RiaDefines::MultiPlotPageUpdateType> pagesToUpdate;

    pagesToUpdate.swap( m_plotPagesToUpdate );
    plotBooksToUpdate.swap( m_plotBooksToUpdate );

    for ( auto& [plotBook, updateType] : plotBooksToUpdate )
    {
        if ( plotBook.isNull() ) continue;

        if ( RiaDefines::isPlotUpdate( updateType ) )
        {
            for ( RiuMultiPlotPage* page : plotBook->pages() )
            {
                if ( pagesToUpdate.count( page ) > 0 ) pagesToUpdate.erase( page );
            }
        }
        plotBook->performUpdate( updateType );
    }

    for ( auto& [page, updateType] : pagesToUpdate )
    {
        if ( page.isNull() ) continue;

        page->performUpdate( updateType );
    }

    // PERFORMANCE NOTE
    // As the book and page updates can trigger widget updates, make sure to get the list of widgets to replot after
    // these updates
    std::set<QPointer<RiuPlotWidget>> plotWidgetsToReplot;
    plotWidgetsToReplot.swap( m_plotWidgetsToReplot );

    for ( const QPointer<RiuPlotWidget>& plot : plotWidgetsToReplot )
    {
        if ( !plot.isNull() )
        {
            plot->replot();
        }
    }
}
