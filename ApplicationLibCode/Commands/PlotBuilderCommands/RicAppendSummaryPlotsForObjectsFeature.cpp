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
void RicAppendSummaryPlotsForObjectsFeature::onActionTriggered( bool isChecked )
{
    // - Select a set of objects in Data Source (wells, groups, regions, ..)
    // - Use context menu to activate action
    // - For each plot in the current active plot, create a duplicate plot and replace the object name

    auto sumAddressCollections = selectedCollections();
    if ( sumAddressCollections.empty() ) return;

    RiaGuiApplication* app = RiaGuiApplication::instance();

    auto activePlotWindow = dynamic_cast<RimSummaryMultiPlot*>( app->activePlotWindow() );
    if ( activePlotWindow )
    {
        if ( !isSelectionCompatibleWithPlot( sumAddressCollections, activePlotWindow ) ) return;

        std::vector<RimSummaryPlot*> sourcePlots;
        {
            auto plots = activePlotWindow->plots();

            for ( auto p : plots )
            {
                auto sumPlot = dynamic_cast<RimSummaryPlot*>( p );

                if ( p ) sourcePlots.push_back( sumPlot );
            }
        }

        for ( auto summaryAdrCollection : sumAddressCollections )
        {
            auto duplicatedPlots = RicSummaryPlotBuilder::duplicateSummaryPlots( sourcePlots );
            for ( auto duplicatedPlot : duplicatedPlots )
            {
                auto curveSets = duplicatedPlot->curveSets();
                if ( !curveSets.empty() )
                {
                    for ( auto curveSet : curveSets )
                    {
                        auto sourceAddress = curveSet->summaryAddress();
                        auto modifiedAdr   = modifyAddress( sourceAddress, summaryAdrCollection );

                        curveSet->setSummaryAddress( modifiedAdr );
                    }
                }
                else
                {
                    auto curves = duplicatedPlot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS );
                    for ( auto c : curves )
                    {
                        auto sourceAddress = c->summaryAddressY();
                        auto modifiedAdr   = modifyAddress( sourceAddress, summaryAdrCollection );

                        c->setSummaryAddressY( modifiedAdr );
                    }
                }

                activePlotWindow->addPlot( duplicatedPlot );

                duplicatedPlot->resolveReferencesRecursively();
            }
        }

        activePlotWindow->loadDataAndUpdate();
    }
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
        if ( firstAdr->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL )
        {
            objectType = "Wells";
        }
        else if ( firstAdr->contentType() == RimSummaryAddressCollection::CollectionContentType::GROUP )
        {
            objectType = "Groups";
        }
        else if ( firstAdr->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION )
        {
            objectType = "Regions";
        }
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
        if ( coll->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL_FOLDER ||
             coll->contentType() == RimSummaryAddressCollection::CollectionContentType::GROUP_FOLDER ||
             coll->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION_FOLDER )
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

    RiaSummaryAddressAnalyzer analyzer;
    bool                      sourcePlotHasEnsembleData = false;

    {
        std::set<RifEclipseSummaryAddress> allAddresses;

        auto curveSets = summaryMultiPlot->curveSets();
        if ( !curveSets.empty() )
        {
            for ( auto curveSet : curveSets )
            {
                allAddresses.insert( curveSet->summaryAddress() );
            }
            sourcePlotHasEnsembleData = true;
        }
        else
        {
            auto curves = summaryMultiPlot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS );
            for ( auto c : curves )
            {
                allAddresses.insert( c->summaryAddressY() );
            }
        }

        analyzer.appendAddresses( allAddresses );
    }

    QString errorText;
    auto    selectionType = selection.front()->contentType();
    if ( selectionType == RimSummaryAddressCollection::CollectionContentType::WELL )
    {
        if ( analyzer.wellNames().size() != 1 )
        {
            errorText = "Source plot must contain one well only to be able to duplicate a selection of wells";
        }
    }
    else if ( selectionType == RimSummaryAddressCollection::CollectionContentType::GROUP )
    {
        if ( analyzer.groupNames().size() != 1 )
        {
            errorText = "Source plot must contain one well group only to be able to duplicate a selection of groups";
        }
    }
    else if ( selectionType == RimSummaryAddressCollection::CollectionContentType::REGION )
    {
        if ( analyzer.regionNumbers().size() != 1 )
        {
            errorText = "Source plot must contain one region only to be able to duplicate a selection of regions";
        }
    }

    if ( sourcePlotHasEnsembleData && selection.front()->ensembleId() == -1 )
    {
        errorText = "Source plot must contain single cases to be able to duplicate a selection of single cases";
    }
    else if ( !sourcePlotHasEnsembleData && selection.front()->caseId() == -1 )
    {
        errorText =
            "Source plot must contain ensemble case plots to be able to duplicate a selection of ensemble cases";
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
