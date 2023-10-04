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

#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "cafSelectionManagerTools.h"

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

    std::string wellName;
    std::string groupName;

    if ( auto addr = dynamic_cast<RimSummaryAddress*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        wellName  = addr->address().wellName();
        groupName = addr->address().groupName();
    }

    if ( auto addrCollection = dynamic_cast<RimSummaryAddressCollection*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        collectionContentType = addrCollection->contentType();

        if ( ( collectionContentType == RimSummaryAddressCollection::CollectionContentType::WELL ) && wellName.empty() )
        {
            auto addresses = addrCollection->descendantsOfType<RimSummaryAddress>();
            if ( !addresses.empty() )
            {
                wellName = addresses.front()->address().wellName();
            }
        }

        if ( ( collectionContentType == RimSummaryAddressCollection::CollectionContentType::GROUP ) && groupName.empty() )
        {
            auto addresses = addrCollection->descendantsOfType<RimSummaryAddress>();
            if ( !addresses.empty() )
            {
                groupName = addresses.front()->address().groupName();
            }
        }
    }

    bool isWell  = ( ( collectionContentType == RimSummaryAddressCollection::CollectionContentType::WELL ) || !wellName.empty() );
    bool isGroup = ( ( collectionContentType == RimSummaryAddressCollection::CollectionContentType::GROUP ) || !groupName.empty() );

    auto textList = text.split( ";" );
    for ( const auto& s : textList )
    {
        auto originalString = s.toStdString();
        auto modifiedString = originalString;

        if ( isWell )
        {
            std::string wellString;

            auto words = s.split( " " );
            for ( const auto& w : words )
            {
                auto tmp = w.toStdString().substr( 1 );

                tmp = "W" + tmp + ":" + wellName;

                if ( !wellString.empty() ) wellString += " ";
                wellString += tmp;
            }

            modifiedString = wellString;
        }
        else if ( isGroup )
        {
            std::string groupString;

            auto words = s.split( " " );
            for ( const auto& w : words )
            {
                auto tmp = w.toStdString().substr( 1 );

                tmp = "G" + tmp + ":" + groupName;

                if ( !groupString.empty() ) groupString += " ";
                groupString += tmp;
            }

            modifiedString = groupString;
        }

        auto action = subMenu->addAction( QString::fromStdString( modifiedString ) );
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

    if ( auto* action = qobject_cast<QAction*>( sender() ) )
    {
        auto words = action->text().split( " " );

        if ( !words.empty() ) addressY = words[0];
        if ( words.size() > 1 ) addressX = words[1];
    }

    RifEclipseSummaryAddress adrX = RifEclipseSummaryAddress::fromEclipseTextAddress( addressX.toStdString() );
    RifEclipseSummaryAddress adrY = RifEclipseSummaryAddress::fromEclipseTextAddress( addressY.toStdString() );

    RiaSummaryCurveAddress curveAddress( adrX, adrY );

    auto selectedCases     = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCase>();
    auto selectedEnsembles = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCaseCollection>();

    auto newPlot = RicSummaryPlotBuilder::createCrossPlot( { curveAddress }, { selectedCases }, { selectedEnsembles } );

    RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( newPlot );
}
