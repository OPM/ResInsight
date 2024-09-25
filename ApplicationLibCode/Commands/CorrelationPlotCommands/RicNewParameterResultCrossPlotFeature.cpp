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

#include "RicNewParameterResultCrossPlotFeature.h"

#include "RicNewCorrelationPlotFeature.h"

#include "RimCorrelationPlotCollection.h"
#include "RimParameterResultCrossPlot.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewParameterResultCrossPlotFeature, "RicNewParameterResultCrossPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewParameterResultCrossPlotFeature::isCommandEnabled() const
{
    if ( caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationPlotCollection>() ) return true;
    if ( caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewParameterResultCrossPlotFeature::onActionTriggered( bool isChecked )
{
    RimCorrelationPlotCollection* correlationPlotColl = caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationPlotCollection>();

    RimSummaryEnsemble* ensemble = nullptr;
    QString             quantityName;
    QString             ensembleParameter;
    std::time_t         timeStep = 0;

    RimParameterResultCrossPlot* newPlot = nullptr;
    if ( !correlationPlotColl )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<EnsemblePlotParams>() )
        {
            auto correlationPlotCollections = RimProject::current()->descendantsOfType<RimCorrelationPlotCollection>();
            CAF_ASSERT( !correlationPlotCollections.empty() );
            correlationPlotColl = correlationPlotCollections.front();

            EnsemblePlotParams params = userData.value<EnsemblePlotParams>();
            ensemble                  = params.ensemble;
            quantityName              = params.mainQuantityName;
            ensembleParameter         = params.ensembleParameter;

            timeStep = params.timeStep;

            newPlot = correlationPlotColl->createParameterResultCrossPlot( ensemble, ensembleParameter, quantityName, timeStep );
        }
    }

    if ( !newPlot && correlationPlotColl )
    {
        newPlot = correlationPlotColl->createParameterResultCrossPlot();
    }

    newPlot->loadDataAndUpdate();

    if ( correlationPlotColl ) correlationPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::onObjectAppended( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewParameterResultCrossPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Parameter vs Result Cross Plot" );
    actionToSetup->setIcon( QIcon( ":/CorrelationCrossPlot16x16.png" ) );
}
