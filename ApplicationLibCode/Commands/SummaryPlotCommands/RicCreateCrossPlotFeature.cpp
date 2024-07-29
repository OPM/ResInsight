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
#include "RimSummaryEnsemble.h"

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

auto newCrossPlotText = []() -> QString { return "Custom Cross Plot"; };

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateCrossPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Summary Cross Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryXPlotLight16x16.png" ) );

    auto subMenu = new QMenu;

    auto menuTexts = crossPlotAddressesBasedOnSelection();
    for ( const auto& crossPlotAddresses : menuTexts )
    {
        auto action = subMenu->addAction( crossPlotAddresses );
        action->setIcon( QIcon( ":/SummaryXPlotLight16x16.png" ) );

        connect( action, &QAction::triggered, this, &RicCreateCrossPlotFeature::onSubMenuActionTriggered );
    }

    subMenu->addSeparator();

    auto newCustomPlotAction = subMenu->addAction( newCrossPlotText() );
    newCustomPlotAction->setIcon( QIcon( ":/SummaryXPlotLight16x16.png" ) );

    connect( newCustomPlotAction, &QAction::triggered, this, &RicCreateCrossPlotFeature::onSubMenuActionTriggered );

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
        auto text = action->text();
        if ( text != newCrossPlotText() )
        {
            auto words = action->text().split( " " );

            if ( !words.empty() ) addressY = words[0];
            if ( words.size() > 1 ) addressX = words[1];
        }
    }

    RifEclipseSummaryAddress adrX = RifEclipseSummaryAddress::fromEclipseTextAddress( addressX.toStdString() );
    RifEclipseSummaryAddress adrY = RifEclipseSummaryAddress::fromEclipseTextAddress( addressY.toStdString() );

    RiaSummaryCurveAddress curveAddress( adrX, adrY );

    auto selectedCases     = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCase>();
    auto selectedEnsembles = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryEnsemble>();

    auto newPlot = RicSummaryPlotBuilder::createCrossPlot( { curveAddress }, { selectedCases }, { selectedEnsembles } );

    RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( newPlot );
}

//--------------------------------------------------------------------------------------------------
// Returns a list of cross plot address combinations based on the current selection
// The vectors defined in preferences is given in field vectors
// If the selection is a field address, use the list from preferences
// If the selection is a well address, F is replaced with W and well name is appended
// If the selection is a group address, F is replaced with G and group name is appended
//--------------------------------------------------------------------------------------------------
QStringList RicCreateCrossPlotFeature::crossPlotAddressesBasedOnSelection()
{
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

    QStringList crossPlotAddressCombinations;

    auto textList = text.split( ";" );
    for ( const auto& inputText : textList )
    {
        auto modifiedString = inputText.toStdString();

        if ( isWell )
        {
            std::string wellAddress;

            auto words = inputText.split( " " );
            for ( const auto& w : words )
            {
                auto tmp = w.toStdString().substr( 1 );

                tmp = "W" + tmp + ":" + wellName;

                if ( !wellAddress.empty() ) wellAddress += " ";
                wellAddress += tmp;
            }

            modifiedString = wellAddress;
        }
        else if ( isGroup )
        {
            std::string groupAddress;

            auto words = inputText.split( " " );
            for ( const auto& w : words )
            {
                auto tmp = w.toStdString().substr( 1 );

                tmp = "G" + tmp + ":" + groupName;

                if ( !groupAddress.empty() ) groupAddress += " ";
                groupAddress += tmp;
            }

            modifiedString = groupAddress;
        }

        crossPlotAddressCombinations.append( QString::fromStdString( modifiedString ) );
    }

    return crossPlotAddressCombinations;
}
