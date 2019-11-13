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

#include "RiuMultiPlotWindow.h"
#include "RiuQwtPlotWidget.h"

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
void RiaPlotWindowRedrawScheduler::schedulePlotWindowUpdate( RiuMultiPlotWindow* plotWindow )
{
    m_plotWindowsToUpdate.push_back( plotWindow );

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::schedulePlotWidgetReplot( RiuQwtPlotWidget* plotWidget )
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
    m_plotWindowsToUpdate.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::performScheduledUpdatesAndReplots()
{
    std::set<RiuQwtPlotWidget*>   updatedPlots;
    std::set<RiuMultiPlotWindow*> updatedPlotWindows;

    for ( RiuMultiPlotWindow* plotWindow : m_plotWindowsToUpdate )
    {
        if ( plotWindow && !updatedPlotWindows.count( plotWindow ) )
        {
            plotWindow->performUpdate();
            updatedPlotWindows.insert( plotWindow );
        }
    }

    //  Perform update and replot. Make sure we handle legend update
    for ( RiuQwtPlotWidget* plot : m_plotWidgetsToReplot )
    {
        if ( plot && !updatedPlots.count( plot ) )
        {
            plot->replot();
            updatedPlots.insert( plot );
        }
    }

    m_plotWidgetsToReplot.clear();
    m_plotWindowsToUpdate.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotWindowRedrawScheduler::slotUpdateAndReplotScheduledItemsWhenReady()
{
    if ( caf::ProgressState::isActive() )
    {
        startTimer( 10 );
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
