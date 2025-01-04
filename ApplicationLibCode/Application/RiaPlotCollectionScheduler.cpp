/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RiaPlotCollectionScheduler.h"

#include "RimAbstractPlotCollection.h"
#include "RimViewWindow.h"

#include <QTimer>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPlotCollectionScheduler::RiaPlotCollectionScheduler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPlotCollectionScheduler::~RiaPlotCollectionScheduler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPlotCollectionScheduler* RiaPlotCollectionScheduler::instance()
{
    static RiaPlotCollectionScheduler theInstance;

    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotCollectionScheduler::schedulePlotCollectionUpdate( const std::vector<RimPlotCollection*> plotCollections )
{
    m_plotCollectionsToUpdate.insert( m_plotCollectionsToUpdate.end(), plotCollections.begin(), plotCollections.end() );

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPlotCollectionScheduler::performScheduledUpdates()
{
    for ( auto p : m_plotCollectionsToUpdate )
    {
        if ( p == nullptr ) continue;

        p->loadDataAndUpdateAllPlots();
    }
}
