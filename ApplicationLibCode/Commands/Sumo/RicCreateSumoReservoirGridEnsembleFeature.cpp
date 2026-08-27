/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RicCreateSumoReservoirGridEnsembleFeature.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "Cloud/RiaSumoConnector.h"

#include "RicNewViewFeature.h"

#include "Cloud/RimSumoDataSource.h"
#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirGridEnsembleSumo.h"
#include "RimViewNameConfig.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <algorithm>
#include <format>

CAF_CMD_SOURCE_INIT( RicCreateSumoReservoirGridEnsembleFeature, "RicCreateSumoReservoirGridEnsembleFeature" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// Log the IJK dimensions Sumo reports for each selected realization, and the grid mode they lead to.
//--------------------------------------------------------------------------------------------------
void logRealizationDimensions( const QString& gridName, const SumoGridInfo& gridInfo, const std::vector<int>& realizations, bool dimensionsAreIdentical )
{
    RiaLogging::debug( std::format( "Sumo grid '{}': {} of {} realizations selected",
                                    gridName.toStdString(),
                                    realizations.size(),
                                    gridInfo.realizationInfos.size() ) );

    for ( int realization : realizations )
    {
        auto it = std::ranges::find( gridInfo.realizationInfos, realization, &SumoGridRealizationInfo::realization );
        if ( it == gridInfo.realizationInfos.end() ) continue;

        RiaLogging::debug(
            std::format( "  realization {}: {}x{}x{}", realization, it->dimensions.iCount, it->dimensions.jCount, it->dimensions.kCount ) );
    }

    RiaLogging::debug( dimensionsAreIdentical ? "  -> dimensions identical, shared grid mode" : "  -> dimensions differ, individual grids" );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoReservoirGridEnsembleFeature::onActionTriggered( bool isChecked )
{
    auto dataSources = caf::selectedObjectsByType<RimSumoDataSource*>();

    for ( auto dataSource : dataSources )
    {
        createGridEnsemble( dataSource );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoReservoirGridEnsembleFeature::createGridEnsemble( RimSumoDataSource* dataSource )
{
    if ( !dataSource ) return;

    const QString gridName = dataSource->selectedGridName();
    if ( gridName.isEmpty() )
    {
        RiaLogging::warning( "No grid selected. Unable to create grid ensemble from Sumo." );
        return;
    }

    RimProject* project = RimProject::current();
    if ( !project ) return;

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return;

    RimEclipseCaseCollection* analysisModels = oilfield->analysisModels();
    if ( !analysisModels ) return;

    auto sumoConnector = RiaApplication::instance()->makeSumoConnector();
    if ( !sumoConnector )
    {
        RiaLogging::warning( "No Sumo connector available, unable to create grid ensemble from Sumo." );
        return;
    }

    // The realizations of the selected grid, each with the IJK dimensions Sumo reports for it. The
    // dimensions decide whether the realizations can share one grid, so no grid is downloaded to find out.
    const auto gridInfo = sumoConnector->grid().gridInfo( dataSource->caseId(), dataSource->ensembleName(), gridName );

    // The response body is discarded on any failed request, so an empty result is also what a failed
    // transfer looks like. Report that separately from a grid the filter selected no realizations of.
    if ( gridInfo.realizationInfos.empty() )
    {
        RiaLogging::warning( QString( "No realizations found for grid '%1' in ensemble '%2'. No grid ensemble created, "
                                      "see the log for a failed request." )
                                 .arg( gridName, dataSource->ensembleName() )
                                 .toStdString() );
        return;
    }

    const auto gridRealizations = gridInfo.realizationIds();

    // The realizations selected on the data source are the realizations of the ensemble. That selection says
    // nothing about which data types exist for them, so the grid can be missing for some. A realization the
    // grid does not exist for would only produce a case failing to load, so skip it - but report it, so an
    // ensemble holding fewer cases than the selection is visible rather than silent.
    std::vector<int> realizations;
    std::vector<int> realizationsWithoutGrid;
    for ( const QString& realizationId : dataSource->selectedRealizationIds() )
    {
        bool ok          = false;
        int  realization = realizationId.toInt( &ok );
        if ( !ok ) continue;

        if ( std::ranges::find( gridRealizations, realization ) == gridRealizations.end() )
        {
            realizationsWithoutGrid.push_back( realization );
            continue;
        }

        realizations.push_back( realization );
    }

    if ( !realizationsWithoutGrid.empty() )
    {
        std::ranges::sort( realizationsWithoutGrid );
        RiaLogging::warning( QString( "Grid '%1' has no data for %2 of the selected realizations, no cases created for "
                                      "them: %3" )
                                 .arg( gridName )
                                 .arg( realizationsWithoutGrid.size() )
                                 .arg( QString::fromStdString( RiaStdStringTools::formatRangeSelection( realizationsWithoutGrid ) ) )
                                 .toStdString() );
    }

    // The realizations of the data source are the realizations of the ensemble, so the grid should not report
    // anything outside them. Leave a trace if it does, rather than reporting an inconsistency the user cannot
    // act on. realizations holds exactly the grid realizations that are also selected, so a grid realization
    // missing from it is one the ensemble does not have.
    std::vector<int> realizationsNotInEnsemble;
    for ( int gridRealization : gridRealizations )
    {
        if ( std::ranges::find( realizations, gridRealization ) == realizations.end() )
        {
            realizationsNotInEnsemble.push_back( gridRealization );
        }
    }

    if ( !realizationsNotInEnsemble.empty() )
    {
        RiaLogging::debug( QString( "Grid '%1' reports %2 realization(s) that are not selected realizations of the ensemble: %3" )
                               .arg( gridName )
                               .arg( realizationsNotInEnsemble.size() )
                               .arg( QString::fromStdString( RiaStdStringTools::formatRangeSelection( realizationsNotInEnsemble ) ) )
                               .toStdString() );
    }

    if ( realizations.empty() )
    {
        RiaLogging::warning( QString( "No selected realizations have the grid '%1'. No grid ensemble created." ).arg( gridName ).toStdString() );
        return;
    }

    const bool dimensionsAreIdentical = gridInfo.hasIdenticalDimensions( realizations );

    // Log the dimensions the endpoint reported, so a grid ensemble that does not share its grid can be
    // told apart from an endpoint reporting the wrong dimensions, before anything is downloaded.
    logRealizationDimensions( gridName, gridInfo, realizations, dimensionsAreIdentical );

    auto* gridEnsemble = new RimReservoirGridEnsembleSumo;
    gridEnsemble->setName( QString( "%1 - %2" ).arg( dataSource->ensembleName(), gridName ) );
    gridEnsemble->setSumoSource( dataSource, gridName, realizations, dimensionsAreIdentical );
    gridEnsemble->setGridRealizations( gridRealizations );
    gridEnsemble->setDoComputeMobileVolumeWeightedMean( dataSource->doComputeMobileVolumeWeightedMean() );

    // Put the ensemble in the project tree before the cases are created, so RimProject::assignCaseIdToCase
    // finds them as descendants of the project and hands out unique case ids.
    project->assignIdToCaseGroup( gridEnsemble );
    analysisModels->reservoirGridEnsembles.push_back( gridEnsemble );

    gridEnsemble->createGridCasesFromSumoSource();
    gridEnsemble->loadDataAndUpdate();

    analysisModels->updateAllRequiredEditors();

    auto allCases = gridEnsemble->cases();
    if ( allCases.empty() )
    {
        RiaLogging::warning( "No valid realizations selected. No grid ensemble created." );
        return;
    }

    if ( auto view = RicNewViewFeature::addReservoirView( allCases.front(), nullptr, gridEnsemble->viewCollection() ) )
    {
        view->nameConfig()->setAddCaseName( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoReservoirGridEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Reservoir Grid Ensemble" + RiaDefines::betaFeaturePostfix() );
    actionToSetup->setIcon( QIcon( ":/GridCaseGroup16x16.png" ) );
}
