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

#include "RicCreateRftCorrelationReportFeature.h"

#include "RimCorrelationPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimRftCorrelationReportPlot.h"
#include "RimWellPlotTools.h"
#include "RimWellRftPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateRftCorrelationReportFeature, "RicCreateRftCorrelationReportFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateRftCorrelationReportFeature::isCommandEnabled() const
{
    if ( dynamic_cast<RimWellRftPlot*>( caf::SelectionManager::instance()->selectedItem() ) ) return true;

    if ( dynamic_cast<RimCorrelationPlotCollection*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        return !RimWellPlotTools::firstWellNameWithEnsembleRftData().isEmpty();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftCorrelationReportFeature::onActionTriggered( bool /*isChecked*/ )
{
    // Without a source plot, a default report is created for the first available ensemble RFT data
    auto* sourcePlot = dynamic_cast<RimWellRftPlot*>( caf::SelectionManager::instance()->selectedItem() );

    auto* correlationColl = RimMainPlotCollection::current()->correlationPlotCollection();
    if ( !correlationColl ) return;

    auto* report = correlationColl->createRftCorrelationReportPlot( sourcePlot );
    if ( !report ) return;

    report->loadDataAndUpdate();
    correlationColl->updateConnectedEditors();

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::onObjectAppended( report );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRftCorrelationReportFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create RFT Correlation Report" );
    actionToSetup->setIcon( QIcon( ":/CorrelationReportPlot16x16.png" ) );
}
