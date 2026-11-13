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

#include "RicNewGroupNetworkDataFeature.h"

#include "GroupNetwork/RimGroupNetworkData.h"
#include "GroupNetwork/RimGroupNetworkDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewGroupNetworkDataFeature, "RicNewGroupNetworkDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewGroupNetworkDataFeature::isCommandEnabled() const
{
    auto* project = RimProject::current();
    return project && project->activeOilField() && project->activeOilField()->groupNetworkCollection() && !project->allSummaryCases().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGroupNetworkDataFeature::onActionTriggered( bool isChecked )
{
    auto* project = RimProject::current();
    if ( !project || !project->activeOilField() || !project->activeOilField()->groupNetworkCollection() ) return;
    RimGroupNetworkDataCollection* collection = project->activeOilField()->groupNetworkCollection();

    auto* network = new RimGroupNetworkData();

    RimSummaryCase*     summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimSummaryCase>();
    RimSummaryEnsemble* ensemble    = caf::SelectionManager::instance()->selectedItemOfType<RimSummaryEnsemble>();
    if ( !summaryCase && ensemble && ensemble->isEnsemble() )
    {
        network->setSummaryEnsemble( ensemble );
    }
    else
    {
        if ( !summaryCase )
        {
            const auto summaryCases = project->allSummaryCases();
            if ( !summaryCases.empty() ) summaryCase = summaryCases.front();
        }
        if ( !summaryCase )
        {
            delete network;
            return;
        }
        network->setSummaryCase( summaryCase );
    }

    collection->addItem( network );
    collection->updateConnectedEditors();

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::onObjectAppended( network, collection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGroupNetworkDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Production Network Data Source" );
    actionToSetup->setIcon( QIcon( ":/summary/components/images/production-network.svg" ) );
}
