/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewCorrelationReportPlotFeature.h"
#include "RicNewCorrelationPlotFeature.h"

#include "RimCorrelationPlotCollection.h"
#include "RimCorrelationReportPlot.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCorrelationReportPlotFeature, "RicNewCorrelationReportPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewCorrelationReportPlotFeature::isCommandEnabled() const
{
    if ( caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationPlotCollection>() ) return true;
    if ( caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationReportPlotFeature::onActionTriggered( bool isChecked )
{
    RimCorrelationPlotCollection* correlationPlotColl = caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationPlotCollection>();

    RimCorrelationReportPlot* newPlot = nullptr;
    if ( !correlationPlotColl )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<EnsemblePlotParams>() )
        {
            auto correlationPlotCollections = RimProject::current()->descendantsOfType<RimCorrelationPlotCollection>();
            CAF_ASSERT( !correlationPlotCollections.empty() );
            correlationPlotColl = correlationPlotCollections.front();

            EnsemblePlotParams   params   = userData.value<EnsemblePlotParams>();
            RimSummaryEnsemble*  ensemble = params.ensemble;
            std::vector<QString> includedQuantityNames =
                std::vector<QString>( params.includedQuantityNames.begin(), params.includedQuantityNames.end() );

            QString     mainQuantityName = params.mainQuantityName;
            std::time_t timeStep         = params.timeStep;

            newPlot = correlationPlotColl->createCorrelationReportPlot( ensemble, includedQuantityNames, mainQuantityName, timeStep );
        }
    }

    if ( !newPlot && correlationPlotColl )
    {
        newPlot = correlationPlotColl->createCorrelationReportPlot();
    }
    newPlot->loadDataAndUpdate();

    if ( correlationPlotColl ) correlationPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::onObjectAppended( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationReportPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Correlation Report Plot" );
    actionToSetup->setIcon( QIcon( ":/CorrelationReportPlot16x16.png" ) );
}
