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
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPlotInterface.h"
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

    RimMultiPlot* plotWindow = new RimMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiPlot( plotWindow );

    if ( !m_plots().empty() )
    {
        std::vector<RimPlotInterface*> plotInterfaces;
        for ( auto ptr : m_plots() )
        {
            plotInterfaces.push_back( reinterpret_cast<RimPlotInterface*>( ptr ) );
        }
        plotWindow->movePlotsToThis( plotInterfaces, nullptr );
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

    auto selectedPlots = selectedPlotInterfaces();

    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    return !selectedPlots.empty() && selectedPlots.size() == selectedUiItems.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPlotFeature::onActionTriggered( bool isChecked )
{
    m_plots.v().clear();
    auto selectedPlots = selectedPlotInterfaces();
    for ( RimPlotInterface* plotInterface : selectedPlots )
    {
        m_plots.v().push_back( reinterpret_cast<uintptr_t>( plotInterface ) );
    }
    execute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    if ( selectedPlotInterfaces().empty() )
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
std::vector<RimPlotInterface*> RicNewMultiPlotFeature::selectedPlotInterfaces()
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    std::vector<RimPlotInterface*> plotInterfaces;
    for ( caf::PdmUiItem* uiItem : uiItems )
    {
        RimPlotInterface* plotInterface = dynamic_cast<RimPlotInterface*>( uiItem );
        if ( plotInterface )
        {
            plotInterfaces.push_back( plotInterface );
        }
    }
    return plotInterfaces;
}
