/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicShowWellAllocationPlotFeature.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimWellAllocationOverTimePlot.h"
#include "RimWellAllocationPlot.h"
#include "RimWellConnectivityTable.h"
#include "RimWellPath.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowWellAllocationPlotFeature, "RicShowWellAllocationPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowWellAllocationPlotFeature::isCommandEnabled() const
{
    if ( !caf::SelectionManager::instance()->objectsByType<RimSimWellInView>().empty() )
    {
        return true;
    }

    const auto wellPathCollection = caf::SelectionManager::instance()->objectsByType<RimWellPath>();
    if ( wellPathCollection.empty() ) return false;

    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return false;
    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );
    if ( !eclView ) return false;

    RimSimWellInView* simWellFromWellPath = eclView->wellCollection()->findWell( wellPathCollection[0]->associatedSimulationWellName() );

    return simWellFromWellPath != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowWellAllocationPlotFeature::onActionTriggered( bool isChecked )
{
    const auto collection         = caf::SelectionManager::instance()->objectsByType<RimSimWellInView>();
    const auto wellPathCollection = caf::SelectionManager::instance()->objectsByType<RimWellPath>();

    RimSimWellInView* simWell = nullptr;

    if ( !collection.empty() )
    {
        simWell = collection[0];
    }
    else if ( !wellPathCollection.empty() )
    {
        Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
        if ( !view ) return;
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );
        if ( !eclView ) return;

        simWell = eclView->wellCollection()->findWell( wellPathCollection[0]->associatedSimulationWellName() );
        if ( !simWell ) return;
    }
    else
        return;

    RimFlowPlotCollection* flowPlotColl = RimMainPlotCollection::current()->flowPlotCollection();
    if ( flowPlotColl )
    {
        flowPlotColl->defaultWellAllocPlot()->setFromSimulationWell( simWell );
        flowPlotColl->defaultWellAllocPlot()->updateConnectedEditors();

        flowPlotColl->defaultWellAllocOverTimePlot()->setFromSimulationWell( simWell );
        flowPlotColl->defaultWellAllocOverTimePlot()->updateConnectedEditors();

        flowPlotColl->defaultWellConnectivityTable()->setFromSimulationWell( simWell );

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::onObjectAppended( flowPlotColl->defaultWellAllocOverTimePlot() );
        RiuPlotMainWindowTools::onObjectAppended( flowPlotColl->defaultWellAllocPlot() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowWellAllocationPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/WellAllocPlot16x16.png" ) );
    actionToSetup->setText( "Plot Well Allocation" );
}
