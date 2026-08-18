/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicCreateSumoGridEnsembleFeature.h"

#include "RiaDefines.h"
#include "RiaLogging.h"

#include "RicNewViewFeature.h"

#include "Rim3dView.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseViewCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRoffCaseSumo.h"
#include "RimViewNameConfig.h"
#include "Sumo/RimSumoDataSource.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSumoGridEnsembleFeature, "RicCreateSumoGridEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoGridEnsembleFeature::onActionTriggered( bool isChecked )
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
void RicCreateSumoGridEnsembleFeature::createGridEnsemble( RimSumoDataSource* dataSource )
{
    if ( !dataSource ) return;

    const QString              gridName       = dataSource->selectedGridName();
    const std::vector<QString> realizationIds = dataSource->selectedRealizationIds();

    if ( gridName.isEmpty() )
    {
        RiaLogging::warning( "No grid selected. Unable to create grid ensemble from Sumo." );
        return;
    }

    if ( realizationIds.empty() )
    {
        RiaLogging::warning( "No realizations selected. Unable to create grid ensemble from Sumo." );
        return;
    }

    RimProject* project = RimProject::current();
    if ( !project ) return;

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return;

    auto eclipseCaseEnsemble = new RimEclipseCaseEnsemble;
    eclipseCaseEnsemble->setName( QString( "%1 - %2" ).arg( dataSource->ensembleName(), gridName ) );
    eclipseCaseEnsemble->setDoComputeMobileVolumeWeightedMean( dataSource->doComputeMobileVolumeWeightedMean() );

    for ( const QString& realizationId : realizationIds )
    {
        bool ok          = false;
        int  realization = realizationId.toInt( &ok );
        if ( !ok ) continue;

        if ( auto* gridCase = RimRoffCaseSumo::createFromDataSource( dataSource, gridName, realization ) )
        {
            eclipseCaseEnsemble->addCase( gridCase );
        }
    }

    if ( eclipseCaseEnsemble->cases().empty() )
    {
        RiaLogging::warning( "No valid realizations selected. No grid ensemble created." );
        delete eclipseCaseEnsemble;
        return;
    }

    oilfield->analysisModels()->caseEnsembles.push_back( eclipseCaseEnsemble );
    oilfield->analysisModels()->updateConnectedEditors();

    auto firstCase = eclipseCaseEnsemble->cases().front();
    if ( !firstCase ) return;

    auto view = RicNewViewFeature::addReservoirView( firstCase, nullptr, eclipseCaseEnsemble->viewCollection() );
    if ( view )
    {
        view->nameConfig()->setAddCaseName( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoGridEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Grid Ensemble" + RiaDefines::betaFeaturePostfix() );
    actionToSetup->setIcon( QIcon( ":/CreateGridCaseGroup16x16.png" ) );
}
