/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicNewDerivedSummaryFeature.h"

#include "RimDerivedSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDerivedSummaryFeature, "RicNewDerivedSummaryFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDerivedSummaryFeature::isCommandEnabled()
{
    if ( mainCollection() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDerivedSummaryFeature::onActionTriggered( bool isChecked )
{
    auto mainColl = mainCollection();
    if ( mainColl )
    {
        auto derivedCase = new RimDerivedSummaryCase;

        auto selectedCases = twoSelectedSummaryCases();
        if ( !selectedCases.empty() )
        {
            derivedCase->setSummaryCases( selectedCases[0], selectedCases[1] );
            derivedCase->createSummaryReaderInterface();
        }

        mainColl->addCase( derivedCase );
        derivedCase->updateDisplayNameFromCases();

        mainColl->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( derivedCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDerivedSummaryFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Delta Summary Case" );
    actionToSetup->setIcon( QIcon( ":/SummaryCase.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection* RicNewDerivedSummaryFeature::mainCollection()
{
    std::vector<RimSummaryCaseMainCollection*> mainColls =
        caf::selectedObjectsByTypeStrict<RimSummaryCaseMainCollection*>();

    if ( mainColls.size() == 1 ) return mainColls.front();

    auto sumCases = twoSelectedSummaryCases();
    if ( !sumCases.empty() )
    {
        RimSummaryCaseMainCollection* mainColl = nullptr;
        sumCases.front()->firstAncestorOfType( mainColl );
        return mainColl;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicNewDerivedSummaryFeature::twoSelectedSummaryCases()
{
    auto sumCases = caf::selectedObjectsByTypeStrict<RimSummaryCase*>();
    if ( sumCases.size() == 2 ) return sumCases;

    return {};
}
