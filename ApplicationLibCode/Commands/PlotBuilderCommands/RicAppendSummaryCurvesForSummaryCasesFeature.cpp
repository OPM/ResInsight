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

#include "RiaGuiApplication.h"

#include "RicAppendSummaryPlotsForObjectsFeature.h"

#include "RimSummaryAddressCollection.h"
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
    return !selectedCases().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryCurvesForSummaryCasesFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    auto summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( app->activePlotWindow() );
    if ( !summaryMultiPlot ) return;

    auto cases = selectedCases();
    if ( cases.empty() ) return;

    for ( auto plot : summaryMultiPlot->summaryPlots() )
    {
        plot->handleDroppedObjects( cases );
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
std::vector<caf::PdmObjectHandle*> RicAppendSummaryCurvesForSummaryCasesFeature::selectedCases()
{
    // Make the object type general to match the expected type for handleDroppedObjects();
    std::vector<caf::PdmObjectHandle*> generalObjects;

    {
        std::vector<RimSummaryCase*> objects;
        caf::SelectionManager::instance()->objectsByType( &objects );

        generalObjects.insert( generalObjects.begin(), objects.begin(), objects.end() );
    }
    {
        std::vector<RimSummaryEnsemble*> objects;
        caf::SelectionManager::instance()->objectsByType( &objects );

        generalObjects.insert( generalObjects.begin(), objects.begin(), objects.end() );
    }

    return generalObjects;
}
