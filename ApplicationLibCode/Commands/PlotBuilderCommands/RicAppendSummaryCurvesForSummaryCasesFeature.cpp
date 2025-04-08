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

#include "RicAppendSummaryCurvesForSummaryCasesFeature.h"

#include "Summary/RiaSummaryPlotTools.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSummaryCurvesForSummaryCasesFeature, "RicAppendSummaryCurvesForSummaryCasesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryCurvesForSummaryCasesFeature::isCommandEnabled() const
{
    return !dragDropObjects().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryCurvesForSummaryCasesFeature::onActionTriggered( bool isChecked )
{
    auto objectsForDrop = dragDropObjects();
    if ( objectsForDrop.empty() ) return;

    auto selectedMultiPlots = RiaSummaryPlotTools::selectedSummaryMultiPlots();
    for ( auto summaryMultiPlot : selectedMultiPlots )
    {
        for ( auto plot : summaryMultiPlot->summaryPlots() )
        {
            plot->handleDroppedObjects( objectsForDrop );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryCurvesForSummaryCasesFeature::setupActionLook( QAction* actionToSetup )
{
    QString objectType = "Cases";

    auto text = QString( "Append Curves For " ) + objectType;
    actionToSetup->setText( text );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmObjectHandle*> RicAppendSummaryCurvesForSummaryCasesFeature::dragDropObjects()
{
    // Make the object type general to match the expected type for handleDroppedObjects();
    std::vector<caf::PdmObjectHandle*> objects;

    const auto summaryCases = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
    objects.insert( objects.begin(), summaryCases.begin(), summaryCases.end() );

    const auto ensembles = caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();
    objects.insert( objects.begin(), ensembles.begin(), ensembles.end() );

    return objects;
}
