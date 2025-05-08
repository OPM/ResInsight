/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicAppendSummaryPlotsForSummaryCasesFeature.h"

#include "Summary/RiaSummaryPlotTools.h"

#include "RicAppendSummaryPlotsForObjectsFeature.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSummaryPlotsForSummaryCasesFeature, "RicAppendSummaryPlotsForSummaryCasesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryPlotsForSummaryCasesFeature::isCommandEnabled() const
{
    auto cases     = selectedCases();
    auto ensembles = selectedEnsembles();

    return !( cases.empty() && ensembles.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryCasesFeature::onActionTriggered( bool isChecked )
{
    auto cases     = selectedCases();
    auto ensembles = selectedEnsembles();

    auto selectedMultiPlots = RiaSummaryPlotTools::selectedSummaryMultiPlots();
    for ( auto summaryMultiPlot : selectedMultiPlots )
    {
        RicAppendSummaryPlotsForObjectsFeature::appendPlots( summaryMultiPlot, cases, ensembles );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryCasesFeature::setupActionLook( QAction* actionToSetup )
{
    auto text = QString( "Append as New Plot(s)" );
    actionToSetup->setText( text );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicAppendSummaryPlotsForSummaryCasesFeature::selectedCases()
{
    return caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> RicAppendSummaryPlotsForSummaryCasesFeature::selectedEnsembles()
{
    return caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();
}
