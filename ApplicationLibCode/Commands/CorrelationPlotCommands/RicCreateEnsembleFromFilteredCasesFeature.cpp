/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicCreateEnsembleFromFilteredCasesFeature.h"

#include "RiaLogging.h"

#include "RicImportEnsembleFeature.h"

#include "RimCorrelationPlot.h"
#include "RimCorrelationPlotCollection.h"
#include "RimCorrelationReportPlot.h"
#include "RimParameterResultCrossPlot.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateEnsembleFromFilteredCasesFeature, "RicCreateEnsembleFromFilteredCasesFeature" );

namespace internal
{
RimParameterResultCrossPlot* selectedCrossPlot()
{
    auto selectedPlots = caf::selectedObjectsByTypeStrict<RimParameterResultCrossPlot*>();
    if ( selectedPlots.size() == 1 ) return selectedPlots.front();

    if ( auto reportPlot = caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationReportPlot>() )
    {
        return reportPlot->crossPlot();
    }

    return nullptr;
}
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateEnsembleFromFilteredCasesFeature::isCommandEnabled() const
{
    return internal::selectedCrossPlot() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleFromFilteredCasesFeature::onActionTriggered( bool isChecked )
{
    auto crossPlot = internal::selectedCrossPlot();
    if ( !crossPlot ) return;

    auto excludedCases = crossPlot->summaryCasesExcludedByFilter();
    if ( excludedCases.empty() )
    {
        RiaLogging::info( "No filtered cases found, no ensemble created." );
        return;
    }

    auto ensemble = excludedCases.front()->ensemble();
    if ( !ensemble )
    {
        RiaLogging::info( "No ensemble found for selected cases, no ensemble created." );
        return;
    }

    auto casesForNewEnsemble = ensemble->allSummaryCases();
    for ( auto excludedCase : excludedCases )
    {
        casesForNewEnsemble.erase( std::remove( casesForNewEnsemble.begin(), casesForNewEnsemble.end(), excludedCase ),
                                   casesForNewEnsemble.end() );
    }

    auto newEnsemble = RicImportEnsembleFeature::createSummaryEnsemble( casesForNewEnsemble );
    if ( newEnsemble )
    {
        RiaLogging::info( "Created ensemble " + ensemble->name() );
    }
    else
    {
        RiaLogging::error( "Failed to create ensemble from filtered cases." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleFromFilteredCasesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble.svg" ) );
    actionToSetup->setText( "Create Ensemble from Filtered Cases" );
}
