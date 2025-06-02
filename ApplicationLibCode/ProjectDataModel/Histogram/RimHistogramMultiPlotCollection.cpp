/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimHistogramMultiPlotCollection.h"

#include "RimHistogramMultiPlot.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimHistogramMultiPlotCollection, "RimHistogramMultiPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlotCollection::RimHistogramMultiPlotCollection()
{
    CAF_PDM_InitObject( "Histogram Plots", ":/VfpPlotCollection.svg" );

    CAF_PDM_InitFieldNoDefault( &m_histogramMultiPlots, "HistogramMultiPlots", "Histogram Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlotCollection* RimHistogramMultiPlotCollection::instance()
{
    return RimProject::current()->mainPlotCollection()->histogramMultiPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlot* RimHistogramMultiPlotCollection::appendTableDataObject( const QString& fileName )
{
    auto* vfpTableData = new RimHistogramMultiPlot();
    // vfpTableData->setFileName( fileName );
    m_histogramMultiPlots.push_back( vfpTableData );

    // vfpTableData->ensureDataIsImported();
    // auto dataSources = vfpTableData->tableDataSources();

    // if ( !dataSources.empty() )
    // {
    //     return dataSources.front();
    // }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramMultiPlot*> RimHistogramMultiPlotCollection::vfpTableData() const
{
    return m_histogramMultiPlots.childrenByType();

    // std::vector<RimHistogramMultiPlot*> tableDataSources;

    //   for ( auto vfpTableData :
    //   {
    //       for ( auto table : vfpTableData->tableDataSources() )
    //       {
    //           tableDataSources.push_back( table );
    //       }
    //   }

    //   return tableDataSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlotCollection::loadDataAndUpdateAllPlots()
{
    // for ( auto vfpTableData : m_histogramMultiPlots.childrenByType() )
    // {
    //     vfpTableData->ensureDataIsImported();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlotCollection::deleteAllPlots()
{
    m_histogramMultiPlots.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimHistogramMultiPlotCollection::plotCount() const
{
    return m_histogramMultiPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlotCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder.addCmdFeature( "RicNewHistogramMultiPlot", "New Histogram Plot" );
}
