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

#include "RicNewSummaryMultiPlotFromDataVectorFeature.h"

#include "RiaPreferencesSummary.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RifEclipseSummaryAddress.h"
#include "RifReaderEclipseSummary.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryMultiPlotFromDataVectorFeature, "RicNewSummaryMultiPlotFromDataVectorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryMultiPlotFromDataVectorFeature::isCommandEnabled() const
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
void RicNewSummaryMultiPlotFromDataVectorFeature::onActionTriggered( bool isChecked )
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

    std::set<RifEclipseSummaryAddress> availableAddresses;

    if ( isEnsemble )
    {
        for ( auto id : ensembleIds )
        {
            auto ensemble = RiaSummaryTools::ensembleById( id );
            if ( ensemble )
            {
                selectedEnsembles.push_back( ensemble );

                if ( availableAddresses.empty() ) availableAddresses = ensemble->ensembleSummaryAddresses();
            }
        }
    }
    else
    {
        for ( auto id : caseIds )
        {
            auto summaryCase = RiaSummaryTools::summaryCaseById( id );
            if ( summaryCase )
            {
                selectedCases.push_back( summaryCase );
                if ( availableAddresses.empty() && summaryCase->summaryReader() )
                    availableAddresses = summaryCase->summaryReader()->allResultAddresses();
            }
        }
    }

    if ( RiaPreferencesSummary::current()->appendHistoryVectors() )
    {
        auto sourceAddresses = eclipseAddresses;

        for ( const auto& addr : sourceAddresses )
        {
            if ( !addr.isHistoryVector() )
            {
                auto historyAddr = addr;
                historyAddr.setVectorName( addr.vectorName() + RifEclipseSummaryAddressDefines::historyIdentifier() );

                if ( availableAddresses.count( historyAddr ) > 0 ) eclipseAddresses.insert( historyAddr );
            }
        }
    }

    auto newPlot = RiaSummaryPlotTools::createPlot( eclipseAddresses, selectedCases, selectedEnsembles );

    std::vector<RimSummaryPlot*> plots{ newPlot };

    RiaSummaryPlotTools::createAndAppendSummaryMultiPlot( plots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFromDataVectorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
