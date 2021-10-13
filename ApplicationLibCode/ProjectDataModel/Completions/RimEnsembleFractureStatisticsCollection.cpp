/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleFractureStatisticsCollection.h"

#include "RimEnsembleFractureStatistics.h"

CAF_PDM_SOURCE_INIT( RimEnsembleFractureStatisticsCollection, "FractureGroupStatisticsCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatisticsCollection::RimEnsembleFractureStatisticsCollection()
{
    CAF_PDM_InitObject( "Ensemble Fracture Statistics", ":/FractureTemplates16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureGroupStatistics, "FractureGroupStatistics", "", "", "", "" );
    m_fractureGroupStatistics.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsCollection::addFractureGroupStatistics( RimEnsembleFractureStatistics* fractureGroupStatistics )
{
    m_fractureGroupStatistics.push_back( fractureGroupStatistics );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsCollection::loadAndUpdateData()
{
    for ( auto f : m_fractureGroupStatistics.childObjects() )
    {
        f->loadAndUpdateData();
    }
}
