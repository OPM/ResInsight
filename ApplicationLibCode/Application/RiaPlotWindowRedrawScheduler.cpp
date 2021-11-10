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

#include <QCoreApplication>
#include <QDebug>

#include <set>

#include "cafProgressState.h"

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
void RiaPlotWindowRedrawScheduler::scheduleMultiPlotWindowUpdate( RiuMultiPlotBook* plotWindow )
{
    m_plotWindowsToUpdate.push_back( plotWindow );

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::scheduleMultiPlotPageUpdate( RiuMultiPlotPage* plotPage )
{
    m_plotPagesToUpdate.push_back( plotPage );

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::schedulePlotWidgetReplot( RiuPlotWidget* plotWidget )
{
    m_plotWidgetsToReplot.push_back( plotWidget );

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::clearAllScheduledUpdates()
{
    if ( m_plotWindowUpdateTimer )
    {
        while ( m_plotWindowUpdateTimer->isActive() )
        {
            QCoreApplication::processEvents();
        }
    }
    m_plotWidgetsToReplot.clear();
    m_plotPagesToUpdate.clear();
    m_plotWindowsToUpdate.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::performScheduledUpdatesAndReplots()
{
    std::vector<QPointer<RiuMultiPlotBook>> plotWindowsToUpdate;
    std::vector<QPointer<RiuMultiPlotPage>> plotPagesToUpdate;
    std::vector<QPointer<RiuPlotWidget>>    plotWidgetsToReplot;

    plotWindowsToUpdate.swap( m_plotWindowsToUpdate );
    plotPagesToUpdate.swap( m_plotPagesToUpdate );
    plotWidgetsToReplot.swap( m_plotWidgetsToReplot );

    std::set<QPointer<RiuPlotWidget>>    updatedPlots;
    std::set<QPointer<RiuMultiPlotBook>> updatedPlotWindows;
    std::set<QPointer<RiuMultiPlotPage>> updatedPlotPages;

    for ( QPointer<RiuMultiPlotBook> plotWindow : plotWindowsToUpdate )
    {
        if ( !plotWindow.isNull() && !updatedPlotWindows.count( plotWindow ) )
        {
            for ( RiuMultiPlotPage* page : plotWindow->pages() )
            {
                plotPagesToUpdate.erase( std::remove( plotPagesToUpdate.begin(), plotPagesToUpdate.end(), page ),
                                         plotPagesToUpdate.end() );
            }

            plotWindow->performUpdate();
            updatedPlotWindows.insert( plotWindow );
        }
    }

    for ( QPointer<RiuMultiPlotPage> plotPage : plotPagesToUpdate )
    {
        if ( !plotPage.isNull() && !updatedPlotPages.count( plotPage ) )
        {
            plotPage->performUpdate();
            updatedPlotPages.insert( plotPage );
        }
    }

    //  Perform update and replot. Make sure we handle legend update
    for ( QPointer<RiuPlotWidget> plot : plotWidgetsToReplot )
    {
        if ( !plot.isNull() && !updatedPlots.count( plot ) )
        {
            plot->replot();
            updatedPlots.insert( plot );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::slotUpdateAndReplotScheduledItemsWhenReady()
{
    if ( caf::ProgressState::isActive() )
    {
        startTimer( 100 );
        return;
    }

    performScheduledUpdatesAndReplots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::startTimer( int msecs )
{
    if ( !m_plotWindowUpdateTimer )
    {
        m_plotWindowUpdateTimer.reset( new QTimer( this ) );
        connect( m_plotWindowUpdateTimer.data(),
                 SIGNAL( timeout() ),
                 this,
                 SLOT( slotUpdateAndReplotScheduledItemsWhenReady() ) );
    }

    if ( !m_plotWindowUpdateTimer->isActive() )
    {
        m_plotWindowUpdateTimer->setSingleShot( true );
        m_plotWindowUpdateTimer->start( msecs );
    }
}
