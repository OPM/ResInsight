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
#include "RiaSummaryTools.h"

#include "RicSummaryPlotBuilder.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryAddressModifier.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "cafAssert.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSummaryPlotsForObjectsFeature, "RicAppendSummaryPlotsForObjectsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSummaryPlotsForObjectsFeature::isCommandEnabled() const
{
    return !selectedCollections().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForObjectsFeature::appendPlots( RimSummaryMultiPlot*                             summaryMultiPlot,
                                                          const std::vector<RimSummaryAddressCollection*>& sumAddressCollections )
{
    if ( sumAddressCollections.empty() ) return;

    isSelectionCompatibleWithPlot( sumAddressCollections, summaryMultiPlot );

    auto selectionType       = sumAddressCollections.front()->contentType();
    auto sourcePlots         = summaryMultiPlot->summaryPlots();
    auto plotsForOneInstance = plotsForOneInstanceOfObjectType( sourcePlots, selectionType );

    caf::ProgressInfo info( sumAddressCollections.size(), "Appending plots..." );

    for ( auto summaryAdrCollection : sumAddressCollections )
    {
        auto duplicatedPlots = RicSummaryPlotBuilder::duplicateSummaryPlots( plotsForOneInstance );

        for ( auto duplicatedPlot : duplicatedPlots )
        {
            if ( summaryAdrCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::SUMMARY_CASE )
            {
                summaryMultiPlot->addPlot( duplicatedPlot );
                duplicatedPlot->resolveReferencesRecursively();

                auto summaryCase = RiaSummaryTools::summaryCaseById( summaryAdrCollection->caseId() );
                if ( summaryCase )
                {
                    for ( auto c : duplicatedPlot->summaryCurves() )
                    {
                        c->setSummaryCaseY( summaryCase );
                        c->setSummaryCaseX( summaryCase );
                    }
                }

                auto ensemble = RiaSummaryTools::ensembleById( summaryAdrCollection->ensembleId() );
                if ( ensemble )
                {
                    for ( auto c : duplicatedPlot->curveSets() )
                    {
                        c->setSummaryEnsemble( ensemble );
                    }
                }
            }
            else
            {
                const auto objectName     = summaryAdrCollection->name().toStdString();
                auto       contentType    = summaryAdrCollection->contentType();
                auto       curveProviders = RimSummaryAddressModifier::createAddressProviders( duplicatedPlot );
                RimSummaryAddressModifier::updateAddressesByObjectName( curveProviders, objectName, contentType );

                summaryMultiPlot->addPlot( duplicatedPlot );
                duplicatedPlot->resolveReferencesRecursively();
            }

            duplicatedPlot->loadDataAndUpdate();
        }
        info.incrementProgress();
    }

    summaryMultiPlot->updatePlotTitles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSummaryPlotsForObjectsFeature::appendPlots( RimSummaryMultiPlot*                    summaryMultiPlot,
                                                          const std::vector<RimSummaryCase*>&     cases,
                                                          const std::vector<RimSummaryEnsemble*>& ensembles )
{
    auto addressCollectionsToBeDeleted = RicAppendSummaryPlotsForObjectsFeature::createAddressCollections( cases, ensembles );
    RicAppendSummaryPlotsForObjectsFeature::appendPlots( summaryMultiPlot, addressCollectionsToBeDeleted );

    for ( auto obj : addressCollectionsToBeDeleted )
    {
        delete obj;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressCollection*>
    RicAppendSummaryPlotsForObjectsFeature::createAddressCollections( const std::vector<RimSummaryCase*>&     cases,
                                                                      const std::vector<RimSummaryEnsemble*>& ensembles )
{
    std::vector<RimSummaryAddressCollection*> addresses;

    for ( auto c : cases )
    {
        auto myColl = new RimSummaryAddressCollection;
        myColl->setContentType( RimSummaryAddressCollection::CollectionContentType::SUMMARY_CASE );
        myColl->setCaseId( c->caseId() );
        addresses.push_back( myColl );
    }

    for ( auto c : ensembles )
    {
        auto myColl = new RimSummaryAddressCollection;
        myColl->setContentType( RimSummaryAddressCollection::CollectionContentType::SUMMARY_CASE );
        myColl->setEnsembleId( c->ensembleId() );
        addresses.push_back( myColl );
    }

    return addresses;
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

    appendPlots( summaryMultiPlot, sumAddressCollections );
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
        objectType    = caf::AppEnum<RimSummaryAddressCollection::CollectionContentType>::uiText( firstAdr->contentType() );
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
bool RicAppendSummaryPlotsForObjectsFeature::isSelectionCompatibleWithPlot( const std::vector<RimSummaryAddressCollection*>& selection,
                                                                            RimSummaryMultiPlot* summaryMultiPlot )
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
            auto addresses = RimSummaryAddressModifier::allSummaryAddressesY( plot );
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
std::vector<RimSummaryPlot*>
    RicAppendSummaryPlotsForObjectsFeature::plotsForOneInstanceOfObjectType( const std::vector<RimSummaryPlot*>&                sourcePlots,
                                                                             RimSummaryAddressCollection::CollectionContentType objectType )
{
    std::vector<RimSummaryPlot*> plotsForOneInstance;

    std::string wellNameToMatch;
    std::string groupNameToMatch;
    int         regionToMatch     = -1;
    int         caseIdToMatch     = -1;
    int         ensembleIdToMatch = -1;

    RiaSummaryAddressAnalyzer myAnalyser;
    for ( auto sourcePlot : sourcePlots )
    {
        auto addresses = RimSummaryAddressModifier::allSummaryAddressesY( sourcePlot );
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
    else if ( objectType == RimSummaryAddressCollection::CollectionContentType::SUMMARY_CASE )
    {
        if ( !sourcePlots.empty() )
        {
            auto curves = sourcePlots.back()->summaryCurves();
            if ( !curves.empty() )
            {
                caseIdToMatch = curves.front()->summaryCaseY()->caseId();
            }
            auto curveSets = sourcePlots.back()->curveSets();
            if ( !curveSets.empty() )
            {
                ensembleIdToMatch = curveSets.front()->ensembleId();
            }
        }
    }

    for ( auto sourcePlot : sourcePlots )
    {
        bool isMatching = false;

        if ( caseIdToMatch != -1 )
        {
            auto curves = sourcePlot->summaryCurves();
            for ( auto c : curves )
            {
                if ( c->summaryCaseY()->caseId() == caseIdToMatch ) isMatching = true;
            }
        }
        else if ( ensembleIdToMatch != -1 )
        {
            auto curveSets = sourcePlot->curveSets();
            for ( auto c : curveSets )
            {
                if ( c->summaryEnsemble()->ensembleId() == ensembleIdToMatch ) isMatching = true;
            }
        }
        else
        {
            auto addresses = RimSummaryAddressModifier::allSummaryAddressesY( sourcePlot );

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
        }

        if ( isMatching ) plotsForOneInstance.push_back( sourcePlot );
    }

    return plotsForOneInstance;
}
