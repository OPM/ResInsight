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

#include "RicNewSummaryPlotFromDataVectorFeature.h"

#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RimSummaryAddress.h"
#include "RimSummaryPlot.h"

#include "RifEclipseSummaryAddress.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryPlotFromDataVectorFeature, "RicNewSummaryPlotFromDataVectorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotFromDataVectorFeature::isCommandEnabled() const
{
    const auto                      selectedItems        = caf::SelectionManager::instance()->selectedItems();
    std::vector<RimSummaryAddress*> selectedAddressItems = caf::selectedObjectsByType<RimSummaryAddress*>();

    unsigned int nEnsembles = 0;

    for ( auto adr : selectedAddressItems )
    {
        if ( adr->isEnsemble() ) nEnsembles++;
    }

    bool bOk = ( !selectedAddressItems.empty() );
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
    std::vector<RimSummaryAddress*>    selectedAddressItems = caf::selectedObjectsByType<RimSummaryAddress*>();
    std::set<int>                      caseIds;
    std::set<int>                      ensembleIds;
    std::vector<RimSummaryCase*>       selectedCases;
    std::set<RifEclipseSummaryAddress> eclipseAddresses;
    std::vector<RimSummaryEnsemble*>   selectedEnsembles;

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

    auto newPlot = RiaSummaryPlotTools::createPlot( eclipseAddresses, selectedCases, selectedEnsembles );
    RiaSummaryPlotTools::createAndAppendSingleSummaryMultiPlot( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFromDataVectorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
