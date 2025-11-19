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
#include "RimProject.h"
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
    const auto& [caseSelection, ensembleSelection] = selectedSummarySources();
    return !caseSelection.empty() || !ensembleSelection.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    reloadSelectedCasesAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setToolTip( "Reload selected summary and/or ensemble cases" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryEnsemble*>> RicReloadSummaryCaseFeature::selectedSummarySources()
{
    std::vector<RimSummaryCase*>     caseSelection;
    std::vector<RimSummaryEnsemble*> ensembleSelection;

    auto ensembles = caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();
    for ( RimSummaryEnsemble* ensemble : ensembles )
    {
        ensembleSelection.push_back( ensemble );
    }

    std::vector<RimSummaryCase*> selectionManagerCases = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
    for ( RimSummaryCase* summaryCase : selectionManagerCases )
    {
        caseSelection.push_back( summaryCase );
    }

    for ( auto collection : caf::SelectionManager::instance()->objectsByType<RimObservedDataCollection>() )
    {
        std::vector<RimObservedSummaryData*> observedCases = collection->allObservedSummaryData();
        caseSelection.insert( caseSelection.end(), observedCases.begin(), observedCases.end() );
    }

    if ( ensembleSelection.empty() && caseSelection.empty() )
    {
        // Fallback to all top-level cases and ensembles
        if ( auto sumCaseColl = RiaSummaryTools::summaryCaseMainCollection() )
        {
            ensembleSelection = sumCaseColl->summaryEnsembles();

            // Make sure to select top-level cases only, not cases part of ensembles
            caseSelection = sumCaseColl->topLevelSummaryCases();
        }
    }

    return { caseSelection, ensembleSelection };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::reloadTaggedSummaryCasesAndUpdate()
{
    auto proj = RimProject::current();
    for ( auto summaryCase : proj->allSummaryCases() )
    {
        if ( summaryCase->includeInAutoReload() )
        {
            RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( summaryCase );
            summaryCase->updateConnectedEditors();
        }
    }

    for ( RimSummaryEnsemble* ensemble : proj->summaryEnsembles() )
    {
        if ( ensemble->includeInAutoReload() )
        {
            ensemble->reloadCases();
            ensemble->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadSummaryCaseFeature::reloadSelectedCasesAndUpdate()
{
    const auto& [caseSelection, ensembleSelection] = selectedSummarySources();

    for ( RimSummaryEnsemble* ensemble : ensembleSelection )
    {
        ensemble->reloadCases();
    }

    for ( RimSummaryCase* summaryCase : caseSelection )
    {
        RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( summaryCase );
        summaryCase->updateConnectedEditors();

        RiaLogging::info( QString( "Reloaded data for %1" ).arg( summaryCase->summaryHeaderFilename() ) );
    }
}
