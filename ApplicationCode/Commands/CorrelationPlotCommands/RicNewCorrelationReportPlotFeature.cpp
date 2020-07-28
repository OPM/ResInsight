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

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCorrelationReportPlotFeature, "RicNewCorrelationReportPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewCorrelationReportPlotFeature::isCommandEnabled()
{
    RimCorrelationPlotCollection* correlationPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( correlationPlotColl );
    }

    if ( correlationPlotColl ) return true;

    RimSummaryPlot* summaryPlot = nullptr;
    selObj->firstAncestorOrThisOfType( summaryPlot );
    if ( summaryPlot ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationReportPlotFeature::onActionTriggered( bool isChecked )
{
    RimCorrelationPlotCollection* correlationPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( correlationPlotColl );
    }

    RimSummaryCaseCollection* ensemble = nullptr;
    QString                   quantityName;
    std::time_t               timeStep = 0;

    RimCorrelationReportPlot* newPlot = nullptr;
    if ( !correlationPlotColl )
    {
        QVariant userData = this->userData();
        if ( !userData.isNull() && userData.canConvert<CorrelationPlotParams>() )
        {
            std::vector<RimCorrelationPlotCollection*> correlationPlotCollections;
            RimProject::current()->descendantsOfType( correlationPlotCollections );
            CAF_ASSERT( !correlationPlotCollections.empty() );
            correlationPlotColl = correlationPlotCollections.front();

            CorrelationPlotParams params = userData.value<CorrelationPlotParams>();
            ensemble                     = params.ensemble;
            quantityName                 = params.quantityName;
            timeStep                     = params.timeStep;

            newPlot = correlationPlotColl->createCorrelationReportPlot( ensemble, quantityName, timeStep );
        }
    }
    else
    {
        newPlot = correlationPlotColl->createCorrelationReportPlot();
    }
    newPlot->loadDataAndUpdate();

    correlationPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::setExpanded( newPlot );
    RiuPlotMainWindowTools::selectAsCurrentItem( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCorrelationReportPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Correlation Report Plot" );
    actionToSetup->setIcon( QIcon( ":/CorrelationReportPlot16x16.png" ) );
}
