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

#include "RiaSummaryTools.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "RimSummaryCaseMainCollection.h"
#include "Sumo/RimSummaryEnsembleSumo.h"
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
    for ( auto dataSource : dataSources )
    {
        RimSummaryEnsembleSumo* ensemble = new RimSummaryEnsembleSumo();
        ensemble->setSumoDataSource( dataSource );
        ensemble->updateName();
        RiaSummaryTools::summaryCaseMainCollection()->addEnsemble( ensemble );
        ensemble->loadDataAndUpdate();

        RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( {}, { ensemble } );
    }

    RiaSummaryTools::summaryCaseMainCollection()->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSumoEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Sumo Ensemble" );
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble.svg" ) );
}
