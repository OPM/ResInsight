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

#include "RicHistogramPlotTools.h"
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
    CAF_PDM_InitObject( "Histogram Plots", ":/MultiPlot16x16.png" );

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
RimHistogramMultiPlot* RimHistogramMultiPlotCollection::appendHistogramMultiPlot()
{
    auto* histogramMultiPlot = new RimHistogramMultiPlot();
    m_histogramMultiPlots.push_back( histogramMultiPlot );
    return histogramMultiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramMultiPlot*> RimHistogramMultiPlotCollection::histogramMultiPlots() const
{
    return m_histogramMultiPlots.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlotCollection::loadDataAndUpdateAllPlots()
{
    for ( auto plot : histogramMultiPlots() )
    {
        plot->loadDataAndUpdateAllPlots();
    }
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
    menuBuilder.subMenuStart( "New Histogram Plot" );

    for ( RicHistogramPlotTools::DataSourceType dataSourceType : RicHistogramPlotTools::allDataSourceTypes() )
    {
        caf::AppEnum<RicHistogramPlotTools::DataSourceType> dstEnum( dataSourceType );
        menuBuilder.addCmdFeatureWithUserData( "RicNewHistogramMultiPlotFeature", dstEnum.uiText(), QVariant( dstEnum.text() ) );
    }
    menuBuilder.subMenuEnd();
}
