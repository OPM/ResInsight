/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicCreateCrossPlotFeature.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "RiaPreferencesSummary.h"
#include "RiaSummaryCurveAddress.h"

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "cafSelectionManagerTools.h"

#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include <QAction>
#include <QMenu>

CAF_CMD_SOURCE_INIT( RicCreateCrossPlotFeature, "RicCreateCrossPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateCrossPlotFeature::isCommandEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateCrossPlotFeature::onActionTriggered( bool isChecked )
{
    // Nothing to do here, the sub menus are handled by the onSubMenuActionTriggered
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateCrossPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Summary Cross Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryXPlotLight16x16.png" ) );

    auto* subMenu = new QMenu( "Create Cross Plot" );

    auto text = RiaPreferencesSummary::current()->crossPlotAddressCombinations();

    auto collectionContentType = RimSummaryAddressCollection::CollectionContentType::NOT_DEFINED;

    std::string selectedWellName;
    std::string selectedGroupName;

    if ( auto addrCollection = dynamic_cast<RimSummaryAddressCollection*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        collectionContentType = addrCollection->contentType();
    }

    if ( auto addr = dynamic_cast<RimSummaryAddress*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        selectedWellName  = addr->address().wellName();
        selectedGroupName = addr->address().groupName();
    }

    bool isWell  = ( ( collectionContentType == RimSummaryAddressCollection::CollectionContentType::WELL ) || !selectedWellName.empty() );
    bool isGroup = ( ( collectionContentType == RimSummaryAddressCollection::CollectionContentType::GROUP ) || !selectedGroupName.empty() );

    auto textList = text.split( ";" );
    for ( const auto& s : textList )
    {
        QString menuText = s;

        auto action = subMenu->addAction( s );
        action->setIcon( QIcon( ":/SummaryXPlotLight16x16.png" ) );

        connect( action, &QAction::triggered, this, &RicCreateCrossPlotFeature::onSubMenuActionTriggered );
    }

    actionToSetup->setMenu( subMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateCrossPlotFeature::onSubMenuActionTriggered( bool isChecked )
{
    QString addressX;
    QString addressY;

    auto* action = qobject_cast<QAction*>( sender() );

    if ( action )
    {
        QString text         = action->text();
        auto    addressTexts = text.split( " " );

        if ( !addressTexts.empty() ) addressY = addressTexts[0];
        if ( addressTexts.size() > 1 ) addressX = addressTexts[1];
    }

    RifEclipseSummaryAddress adrX = RifEclipseSummaryAddress::fromEclipseTextAddress( addressX.toStdString() );
    RifEclipseSummaryAddress adrY = RifEclipseSummaryAddress::fromEclipseTextAddress( addressY.toStdString() );

    RiaSummaryCurveAddress curveAddress( adrX, adrY );

    auto selectedCases     = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCase>();
    auto selectedEnsembles = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCaseCollection>();

    auto newPlot = RicSummaryPlotBuilder::createCrossPlot( { curveAddress }, { selectedCases }, { selectedEnsembles } );

    RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( newPlot );
}
