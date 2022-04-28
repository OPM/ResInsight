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

#include "RicAppendSummaryPlotsForObjectsFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryAddressAnalyzer.h"

#include "RicSummaryPlotBuilder.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryAddressModifier.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "cafAssert.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSummaryPlotsForObjectsFeature, "RicAppendSummaryPlotsForObjectsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryPlotsForObjectsFeature::isCommandEnabled()
{
    return !selectedCollections().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForObjectsFeature::appendPlot( const std::vector<RimSummaryAddressCollection*>& sumAddressCollections,
                                                         RimSummaryMultiPlot* summaryMultiPlot )
{
    isSelectionCompatibleWithPlot( sumAddressCollections, summaryMultiPlot );

    auto                         selectionType       = sumAddressCollections.front()->contentType();
    auto                         sourcePlots         = summaryMultiPlot->summaryPlots();
    std::vector<RimSummaryPlot*> plotsForOneInstance = plotsForOneInstanceOfObjectType( sourcePlots, selectionType );

    for ( auto summaryAdrCollection : sumAddressCollections )
    {
        auto duplicatedPlots = RicSummaryPlotBuilder::duplicateSummaryPlots( plotsForOneInstance );
        for ( auto duplicatedPlot : duplicatedPlots )
        {
            auto adrMods = RimSummaryAddressModifier::createAddressModifiersForPlot( duplicatedPlot );
            for ( auto adrMod : adrMods )
            {
                auto sourceAddress = adrMod.address();
                auto modifiedAdr   = modifyAddress( sourceAddress, summaryAdrCollection );

                adrMod.setAddress( modifiedAdr );
            }

            summaryMultiPlot->addPlot( duplicatedPlot );

            duplicatedPlot->resolveReferencesRecursively();
        }
    }

    summaryMultiPlot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForObjectsFeature::onActionTriggered( bool isChecked )
{
    // - Select a set of objects in Data Source (wells, groups, regions, ..)
    // - Use context menu to activate action
    // - For each plot in the current active plot, create a duplicate plot and replace the object name

    auto sumAddressCollections = selectedCollections();
    if ( sumAddressCollections.empty() ) return;

    RiaGuiApplication* app = RiaGuiApplication::instance();

    auto summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( app->activePlotWindow() );
    if ( !summaryMultiPlot ) return;

    appendPlot( sumAddressCollections, summaryMultiPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForObjectsFeature::setupActionLook( QAction* actionToSetup )
{
    QString objectType = "Objects";

    auto addresses = selectedCollections();

    if ( !addresses.empty() )
    {
        auto firstAdr = addresses.front();
        objectType = caf::AppEnum<RimSummaryAddressCollection::CollectionContentType>::uiText( firstAdr->contentType() );
    }

    auto text = QString( "Append Plots For " ) + objectType;
    actionToSetup->setText( text );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressCollection*> RicAppendSummaryPlotsForObjectsFeature::selectedCollections()
{
    std::vector<RimSummaryAddressCollection*> sumAddressCollections;
    caf::SelectionManager::instance()->objectsByType( &sumAddressCollections );

    if ( sumAddressCollections.size() == 1 )
    {
        auto coll = sumAddressCollections[0];
        if ( coll->isFolder() )
        {
            // If a folder is selected, return all sub items in folder
            auto childObjects = coll->subFolders();

            return childObjects;
        }
    }

    return sumAddressCollections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryPlotsForObjectsFeature::isSelectionCompatibleWithPlot(
    const std::vector<RimSummaryAddressCollection*>& selection,
    RimSummaryMultiPlot*                             summaryMultiPlot )
{
    if ( !summaryMultiPlot ) return false;
    if ( selection.empty() ) return false;

    auto selectionType = selection.front()->contentType();

    RiaSummaryAddressAnalyzer analyzer;

    {
        // Find all plots for one object type
        auto sourcePlots = summaryMultiPlot->summaryPlots();

        std::vector<RimSummaryPlot*> plotsForObjectType =
            RicAppendSummaryPlotsForObjectsFeature::plotsForOneInstanceOfObjectType( sourcePlots, selectionType );

        for ( auto plot : plotsForObjectType )
        {
            auto addresses = RimSummaryAddressModifier::createEclipseSummaryAddress( plot );
            analyzer.appendAddresses( addresses );
        }
    }

    QString errorText;
    if ( selectionType == RimSummaryAddressCollection::CollectionContentType::WELL )
    {
        if ( analyzer.wellNames().empty() )
        {
            errorText = "Source plot must contain at least one well to be able to duplicate a selection of wells";
        }
    }
    else if ( selectionType == RimSummaryAddressCollection::CollectionContentType::GROUP )
    {
        if ( analyzer.groupNames().empty() )
        {
            errorText = "Source plot must contain at least one group to be able to duplicate a selection of groups";
        }
    }
    else if ( selectionType == RimSummaryAddressCollection::CollectionContentType::REGION )
    {
        if ( analyzer.regionNumbers().empty() )
        {
            errorText = "Source plot must contain at least one region to be able to duplicate a selection of regions";
        }
    }

    if ( !errorText.isEmpty() )
    {
        RiaLogging::error( errorText );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RicAppendSummaryPlotsForObjectsFeature::modifyAddress( const RifEclipseSummaryAddress& sourceAddress,
                                                           RimSummaryAddressCollection*    summaryAddressCollection )
{
    CAF_ASSERT( summaryAddressCollection );

    auto adr = sourceAddress;

    auto objectName = summaryAddressCollection->name().toStdString();
    if ( summaryAddressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL )
    {
        adr.setWellName( objectName );
    }
    else if ( summaryAddressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::GROUP )
    {
        adr.setGroupName( objectName );
    }
    else if ( summaryAddressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION )
    {
        int intValue = RiaStdStringTools::toInt( objectName );
        if ( intValue == -1 )
        {
            QString errorText = QString( "Failed to convert region text to region integer value "
                                         "for region text : " ) +
                                summaryAddressCollection->name();

            RiaLogging::error( errorText );
        }
        else
        {
            adr.setRegion( intValue );
        }
    }

    return adr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RicAppendSummaryPlotsForObjectsFeature::plotsForOneInstanceOfObjectType(
    const std::vector<RimSummaryPlot*>&                sourcePlots,
    RimSummaryAddressCollection::CollectionContentType objectType )
{
    std::vector<RimSummaryPlot*> plotsForOneInstance;

    std::string wellNameToMatch;
    std::string groupNameToMatch;
    int         regionToMatch = -1;

    RiaSummaryAddressAnalyzer myAnalyser;
    for ( auto sourcePlot : sourcePlots )
    {
        auto addresses = RimSummaryAddressModifier::createEclipseSummaryAddress( sourcePlot );
        myAnalyser.appendAddresses( addresses );
    }

    if ( objectType == RimSummaryAddressCollection::CollectionContentType::WELL )
    {
        if ( !myAnalyser.wellNames().empty() ) wellNameToMatch = *( myAnalyser.wellNames().begin() );
    }
    else if ( objectType == RimSummaryAddressCollection::CollectionContentType::GROUP )
    {
        if ( !myAnalyser.groupNames().empty() ) groupNameToMatch = *( myAnalyser.groupNames().begin() );
    }
    else if ( objectType == RimSummaryAddressCollection::CollectionContentType::REGION )
    {
        if ( !myAnalyser.regionNumbers().empty() ) regionToMatch = *( myAnalyser.regionNumbers().begin() );
    }

    for ( auto sourcePlot : sourcePlots )
    {
        auto addresses = RimSummaryAddressModifier::createEclipseSummaryAddress( sourcePlot );

        bool isMatching = false;
        for ( const auto& a : addresses )
        {
            if ( !wellNameToMatch.empty() && a.wellName() == wellNameToMatch )
            {
                isMatching = true;
            }
            else if ( !groupNameToMatch.empty() && a.groupName() == groupNameToMatch )
            {
                isMatching = true;
            }
            else if ( regionToMatch != -1 && a.regionNumber() == regionToMatch )
            {
                isMatching = true;
            }
        }

        if ( isMatching ) plotsForOneInstance.push_back( sourcePlot );
    }

    return plotsForOneInstance;
}
