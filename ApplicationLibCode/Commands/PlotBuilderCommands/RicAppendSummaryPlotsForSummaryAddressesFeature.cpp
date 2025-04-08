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

#include "RicAppendSummaryPlotsForSummaryAddressesFeature.h"

#include "Summary/RiaSummaryPlotTools.h"

#include "RimSummaryAddress.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSummaryPlotsForSummaryAddressesFeature, "RicAppendSummaryPlotsForSummaryAddressesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryAddressesFeature::appendPlotsForAddresses( RimSummaryMultiPlot*                   summaryMultiPlot,
                                                                               const std::vector<RimSummaryAddress*>& addresses )
{
    if ( !summaryMultiPlot ) return;
    if ( addresses.empty() ) return;

    caf::ProgressInfo info( addresses.size(), "Appending plots..." );

    for ( auto adr : addresses )
    {
        auto* plot = new RimSummaryPlot();
        plot->enableAutoPlotTitle( true );
        plot->handleDroppedObjects( { adr } );

        summaryMultiPlot->addPlot( plot );

        info.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryPlotsForSummaryAddressesFeature::isCommandEnabled() const
{
    return !selectedAddresses().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryAddressesFeature::onActionTriggered( bool isChecked )
{
    auto addresses = selectedAddresses();
    if ( addresses.empty() ) return;

    auto selectedMultiPlots = RiaSummaryPlotTools::selectedSummaryMultiPlots();
    for ( auto summaryMultiPlot : selectedMultiPlots )
    {
        appendPlotsForAddresses( summaryMultiPlot, addresses );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForSummaryAddressesFeature::setupActionLook( QAction* actionToSetup )
{
    auto text = QString( "Append Plots For Vector" );
    actionToSetup->setText( text );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddress*> RicAppendSummaryPlotsForSummaryAddressesFeature::selectedAddresses()
{
    return caf::SelectionManager::instance()->objectsByType<RimSummaryAddress>();
}
