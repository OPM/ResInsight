/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimMainPlotCollection.h"

#include "RiaPlotCollectionScheduler.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "RimAbstractPlotCollection.h"
#include "RimAnalysisPlotCollection.h"
#include "RimCorrelationPlotCollection.h"
#include "RimCorrelationReportPlot.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowPlotCollection.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSaturationPressurePlot.h"
#include "RimSaturationPressurePlotCollection.h"
#include "RimStimPlanModelPlot.h"
#include "RimStimPlanModelPlotCollection.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlotCollection.h"
#include "RimVfpPlotCollection.h"
#include "RimViewWindow.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#ifdef USE_QTCHARTS
#include "RimEnsembleFractureStatisticsPlot.h"
#include "RimEnsembleFractureStatisticsPlotCollection.h"
#include "RimGridStatisticsPlot.h"
#include "RimGridStatisticsPlotCollection.h"
#endif

#include "RiuMainWindow.h"
#include "RiuProjectPropertyView.h"

#include "cafProgressInfo.h"

CAF_PDM_SOURCE_INIT( RimMainPlotCollection, "MainPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::RimMainPlotCollection()
{
    CAF_PDM_InitObject( "Plots" );

    CAF_PDM_InitField( &m_show, "Show", true, "Show 2D Plot Window" );
    m_show.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogPlotCollection, "WellLogPlotCollection", "" );
    m_wellLogPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_rftPlotCollection, "RftPlotCollection", "" );
    m_rftPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pltPlotCollection, "PltPlotCollection", "" );
    m_pltPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_summaryMultiPlotCollection, "SummaryMultiPlotCollection", "Multi Summary Plots" );
    m_summaryMultiPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_analysisPlotCollection, "AnalysisPlotCollection", "Analysis Plots" );
    m_analysisPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_correlationPlotCollection, "CorrelationPlotCollection", "Correlation Plots" );
    m_correlationPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_summaryCrossPlotCollection, "SummaryCrossPlotCollection", "Summary Cross Plots" );
    m_summaryCrossPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_flowPlotCollection, "FlowPlotCollection", "Flow Diagnostics Plots" );
    m_flowPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_gridCrossPlotCollection, "Rim3dCrossPlotCollection", "3d Cross Plots" );
    m_gridCrossPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_saturationPressurePlotCollection,
                                "RimSaturationPressurePlotCollection",
                                "Saturation Pressure Plots" );
    m_saturationPressurePlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_multiPlotCollection, "RimMultiPlotCollection", "Multi Plots" );
    m_multiPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_stimPlanModelPlotCollection, "StimPlanModelPlotCollection", "" );
    m_stimPlanModelPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_vfpPlotCollection, "VfpPlotCollection", "" );
    m_vfpPlotCollection.uiCapability()->setUiTreeHidden( true );
#ifdef USE_QTCHARTS
    CAF_PDM_InitFieldNoDefault( &m_gridStatisticsPlotCollection, "GridStatisticsPlotCollection", "" );
    m_gridStatisticsPlotCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensembleFractureStatisticsPlotCollection, "EnsembleFractureStatisticsPlotCollection", "" );
    m_ensembleFractureStatisticsPlotCollection.uiCapability()->setUiTreeHidden( true );
#endif

    m_wellLogPlotCollection            = new RimWellLogPlotCollection();
    m_rftPlotCollection                = new RimRftPlotCollection();
    m_pltPlotCollection                = new RimPltPlotCollection();
    m_summaryMultiPlotCollection       = new RimSummaryMultiPlotCollection();
    m_summaryCrossPlotCollection       = new RimSummaryCrossPlotCollection();
    m_flowPlotCollection               = new RimFlowPlotCollection();
    m_gridCrossPlotCollection          = new RimGridCrossPlotCollection;
    m_saturationPressurePlotCollection = new RimSaturationPressurePlotCollection;
    m_multiPlotCollection              = new RimMultiPlotCollection;
    m_analysisPlotCollection           = new RimAnalysisPlotCollection;
    m_correlationPlotCollection        = new RimCorrelationPlotCollection;
    m_stimPlanModelPlotCollection      = new RimStimPlanModelPlotCollection;
    m_vfpPlotCollection                = new RimVfpPlotCollection();
#ifdef USE_QTCHARTS
    m_gridStatisticsPlotCollection             = new RimGridStatisticsPlotCollection;
    m_ensembleFractureStatisticsPlotCollection = new RimEnsembleFractureStatisticsPlotCollection;
#endif

    CAF_PDM_InitFieldNoDefault( &m_summaryPlotCollection_OBSOLETE, "SummaryPlotCollection", "Summary Plots" );
    m_summaryPlotCollection_OBSOLETE.uiCapability()->setUiTreeHidden( true );
    m_summaryPlotCollection_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_summaryPlotCollection_OBSOLETE = new RimSummaryPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::~RimMainPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection* RimMainPlotCollection::current()
{
    return RimProject::current()->mainPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::initAfterRead()
{
    std::vector<RimSummaryPlot*> plotsToMove;
    for ( auto singlePlot : m_summaryPlotCollection_OBSOLETE()->plots() )
    {
        plotsToMove.push_back( singlePlot );
    }

    for ( auto singlePlot : plotsToMove )
    {
        m_summaryPlotCollection_OBSOLETE()->removePlot( singlePlot );

        RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlotNoAutoSettings( singlePlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMainPlotCollection::objectToggleField()
{
    return &m_show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RimMainPlotCollection::wellLogPlotCollection() const
{
    return m_wellLogPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftPlotCollection* RimMainPlotCollection::rftPlotCollection() const
{
    return m_rftPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPltPlotCollection* RimMainPlotCollection::pltPlotCollection() const
{
    return m_pltPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection* RimMainPlotCollection::summaryMultiPlotCollection() const
{
    return m_summaryMultiPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection* RimMainPlotCollection::summaryCrossPlotCollection() const
{
    return m_summaryCrossPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection* RimMainPlotCollection::flowPlotCollection() const
{
    return m_flowPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCollection* RimMainPlotCollection::gridCrossPlotCollection() const
{
    return m_gridCrossPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlotCollection* RimMainPlotCollection::saturationPressurePlotCollection() const
{
    return m_saturationPressurePlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotCollection* RimMainPlotCollection::multiPlotCollection() const
{
    return m_multiPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlotCollection* RimMainPlotCollection::vfpPlotCollection() const
{
    return m_vfpPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlotCollection* RimMainPlotCollection::analysisPlotCollection() const
{
    return m_analysisPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlotCollection* RimMainPlotCollection::correlationPlotCollection() const
{
    return m_correlationPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlotCollection* RimMainPlotCollection::stimPlanModelPlotCollection() const
{
    return m_stimPlanModelPlotCollection();
}

#ifdef USE_QTCHARTS
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsPlotCollection* RimMainPlotCollection::gridStatisticsPlotCollection() const
{
    return m_gridStatisticsPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatisticsPlotCollection* RimMainPlotCollection::ensembleFractureStatisticsPlotCollection() const
{
    return m_ensembleFractureStatisticsPlotCollection();
}
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::deleteAllContainedObjects()
{
    std::vector<RimPlotCollection*> plotCollections = allPlotCollections();
    for ( auto p : plotCollections )
        p->deleteAllPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::updateCurrentTimeStepInPlots()
{
    m_flowPlotCollection()->defaultFlowCharacteristicsPlot()->updateCurrentTimeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::updatePlotsWithFormations()
{
    std::vector<RimPlotCollection*> plotCollections = plotCollectionsWithFormations();
    loadDataAndUpdatePlotCollections( plotCollections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::scheduleUpdatePlotsWithCompletions()
{
    std::vector<RimPlotCollection*> plotCollections = plotCollectionsWithCompletions();

    RiaPlotCollectionScheduler::instance()->schedulePlotCollectionUpdate( plotCollections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::deleteAllCachedData()
{
    m_wellLogPlotCollection()->deleteAllExtractors();
    m_rftPlotCollection()->deleteAllExtractors();
    m_pltPlotCollection()->deleteAllExtractors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::ensureDefaultFlowPlotsAreCreated()
{
    m_flowPlotCollection()->ensureDefaultFlowPlotsAreCreated();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::ensureCalculationIdsAreAssigned()
{
    std::vector<RimSummaryAddress*> allAddresses;
    this->descendantsIncludingThisOfType( allAddresses );

    for ( RimSummaryAddress* adr : allAddresses )
    {
        adr->ensureCalculationIdIsAssigned();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::loadDataAndUpdateAllPlots()
{
    std::vector<RimPlotCollection*> plotCollections = allPlotCollections();
    loadDataAndUpdatePlotCollectionsWithProgressInfo( plotCollections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::updateSelectedWell( QString wellName )
{
    for ( auto plot : summaryMultiPlotCollection()->multiPlots() )
    {
        plot->selectWell( wellName );
    }

    for ( auto plot : wellLogPlotCollection()->wellLogPlots() )
    {
        plot->selectWell( wellName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::loadDataAndUpdatePlotCollectionsWithProgressInfo( const std::vector<RimPlotCollection*>& plotCollections )
{
    size_t plotCount = 0;
    for ( auto coll : plotCollections )
        if ( coll ) plotCount += coll->plotCount();

    if ( plotCount > 0 )
    {
        caf::ProgressInfo plotProgress( plotCount, "Loading Plot Data" );
        for ( auto coll : plotCollections )
        {
            if ( coll )
            {
                plotProgress.setNextProgressIncrement( coll->plotCount() );
                coll->loadDataAndUpdateAllPlots();
                plotProgress.incrementProgress();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::loadDataAndUpdatePlotCollections( const std::vector<RimPlotCollection*>& plotCollections )
{
    for ( auto coll : plotCollections )
    {
        if ( coll ) coll->loadDataAndUpdateAllPlots();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCollection*> RimMainPlotCollection::allPlotCollections() const
{
    std::vector<RimPlotCollection*> plotCollections;
    plotCollections.push_back( wellLogPlotCollection() );
    plotCollections.push_back( summaryMultiPlotCollection() );
    plotCollections.push_back( summaryCrossPlotCollection() );
    plotCollections.push_back( gridCrossPlotCollection() );
    plotCollections.push_back( analysisPlotCollection() );
    plotCollections.push_back( vfpPlotCollection() );
    plotCollections.push_back( flowPlotCollection() );
    plotCollections.push_back( pltPlotCollection() );
    plotCollections.push_back( rftPlotCollection() );
    plotCollections.push_back( stimPlanModelPlotCollection() );
    plotCollections.push_back( correlationPlotCollection() );
    plotCollections.push_back( saturationPressurePlotCollection() );
    plotCollections.push_back( multiPlotCollection() );

#ifdef USE_QTCHARTS
    plotCollections.push_back( gridStatisticsPlotCollection() );
    plotCollections.push_back( ensembleFractureStatisticsPlotCollection() );
#endif

    return plotCollections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCollection*> RimMainPlotCollection::plotCollectionsWithFormations() const
{
    std::vector<RimPlotCollection*> plotCollections;
    plotCollections.push_back( wellLogPlotCollection() );
    plotCollections.push_back( pltPlotCollection() );
    plotCollections.push_back( rftPlotCollection() );
    plotCollections.push_back( flowPlotCollection() );
    plotCollections.push_back( gridCrossPlotCollection() );
    plotCollections.push_back( multiPlotCollection() );
    plotCollections.push_back( stimPlanModelPlotCollection() );

#ifdef USE_QTCHARTS
    plotCollections.push_back( gridStatisticsPlotCollection() );
    plotCollections.push_back( ensembleFractureStatisticsPlotCollection() );
#endif

    return plotCollections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCollection*> RimMainPlotCollection::plotCollectionsWithCompletions() const
{
    std::vector<RimPlotCollection*> plotCollections;
    plotCollections.push_back( wellLogPlotCollection() );
    plotCollections.push_back( multiPlotCollection() );
    return plotCollections;
}
