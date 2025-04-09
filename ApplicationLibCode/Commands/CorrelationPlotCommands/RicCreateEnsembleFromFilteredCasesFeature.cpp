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
#include "RimEnsembleCurveSet.h"
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

RimEnsembleCurveSet* ensembleCurveSet()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimEnsembleCurveSet>();
}
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateEnsembleFromFilteredCasesFeature::isCommandEnabled() const
{
    if ( internal::ensembleCurveSet() != nullptr ) return true;

    return internal::selectedCrossPlot() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleFromFilteredCasesFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryCase*> casesForNewEnsemble;

    if ( auto crossPlot = internal::selectedCrossPlot() )
    {
        auto excludedCases = crossPlot->summaryCasesExcludedByFilter();
        if ( !excludedCases.empty() )
        {
            if ( auto ensemble = excludedCases.front()->ensemble() )
            {
                casesForNewEnsemble = ensemble->allSummaryCases();
                for ( auto excludedCase : excludedCases )
                {
                    casesForNewEnsemble.erase( std::remove( casesForNewEnsemble.begin(), casesForNewEnsemble.end(), excludedCase ),
                                               casesForNewEnsemble.end() );
                }
            }
        }
    }

    if ( casesForNewEnsemble.empty() )
    {
        if ( auto curveSet = internal::ensembleCurveSet() )
        {
            if ( auto ensemble = curveSet->summaryEnsemble() )
            {
                casesForNewEnsemble = curveSet->filterEnsembleCases( ensemble->allSummaryCases() );
            }
        }
    }

    if ( casesForNewEnsemble.empty() )
    {
        RiaLogging::info( "No filtered cases found, no ensemble created." );
        return;
    }

    if ( auto newEnsemble = RicImportEnsembleFeature::createSummaryEnsemble( casesForNewEnsemble ) )
    {
        RiaLogging::info( "Created ensemble " + newEnsemble->name() );
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
