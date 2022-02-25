/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicNewSummaryMultiPlotFeature.h"

#include "RimMultiPlotCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RicSummaryPlotBuilder.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

RICF_SOURCE_INIT( RicNewSummaryMultiPlotFeature, "RicNewSummaryMultiPlotFeature", "createSummaryMultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewSummaryMultiPlotFeature::RicNewSummaryMultiPlotFeature()
{
    CAF_PDM_InitFieldNoDefault( &m_plots, "plots", "Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryMultiPlotFeature::isCommandEnabled()
{
    auto plots = selectedPlots();

    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    return !plots.empty() && plots.size() == selectedUiItems.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::onActionTriggered( bool isChecked )
{
    m_plots.v().clear();
    auto plots = selectedPlots();
    for ( RimPlot* plot : plots )
    {
        m_plots.v().push_back( reinterpret_cast<uintptr_t>( plot ) );
    }
    execute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Summary Multi Plot from Selected Plots" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RicNewSummaryMultiPlotFeature::selectedPlots()
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    std::vector<RimSummaryPlot*> plots;
    for ( caf::PdmUiItem* uiItem : uiItems )
    {
        RimSummaryPlot* summaryPlot = dynamic_cast<RimSummaryPlot*>( uiItem );
        if ( summaryPlot )
        {
            plots.push_back( summaryPlot );
        }
    }
    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicNewSummaryMultiPlotFeature::execute()
{
    if ( !m_plots().empty() )
    {
        std::vector<RimSummaryPlot*> plots;
        for ( auto ptr : m_plots() )
        {
            plots.push_back( reinterpret_cast<RimSummaryPlot*>( ptr ) );
        }

        auto copyOfPlots = RicSummaryPlotBuilder::duplicateSummaryPlots( plots );

        RicSummaryPlotBuilder::createAndAppendSummaryMultiPlot( copyOfPlots );
    }

    return caf::PdmScriptResponse();
}
