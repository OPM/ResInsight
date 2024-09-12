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

#include "RicCreateSumoEnsembleFeature.h"

#include "RiaDefines.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "Cloud/RimCloudDataSourceCollection.h"
#include "Sumo/RimSummarySumoDataSource.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSumoEnsembleFeature, "RicCreateSumoEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoEnsembleFeature::onActionTriggered( bool isChecked )
{
    auto dataSources = caf::selectedObjectsByType<RimSummarySumoDataSource*>();

    RimCloudDataSourceCollection::createEnsemblesFromSelectedDataSources( dataSources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Ensemble Plot" + RiaDefines::betaFeaturePostfix() );
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble.svg" ) );
}
