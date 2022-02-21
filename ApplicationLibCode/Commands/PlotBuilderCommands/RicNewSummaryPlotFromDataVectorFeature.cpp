/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewSummaryPlotFromDataVectorFeature.h"

#include "RiaSummaryTools.h"
#include "RimSummaryAddress.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RicSummaryPlotBuilder.h"

#include "RifEclipseSummaryAddress.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryPlotFromDataVectorFeature, "RicNewSummaryPlotFromDataVectorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotFromDataVectorFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

    std::vector<RimSummaryAddress*> selectedAddressItems = caf::selectedObjectsByType<RimSummaryAddress*>();

    int nEnsembles = 0;

    for ( auto adr : selectedAddressItems )
    {
        if ( adr->isEnsemble() ) nEnsembles++;
    }

    bool bOk = ( selectedAddressItems.size() > 0 );
    if ( nEnsembles > 0 )
    {
        bOk = bOk && ( nEnsembles == selectedItems.size() );
    }
    bOk = bOk && ( selectedAddressItems.size() == selectedItems.size() );

    return bOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFromDataVectorFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryAddress*>        selectedAddressItems = caf::selectedObjectsByType<RimSummaryAddress*>();
    std::set<int>                          caseIds;
    std::set<int>                          ensembleIds;
    std::vector<RimSummaryCase*>           selectedCases;
    std::set<RifEclipseSummaryAddress>     eclipseAddresses;
    std::vector<RimSummaryCaseCollection*> selectedEnsembles;

    bool isEnsemble = false;

    for ( auto adr : selectedAddressItems )
    {
        eclipseAddresses.insert( adr->address() );
        caseIds.insert( adr->caseId() );
        ensembleIds.insert( adr->ensembleId() );
        isEnsemble = isEnsemble || adr->isEnsemble();
    }

    if ( isEnsemble )
    {
        for ( auto id : ensembleIds )
        {
            selectedEnsembles.push_back( RiaSummaryTools::ensembleById( id ) );
        }
    }
    else
    {
        for ( auto id : caseIds )
        {
            selectedCases.push_back( RiaSummaryTools::summaryCaseById( id ) );
        }
    }

    auto newPlot = RicSummaryPlotBuilder::createPlot( eclipseAddresses, selectedCases, selectedEnsembles );

    auto plotCollection = RiaSummaryTools::summaryPlotCollection();
    newPlot->setAsPlotMdiWindow();

    plotCollection->addPlot( newPlot );

    newPlot->loadDataAndUpdate();

    plotCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFromDataVectorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
