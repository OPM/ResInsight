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

#include "RicNewGridPlotWindowFeature.h"

#include "RiaApplication.h"

#include "RimGridPlotWindow.h"
#include "RimGridPlotWindowCollection.h"
#include "RimMainPlotCollection.h"
#include "RimPlotInterface.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"
#include <QAction>

#include "cafSelectionManager.h"

#include "cvfAssert.h"

RICF_SOURCE_INIT( RicNewGridPlotWindowFeature, "RicNewGridPlotWindowFeature", "createCombinationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewGridPlotWindowFeature::RicNewGridPlotWindowFeature()
{
    CAF_PDM_InitObject( "Create Combination Plot", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_plots, "plots", "Plots", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicNewGridPlotWindowFeature::execute()
{
    RimProject*                  project        = RiaApplication::instance()->project();
    RimGridPlotWindowCollection* plotCollection = project->mainPlotCollection()->combinationPlotCollection();

    RimGridPlotWindow* plotWindow = new RimGridPlotWindow;
    plotWindow->setDescription( QString( "Combination Plot %1" ).arg( plotCollection->gridPlotWindows().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addGridPlotWindow( plotWindow );

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
bool RicNewGridPlotWindowFeature::isCommandEnabled()
{
    RimGridPlotWindowCollection* gridPlotCollection =
        caf::SelectionManager::instance()->selectedItemOfType<RimGridPlotWindowCollection>();
    if ( gridPlotCollection )
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
void RicNewGridPlotWindowFeature::onActionTriggered( bool isChecked )
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
void RicNewGridPlotWindowFeature::setupActionLook( QAction* actionToSetup )
{
    if ( selectedPlotInterfaces().empty() )
    {
        actionToSetup->setText( "New Empty Plot Report" );
        actionToSetup->setIcon( QIcon( ":/WellLogPlot16x16.png" ) );
    }
    else
    {
        actionToSetup->setText( "Create Plot Report from Selected Plots" );
        actionToSetup->setIcon( QIcon( ":/WellLogPlot16x16.png" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotInterface*> RicNewGridPlotWindowFeature::selectedPlotInterfaces()
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
