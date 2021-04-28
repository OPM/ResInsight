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

#include "RimEnsembleFractureStatisticsPlotCollection.h"

#include "RimEnsembleFractureStatisticsPlot.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEnsembleFractureStatisticsPlotCollection, "EnsembleFractureStatisticsPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatisticsPlotCollection::RimEnsembleFractureStatisticsPlotCollection()
{
    CAF_PDM_InitScriptableObject( "Ensemble Fracture Statistics Plots", ":/WellLogPlots16x16.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_ensembleFractureStatisticsPlots, "EnsembleFractureStatisticsPlots", "", "", "", "" );
    m_ensembleFractureStatisticsPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsPlotCollection::reloadAllPlots()
{
    for ( const auto& w : m_ensembleFractureStatisticsPlots() )
    {
        w->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsPlotCollection::addEnsembleFractureStatisticsPlot( RimEnsembleFractureStatisticsPlot* newPlot )
{
    m_ensembleFractureStatisticsPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleFractureStatisticsPlot*>
    RimEnsembleFractureStatisticsPlotCollection::ensembleFractureStatisticsPlots() const
{
    return m_ensembleFractureStatisticsPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatisticsPlotCollection::deleteAllPlots()
{
    m_ensembleFractureStatisticsPlots.deleteAllChildObjects();
}
