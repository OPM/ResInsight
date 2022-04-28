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

#include "RicAppendSummaryCurvesForObjectsFeature.h"

#include "RiaGuiApplication.h"
#include "RicAppendSummaryPlotsForObjectsFeature.h"

#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSummaryCurvesForObjectsFeature, "RicAppendSummaryCurvesForObjectsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryCurvesForObjectsFeature::isCommandEnabled()
{
    return !RicAppendSummaryPlotsForObjectsFeature::selectedCollections().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryCurvesForObjectsFeature::onActionTriggered( bool isChecked )
{
    // - Select a set of objects in Data Source (wells, groups, regions, ..)
    // - Use context menu to activate action
    // - For each plots, append curves using handleDroppedObjects() on RimSummaryPlot

    auto sumAddressCollections = RicAppendSummaryPlotsForObjectsFeature::selectedCollections();
    if ( sumAddressCollections.empty() ) return;

    RiaGuiApplication* app = RiaGuiApplication::instance();

    auto summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( app->activePlotWindow() );
    if ( !summaryMultiPlot ) return;

    RicAppendSummaryPlotsForObjectsFeature::isSelectionCompatibleWithPlot( sumAddressCollections, summaryMultiPlot );

    auto selectionType = sumAddressCollections.front()->contentType();
    auto sourcePlots   = summaryMultiPlot->summaryPlots();
    auto plotsForOneInstance =
        RicAppendSummaryPlotsForObjectsFeature::plotsForOneInstanceOfObjectType( sourcePlots, selectionType );

    std::vector<caf::PdmObjectHandle*> pdmObjects;
    for ( auto summaryAdrCollection : sumAddressCollections )
    {
        pdmObjects.push_back( summaryAdrCollection );
    }

    for ( auto plot : sourcePlots )
    {
        plot->handleDroppedObjects( pdmObjects );
    }

    summaryMultiPlot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryCurvesForObjectsFeature::setupActionLook( QAction* actionToSetup )
{
    QString objectType = "Objects";

    auto addresses = RicAppendSummaryPlotsForObjectsFeature::selectedCollections();

    if ( !addresses.empty() )
    {
        auto firstAdr = addresses.front();
        objectType = caf::AppEnum<RimSummaryAddressCollection::CollectionContentType>::uiText( firstAdr->contentType() );
    }

    auto text = QString( "Append Curves For " ) + objectType;
    actionToSetup->setText( text );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}
