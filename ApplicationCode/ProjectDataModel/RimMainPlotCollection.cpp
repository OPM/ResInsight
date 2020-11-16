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

#include "RimCorrelationPlotCollection.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowPlotCollection.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
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

#include "RimAnalysisPlotCollection.h"
#include "RiuMainWindow.h"
#include "RiuProjectPropertyView.h"

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
