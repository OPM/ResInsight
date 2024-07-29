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

#include "RicNewCorrelationPlotFeature.h"

#include "RimCorrelationPlot.h"
#include "RimCorrelationPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCorrelationPlotFeature, "RicNewCorrelationPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewCorrelationPlotFeature::isCommandEnabled() const
{
    if ( caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationPlotCollection>() ) return true;
    if ( caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationPlotFeature::onActionTriggered( bool isChecked )
{
    RimCorrelationPlotCollection* correlationPlotColl = caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationPlotCollection>();

    RimCorrelationPlot* newPlot = nullptr;
    if ( !correlationPlotColl )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<EnsemblePlotParams>() )
        {
            auto correlationPlotCollections = RimProject::current()->descendantsOfType<RimCorrelationPlotCollection>();
            CAF_ASSERT( !correlationPlotCollections.empty() );
            correlationPlotColl = correlationPlotCollections.front();

            EnsemblePlotParams  params       = userData.value<EnsemblePlotParams>();
            RimSummaryEnsemble* ensemble     = params.ensemble;
            QString             quantityName = params.mainQuantityName;
            std::time_t         timeStep     = params.timeStep;

            newPlot = correlationPlotColl->createCorrelationPlot( ensemble, quantityName, timeStep );
        }
    }

    if ( !newPlot && correlationPlotColl )
    {
        newPlot = correlationPlotColl->createCorrelationPlot();
    }

    newPlot->loadDataAndUpdate();

    if ( correlationPlotColl ) correlationPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::onObjectAppended( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Correlation Tornado Plot" );
    actionToSetup->setIcon( QIcon( ":/CorrelationTornadoPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsemblePlotParams::EnsemblePlotParams()
    : ensemble( nullptr )
    , mainQuantityName( "" )
    , ensembleParameter( "" )
    , timeStep( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsemblePlotParams::EnsemblePlotParams( RimSummaryEnsemble* ensemble,
                                        const QStringList&  includedQuantityNames,
                                        const QString&      mainQuantityName,
                                        const std::time_t&  timeStep )
    : ensemble( ensemble )
    , includedQuantityNames( includedQuantityNames )
    , mainQuantityName( mainQuantityName )
    , ensembleParameter( "" )
    , timeStep( timeStep )
{
}
