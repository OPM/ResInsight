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

#include "RicNewMultiPlotFeature.h"

#include "Summary/RiaSummaryPlotTools.h"

#include "RimPlot.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

RICF_SOURCE_INIT( RicNewMultiPlotFeature, "RicNewMultiPlotFeature", "createMultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewMultiPlotFeature::RicNewMultiPlotFeature()
{
    CAF_PDM_InitFieldNoDefault( &m_plots, "plots", "Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicNewMultiPlotFeature::execute()
{
    if ( !m_plots().empty() )
    {
        std::vector<RimPlot*> plots;
        for ( auto ptr : m_plots() )
        {
            plots.push_back( reinterpret_cast<RimPlot*>( ptr ) );
        }

        auto copyOfPlots = RiaSummaryPlotTools::duplicatePlots( plots );

        RiaSummaryPlotTools::createAndAppendMultiPlot( copyOfPlots );
    }

    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewMultiPlotFeature::isCommandEnabled() const
{
    auto plots = selectedPlots();

    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();

    return !plots.empty() && plots.size() == selectedItems.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPlotFeature::onActionTriggered( bool isChecked )
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
void RicNewMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Multi Plot from Selected Plots" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RicNewMultiPlotFeature::selectedPlots()
{
    std::vector<RimPlot*> plots;
    for ( caf::PdmUiItem* uiItem : caf::SelectionManager::instance()->selectedItems() )
    {
        RimPlot* plotInterface = dynamic_cast<RimPlot*>( uiItem );
        // Special case for all well log tracks which currently need to be in Well Log Plot for
        // depth information and cannot be moved into multiplots.
        // TODO: copy depth information into well log tracks to allow their use separately.
        RimWellLogTrack* wellLogTrack = dynamic_cast<RimWellLogTrack*>( plotInterface );
        if ( plotInterface && !wellLogTrack )
        {
            plots.push_back( plotInterface );
        }
    }
    return plots;
}
