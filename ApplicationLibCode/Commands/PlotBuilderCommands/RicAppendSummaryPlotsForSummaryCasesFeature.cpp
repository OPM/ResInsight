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

#include "RiaGuiApplication.h"

#include "RicAppendSummaryPlotsForObjectsFeature.h"

#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryMultiPlot.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSummaryPlotsForSummaryCasesFeature, "RicAppendSummaryPlotsForSummaryCasesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryCasesFeature::appendPlotsForCases( RimSummaryMultiPlot* summaryMultiPlot,
                                                                       const std::vector<RimSummaryCase*>& cases )
{
    if ( !summaryMultiPlot ) return;
    if ( cases.empty() ) return;

    std::vector<RimSummaryAddressCollection*> tmp;

    for ( auto c : cases )
    {
        auto myColl = new RimSummaryAddressCollection;
        myColl->setContentType( RimSummaryAddressCollection::CollectionContentType::SUMMARY_CASE );
        myColl->setCaseId( c->caseId() );
        tmp.push_back( myColl );
    }

    RicAppendSummaryPlotsForObjectsFeature::appendPlots( summaryMultiPlot, tmp );

    for ( auto obj : tmp )
    {
        delete obj;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryPlotsForSummaryCasesFeature::isCommandEnabled()
{
    return !selectedCases().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryCasesFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    auto summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( app->activePlotWindow() );
    if ( !summaryMultiPlot ) return;

    auto cases = selectedCases();
    if ( cases.empty() ) return;

    appendPlotsForCases( summaryMultiPlot, cases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryCasesFeature::setupActionLook( QAction* actionToSetup )
{
    QString objectType = "Cases";

    auto text = QString( "Append Plots For " ) + objectType;
    actionToSetup->setText( text );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicAppendSummaryPlotsForSummaryCasesFeature::selectedCases()
{
    std::vector<RimSummaryCase*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return objects;
}
