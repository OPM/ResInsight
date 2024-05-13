/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicNewVfpPlotFeature.h"

#include "RimMainPlotCollection.h"

#include "VerticalFlowPerformance/RimVfpDataCollection.h"
#include "VerticalFlowPerformance/RimVfpPlotCollection.h"
#include "VerticalFlowPerformance/RimVfpTableData.h"

#include "cafSelectionManagerTools.h"

#include "RiuPlotMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewVfpPlotFeature, "RicNewVfpPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewVfpPlotFeature::onActionTriggered( bool isChecked )
{
    RimVfpPlotCollection* vfpPlotColl = RimMainPlotCollection::current()->vfpPlotCollection();
    if ( !vfpPlotColl ) return;

    auto selectedTableData = caf::selectedObjectsByTypeStrict<RimVfpTableData*>();
    for ( auto tableData : selectedTableData )
    {
        RimVfpPlot* firstPlot = vfpPlotColl->createAndAppendPlots( tableData );
        vfpPlotColl->updateConnectedEditors();
        RiuPlotMainWindowTools::onObjectAppended( firstPlot, firstPlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewVfpPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create VFP Plot" );
    actionToSetup->setIcon( QIcon( ":/VfpPlot.svg" ) );
}
