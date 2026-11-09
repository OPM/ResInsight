/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-  Equinor ASA
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

#include "RimWellTargetClusterStatistics.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimWellTargetClusterStatistics, "RimWellTargetClusterStatistics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellTargetClusterStatistics::RimWellTargetClusterStatistics()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Well Target Cluster Statistics",
                                                    "",
                                                    "",
                                                    "",
                                                    "WellTargetClusterStatistics",
                                                    "Statistics for one cluster found by a well target mapping. Totals are infinite "
                                                    "when the case does not have the result." );

    CAF_PDM_InitScriptableField( &m_clusterId, "ClusterId", -1, "Cluster Id", "", "", "Value of WELL_TARGET_NUM in the cluster cells" );
    CAF_PDM_InitScriptableField( &m_numCells, "NumCells", 0, "Number of Cells" );
    CAF_PDM_InitScriptableField( &m_totalPorvSoil, "TotalPorvSoil", 0.0, "Total PORV*SOIL" );
    CAF_PDM_InitScriptableField( &m_totalPorvSgas, "TotalPorvSgas", 0.0, "Total PORV*SGAS" );
    CAF_PDM_InitScriptableField( &m_totalPorvSoilAndSgas, "TotalPorvSoilAndSgas", 0.0, "Total PORV*(SOIL+SGAS)" );
    CAF_PDM_InitScriptableField( &m_totalFipOil, "TotalFipOil", 0.0, "Total FIPOIL" );
    CAF_PDM_InitScriptableField( &m_totalFipGas, "TotalFipGas", 0.0, "Total FIPGAS" );
    CAF_PDM_InitScriptableField( &m_totalRfipOil, "TotalRfipOil", 0.0, "Total RFIPOIL" );
    CAF_PDM_InitScriptableField( &m_totalRfipGas, "TotalRfipGas", 0.0, "Total RFIPGAS" );
    CAF_PDM_InitScriptableField( &m_totalSfipOil, "TotalSfipOil", 0.0, "Total SFIPOIL" );
    CAF_PDM_InitScriptableField( &m_totalSfipGas, "TotalSfipGas", 0.0, "Total SFIPGAS" );
    CAF_PDM_InitScriptableField( &m_permeability, "Permeability", 0.0, "Permeability", "", "", "Pore volume weighted mean of PERMX" );
    CAF_PDM_InitScriptableField( &m_pressure, "Pressure", 0.0, "Pressure", "", "", "Pore volume weighted mean of PRESSURE" );

    for ( auto field : fields() )
    {
        field->uiCapability()->setUiReadOnly( true );
        if ( auto scripting = field->capability<caf::PdmAbstractFieldScriptingCapability>() ) scripting->setIOWriteable( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetClusterStatistics::setStatistics( const RigWellTargetMapping::ClusterStatistics& statistics )
{
    m_clusterId            = statistics.id;
    m_numCells             = static_cast<int>( statistics.numCells );
    m_totalPorvSoil        = statistics.totalPorvSoil;
    m_totalPorvSgas        = statistics.totalPorvSgas;
    m_totalPorvSoilAndSgas = statistics.totalPorvSoilAndSgas;
    m_totalFipOil          = statistics.totalFipOil;
    m_totalFipGas          = statistics.totalFipGas;
    m_totalRfipOil         = statistics.totalRfipOil;
    m_totalRfipGas         = statistics.totalRfipGas;
    m_totalSfipOil         = statistics.totalSfipOil;
    m_totalSfipGas         = statistics.totalSfipGas;
    m_permeability         = statistics.permeability;
    m_pressure             = statistics.pressure;
}
