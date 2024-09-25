/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimVfpDataCollection.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimVfpPlotCollection.h"
#include "RimVfpTableData.h"

CAF_PDM_SOURCE_INIT( RimVfpDataCollection, "RimVfpDataCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDataCollection::RimVfpDataCollection()
{
    CAF_PDM_InitObject( "VFP Data", ":/VfpPlotCollection.svg" );

    CAF_PDM_InitFieldNoDefault( &m_vfpTableData, "VfpPlots", "Vertical Flow Performance Data" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDataCollection* RimVfpDataCollection::instance()
{
    return RimProject::current()->activeOilField()->vfpDataCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpTable* RimVfpDataCollection::appendTableDataObject( const QString& fileName )
{
    auto* vfpTableData = new RimVfpTableData();
    vfpTableData->setFileName( fileName );
    m_vfpTableData.push_back( vfpTableData );

    vfpTableData->ensureDataIsImported();
    auto dataSources = vfpTableData->tableDataSources();

    if ( !dataSources.empty() )
    {
        return dataSources.front();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimVfpTable*> RimVfpDataCollection::vfpTableData() const
{
    std::vector<RimVfpTable*> tableDataSources;

    for ( auto vfpTableData : m_vfpTableData.childrenByType() )
    {
        for ( auto table : vfpTableData->tableDataSources() )
        {
            tableDataSources.push_back( table );
        }
    }

    return tableDataSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDataCollection::loadDataAndUpdate()
{
    for ( auto vfpTableData : m_vfpTableData.childrenByType() )
    {
        vfpTableData->ensureDataIsImported();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDataCollection::deleteAllData()
{
    m_vfpTableData.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDataCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    RimVfpPlotCollection::addImportItems( menuBuilder );
}
