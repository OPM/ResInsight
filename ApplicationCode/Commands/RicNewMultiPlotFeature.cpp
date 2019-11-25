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

#include "RiaApplication.h"

#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimMultiPlotWindow.h"
#include "RimPlot.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"
#include <QAction>

#include "cafSelectionManager.h"

#include "cvfAssert.h"

RICF_SOURCE_INIT( RicNewMultiPlotFeature, "RicNewMultiPlotFeature", "createMultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewMultiPlotFeature::RicNewMultiPlotFeature()
{
    CAF_PDM_InitObject( "Create Multi Plot", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_plots, "plots", "Plots", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicNewMultiPlotFeature::execute()
{
    RimProject*             project        = RiaApplication::instance()->project();
    RimMultiPlotCollection* plotCollection = project->mainPlotCollection()->multiPlotCollection();

    RimMultiPlotWindow* plotWindow = new RimMultiPlotWindow;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiPlot( plotWindow );

    if ( !m_plots().empty() )
    {
        std::vector<RimPlot*> plots;
        for ( auto ptr : m_plots() )
        {
            plots.push_back( reinterpret_cast<RimPlot*>( ptr ) );
        }
        plotWindow->movePlotsToThis( plots, nullptr );
    }

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    RiuPlotMainWindowTools::setExpanded( plotCollection, true );
    RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow, true );

    return RicfCommandResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewMultiPlotFeature::isCommandEnabled()
{
    RimMultiPlotCollection* multiPlotCollection =
        caf::SelectionManager::instance()->selectedItemOfType<RimMultiPlotCollection>();
    if ( multiPlotCollection )
    {
        return true;
    }

    auto plots = selectedPlots();

    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    return !plots.empty() && plots.size() == selectedUiItems.size();
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
    if ( selectedPlots().empty() )
    {
        actionToSetup->setText( "New Empty Multi Plot" );
        actionToSetup->setIcon( QIcon( ":/WellLogPlot16x16.png" ) );
    }
    else
    {
        actionToSetup->setText( "Create Multi Plot from Selected Plots" );
        actionToSetup->setIcon( QIcon( ":/WellLogPlot16x16.png" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RicNewMultiPlotFeature::selectedPlots()
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    std::vector<RimPlot*> plots;
    for ( caf::PdmUiItem* uiItem : uiItems )
    {
        RimPlot* plotInterface = dynamic_cast<RimPlot*>( uiItem );
        if ( plotInterface )
        {
            plots.push_back( plotInterface );
        }
    }
    return plots;
}
