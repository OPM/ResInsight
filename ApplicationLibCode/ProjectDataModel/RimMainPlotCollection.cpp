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
    CAF_PDM_InitObject( "Plots", "", "", "" );

    CAF_PDM_InitField( &m_show, "Show", true, "Show 2D Plot Window", "", "", "" );
    m_show.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogPlotCollection, "WellLogPlotCollection", "", "", "", "" );
    m_wellLogPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_rftPlotCollection, "RftPlotCollection", "", "", "", "" );
    m_rftPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pltPlotCollection, "PltPlotCollection", "", "", "", "" );
    m_pltPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_summaryPlotCollection, "SummaryPlotCollection", "Summary Plots", "", "", "" );
    m_summaryPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_analysisPlotCollection, "AnalysisPlotCollection", "Analysis Plots", "", "", "" );
    m_analysisPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_correlationPlotCollection, "CorrelationPlotCollection", "Correlation Plots", "", "", "" );
    m_correlationPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_summaryCrossPlotCollection, "SummaryCrossPlotCollection", "Summary Cross Plots", "", "", "" );
    m_summaryCrossPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_flowPlotCollection, "FlowPlotCollection", "Flow Diagnostics Plots", "", "", "" );
    m_flowPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_gridCrossPlotCollection, "Rim3dCrossPlotCollection", "3d Cross Plots", "", "", "" );
    m_gridCrossPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_saturationPressurePlotCollection,
                                "RimSaturationPressurePlotCollection",
                                "Saturation Pressure Plots",
                                "",
                                "",
                                "" );
    m_saturationPressurePlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_multiPlotCollection, "RimMultiPlotCollection", "Multi Plots", "", "", "" );
    m_multiPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_stimPlanModelPlotCollection, "StimPlanModelPlotCollection", "", "", "", "" );
    m_stimPlanModelPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_vfpPlotCollection, "VfpPlotCollection", "", "", "", "" );
    m_vfpPlotCollection.uiCapability()->setUiHidden( true );
#ifdef USE_QTCHARTS
    CAF_PDM_InitFieldNoDefault( &m_gridStatisticsPlotCollection, "GridStatisticsPlotCollection", "", "", "", "" );
    m_gridStatisticsPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensembleFractureStatisticsPlotCollection,
                                "EnsembleFractureStatisticsPlotCollection",
                                "",
                                "",
                                "",
                                "" );
    m_ensembleFractureStatisticsPlotCollection.uiCapability()->setUiHidden( true );
#endif

    m_wellLogPlotCollection            = new RimWellLogPlotCollection();
    m_rftPlotCollection                = new RimRftPlotCollection();
    m_pltPlotCollection                = new RimPltPlotCollection();
    m_summaryPlotCollection            = new RimSummaryPlotCollection();
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
RimWellLogPlotCollection* RimMainPlotCollection::wellLogPlotCollection()
{
    return m_wellLogPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftPlotCollection* RimMainPlotCollection::rftPlotCollection()
{
    return m_rftPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPltPlotCollection* RimMainPlotCollection::pltPlotCollection()
{
    return m_pltPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection* RimMainPlotCollection::summaryPlotCollection()
{
    return m_summaryPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection* RimMainPlotCollection::summaryCrossPlotCollection()
{
    return m_summaryCrossPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection* RimMainPlotCollection::flowPlotCollection()
{
    return m_flowPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCollection* RimMainPlotCollection::gridCrossPlotCollection()
{
    return m_gridCrossPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlotCollection* RimMainPlotCollection::saturationPressurePlotCollection()
{
    return m_saturationPressurePlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotCollection* RimMainPlotCollection::multiPlotCollection()
{
    return m_multiPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlotCollection* RimMainPlotCollection::vfpPlotCollection()
{
    return m_vfpPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlotCollection* RimMainPlotCollection::analysisPlotCollection()
{
    return m_analysisPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlotCollection* RimMainPlotCollection::correlationPlotCollection()
{
    return m_correlationPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlotCollection* RimMainPlotCollection::stimPlanModelPlotCollection()
{
    return m_stimPlanModelPlotCollection();
}

#ifdef USE_QTCHARTS
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsPlotCollection* RimMainPlotCollection::gridStatisticsPlotCollection()
{
    return m_gridStatisticsPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatisticsPlotCollection* RimMainPlotCollection::ensembleFractureStatisticsPlotCollection()
{
    return m_ensembleFractureStatisticsPlotCollection();
}
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::deleteAllContainedObjects()
{
    m_wellLogPlotCollection()->deleteAllPlots();
    m_rftPlotCollection()->deleteAllPlots();
    m_pltPlotCollection()->deleteAllPlots();
    m_summaryPlotCollection()->deleteAllPlots();
    m_summaryCrossPlotCollection()->deleteAllPlots();
    m_gridCrossPlotCollection->deleteAllPlots();
    m_flowPlotCollection()->closeDefaultPlotWindowAndDeletePlots();
    m_saturationPressurePlotCollection()->deleteAllChildObjects();
    m_multiPlotCollection()->deleteAllChildObjects();
    m_vfpPlotCollection()->deleteAllChildObjects();
    m_analysisPlotCollection()->deleteAllPlots();
    m_correlationPlotCollection()->deleteAllPlots();
    m_stimPlanModelPlotCollection()->deleteAllPlots();
#ifdef USE_QTCHARTS
    m_gridStatisticsPlotCollection()->deleteAllPlots();
    m_ensembleFractureStatisticsPlotCollection()->deleteAllPlots();
#endif
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
    if ( m_wellLogPlotCollection )
    {
        for ( RimWellLogPlot* wellLogPlot : m_wellLogPlotCollection->wellLogPlots() )
        {
            wellLogPlot->loadDataAndUpdate();
        }
    }

    if ( m_pltPlotCollection )
    {
        for ( RimWellPltPlot* pltPlot : m_pltPlotCollection->pltPlots() )
        {
            pltPlot->loadDataAndUpdate();
        }
    }

    if ( m_rftPlotCollection )
    {
        for ( RimWellRftPlot* rftPlot : m_rftPlotCollection->rftPlots() )
        {
            rftPlot->loadDataAndUpdate();
        }
    }

    if ( m_flowPlotCollection )
    {
        m_flowPlotCollection->loadDataAndUpdate();
    }

    if ( m_gridCrossPlotCollection )
    {
        for ( RimGridCrossPlot* crossPlot : m_gridCrossPlotCollection->plots() )
        {
            crossPlot->loadDataAndUpdate();
        }
    }

    if ( m_multiPlotCollection )
    {
        for ( RimMultiPlot* plotWindow : m_multiPlotCollection->multiPlots() )
        {
            plotWindow->loadDataAndUpdate();
        }
    }

    if ( m_stimPlanModelPlotCollection )
    {
        for ( RimStimPlanModelPlot* stimPlanModelPlot : m_stimPlanModelPlotCollection->stimPlanModelPlots() )
        {
            stimPlanModelPlot->loadDataAndUpdate();
        }
    }

#ifdef USE_QTCHARTS
    if ( m_gridStatisticsPlotCollection )
    {
        for ( RimGridStatisticsPlot* gridStatisticsPlot : m_gridStatisticsPlotCollection->gridStatisticsPlots() )
        {
            gridStatisticsPlot->loadDataAndUpdate();
        }
    }

    if ( m_ensembleFractureStatisticsPlotCollection )
    {
        for ( RimEnsembleFractureStatisticsPlot* ensembleFractureStatisticsPlot :
              m_ensembleFractureStatisticsPlotCollection->ensembleFractureStatisticsPlots() )
        {
            ensembleFractureStatisticsPlot->loadDataAndUpdate();
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::updatePlotsWithCompletions()
{
    if ( m_wellLogPlotCollection )
    {
        for ( RimWellLogPlot* wellLogPlot : m_wellLogPlotCollection->wellLogPlots() )
        {
            wellLogPlot->loadDataAndUpdate();
        }
    }

    if ( m_multiPlotCollection )
    {
        for ( RimMultiPlot* plotWindow : m_multiPlotCollection->multiPlots() )
        {
            plotWindow->loadDataAndUpdate();
        }
    }
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
        adr->ensureIdIsAssigned();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::loadDataAndUpdateAllPlots()
{
    RimWellLogPlotCollection*            wlpColl  = nullptr;
    RimSummaryPlotCollection*            spColl   = nullptr;
    RimFlowPlotCollection*               flowColl = nullptr;
    RimRftPlotCollection*                rftColl  = nullptr;
    RimPltPlotCollection*                pltColl  = nullptr;
    RimSaturationPressurePlotCollection* sppColl  = nullptr;
    RimCorrelationPlotCollection*        corrColl = nullptr;
    RimMultiPlotCollection*              gpwColl  = nullptr;
    RimStimPlanModelPlotCollection*      frmColl  = nullptr;

    std::vector<RimAbstractPlotCollection*> plotCollections;
    plotCollections.push_back( summaryCrossPlotCollection() );
    plotCollections.push_back( gridCrossPlotCollection() );
    plotCollections.push_back( analysisPlotCollection() );
    plotCollections.push_back( vfpPlotCollection() );

    if ( wellLogPlotCollection() )
    {
        wlpColl = wellLogPlotCollection();
    }
    if ( summaryPlotCollection() )
    {
        spColl = summaryPlotCollection();
    }
    if ( flowPlotCollection() )
    {
        flowColl = flowPlotCollection();
    }
    if ( rftPlotCollection() )
    {
        rftColl = rftPlotCollection();
    }
    if ( pltPlotCollection() )
    {
        pltColl = pltPlotCollection();
    }
    if ( saturationPressurePlotCollection() )
    {
        sppColl = saturationPressurePlotCollection();
    }
    if ( correlationPlotCollection() )
    {
        corrColl = correlationPlotCollection();
    }
    if ( multiPlotCollection() )
    {
        gpwColl = multiPlotCollection();
    }
    if ( stimPlanModelPlotCollection() )
    {
        frmColl = stimPlanModelPlotCollection();
    }

    size_t plotCount = 0;
    plotCount += wlpColl ? wlpColl->wellLogPlots().size() : 0;
    plotCount += spColl ? spColl->plots().size() : 0;
    plotCount += flowColl ? flowColl->plotCount() : 0;
    plotCount += rftColl ? rftColl->rftPlots().size() : 0;
    plotCount += pltColl ? pltColl->pltPlots().size() : 0;
    plotCount += sppColl ? sppColl->plotCount() : 0;
    plotCount += corrColl ? corrColl->plotCount() + corrColl->reports().size() : 0;
    plotCount += gpwColl ? gpwColl->multiPlots().size() : 0;
    plotCount += frmColl ? frmColl->stimPlanModelPlots().size() : 0;

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

        if ( wlpColl )
        {
            for ( auto wellLogPlot : wlpColl->wellLogPlots() )
            {
                wellLogPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( spColl )
        {
            for ( auto plot : spColl->plots() )
            {
                plot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( flowColl )
        {
            plotProgress.setNextProgressIncrement( flowColl->plotCount() );
            flowColl->loadDataAndUpdate();
            plotProgress.incrementProgress();
        }

        if ( rftColl )
        {
            for ( const auto& rftPlot : rftColl->rftPlots() )
            {
                rftPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( pltColl )
        {
            for ( const auto& pltPlot : pltColl->pltPlots() )
            {
                pltPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( sppColl )
        {
            for ( const auto& sppPlot : sppColl->plots() )
            {
                sppPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( corrColl )
        {
            for ( const auto& corrPlot : corrColl->plots() )
            {
                corrPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
            for ( const auto& reports : corrColl->reports() )
            {
                reports->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( gpwColl )
        {
            for ( const auto& multiPlot : gpwColl->multiPlots() )
            {
                multiPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( frmColl )
        {
            for ( const auto& stimPlanModelPlot : frmColl->stimPlanModelPlots() )
            {
                stimPlanModelPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }
    }
}
