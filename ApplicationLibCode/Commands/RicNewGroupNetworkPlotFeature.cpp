/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicNewGroupNetworkPlotFeature.h"

#include "GroupNetwork/RimGroupNetworkData.h"
#include "GroupNetwork/RimGroupNetworkDataCollection.h"
#include "GroupNetwork/RimGroupNetworkPlot.h"
#include "GroupNetwork/RimGroupNetworkPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewGroupNetworkPlotFeature, "RicNewGroupNetworkPlotFeature" );

namespace
{
RimGroupNetworkData* dataSourceForNewPlot()
{
    if ( auto* selected = caf::SelectionManager::instance()->selectedItemOfType<RimGroupNetworkData>() ) return selected;

    auto* project = RimProject::current();
    if ( !project || !project->activeOilField() || !project->activeOilField()->groupNetworkCollection() ) return nullptr;

    const auto items = project->activeOilField()->groupNetworkCollection()->items();
    return items.empty() ? nullptr : items.front();
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewGroupNetworkPlotFeature::isCommandEnabled() const
{
    auto* mainPlotCollection = RimMainPlotCollection::current();
    return mainPlotCollection && mainPlotCollection->groupNetworkPlotCollection() && dataSourceForNewPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGroupNetworkPlotFeature::onActionTriggered( bool isChecked )
{
    auto* mainPlotCollection = RimMainPlotCollection::current();
    if ( !mainPlotCollection || !mainPlotCollection->groupNetworkPlotCollection() ) return;

    RimGroupNetworkData* data = dataSourceForNewPlot();
    if ( !data ) return;

    RimGroupNetworkPlotCollection* collection = mainPlotCollection->groupNetworkPlotCollection();

    auto* plot = collection->createPlot( data );
    collection->updateConnectedEditors();

    RiuPlotMainWindowTools::showPlotMainWindow();
    plot->loadDataAndUpdate();
    RiuPlotMainWindowTools::onObjectAppended( plot, collection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGroupNetworkPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Production Network Plot" );
    actionToSetup->setIcon( QIcon( ":/summary/components/images/production-network.svg" ) );
}
