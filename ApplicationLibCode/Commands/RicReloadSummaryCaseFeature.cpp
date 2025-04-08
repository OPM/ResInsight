/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicReloadSummaryCaseFeature.h"

#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadSummaryCaseFeature, "RicReloadSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadSummaryCaseFeature::isCommandEnabled() const
{
    std::vector<RimSummaryCase*> caseSelection = selectedSummaryCases();

    return !caseSelection.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryCase*> caseSelection = selectedSummaryCases();
    for ( RimSummaryCase* summaryCase : caseSelection )
    {
        RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( summaryCase );

        RiaLogging::info( QString( "Reloaded data for %1" ).arg( summaryCase->summaryHeaderFilename() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicReloadSummaryCaseFeature::selectedSummaryCases()
{
    const auto mainCollectionSelection = caf::SelectionManager::instance()->objectsByType<RimSummaryCaseMainCollection>();
    if ( !mainCollectionSelection.empty() )
    {
        return mainCollectionSelection[0]->allSummaryCases();
    }

    std::vector<RimSummaryCase*> caseSelection = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();

    {
        for ( auto collection : caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>() )
        {
            std::vector<RimSummaryCase*> summaryCaseCollection = collection->allSummaryCases();
            caseSelection.insert( caseSelection.end(), summaryCaseCollection.begin(), summaryCaseCollection.end() );
        }
    }

    {
        for ( auto collection : caf::SelectionManager::instance()->objectsByType<RimObservedDataCollection>() )
        {
            std::vector<RimObservedSummaryData*> observedCases = collection->allObservedSummaryData();
            caseSelection.insert( caseSelection.end(), observedCases.begin(), observedCases.end() );
        }
    }

    return caseSelection;
}
