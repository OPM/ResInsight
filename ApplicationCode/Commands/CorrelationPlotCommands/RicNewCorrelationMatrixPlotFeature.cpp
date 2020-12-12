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

#include "RicNewCorrelationMatrixPlotFeature.h"

#include "RicNewCorrelationPlotFeature.h"

#include "RimCorrelationMatrixPlot.h"
#include "RimCorrelationPlot.h"
#include "RimCorrelationPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCorrelationMatrixPlotFeature, "RicNewCorrelationMatrixPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewCorrelationMatrixPlotFeature::isCommandEnabled()
{
    RimCorrelationPlotCollection* correlationPlotColl = nullptr;
    RimSummaryPlot*               summaryPlot         = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( correlationPlotColl );
        selObj->firstAncestorOrThisOfType( summaryPlot );
    }

    if ( correlationPlotColl ) return true;

    if ( summaryPlot ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationMatrixPlotFeature::onActionTriggered( bool isChecked )
{
    RimCorrelationPlotCollection* correlationPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( correlationPlotColl );
    }

    RimCorrelationMatrixPlot* newPlot = nullptr;
    if ( !correlationPlotColl )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<EnsemblePlotParams>() )
        {
            std::vector<RimCorrelationPlotCollection*> correlationPlotCollections;
            RimProject::current()->descendantsOfType( correlationPlotCollections );
            CAF_ASSERT( !correlationPlotCollections.empty() );
            correlationPlotColl = correlationPlotCollections.front();

            EnsemblePlotParams params = userData.value<EnsemblePlotParams>();

            std::vector<QString> includedQuantityNames =
                std::vector<QString>( params.includedQuantityNames.begin(), params.includedQuantityNames.end() );
            RimSummaryCaseCollection* ensemble = params.ensemble;
            std::time_t               timeStep = params.timeStep;

            newPlot = correlationPlotColl->createCorrelationMatrixPlot( ensemble, includedQuantityNames, timeStep );
        }
    }
    else
    {
        newPlot = correlationPlotColl->createCorrelationMatrixPlot();
    }

    newPlot->loadDataAndUpdate();

    correlationPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::setExpanded( newPlot );
    RiuPlotMainWindowTools::selectAsCurrentItem( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationMatrixPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Correlation Matrix Plot" );
    actionToSetup->setIcon( QIcon( ":/CorrelationMatrixPlot16x16.png" ) );
}
