/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicShowCumulativePhasePlotFeature.h"

#include "RimFlowPlotCollection.h"
#include "RimWellAllocationPlot.h"
#include "RimWellDistributionPlotCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowCumulativePhasePlotFeature, "RicShowCumulativePhasePlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowCumulativePhasePlotFeature::isCommandEnabled()
{
    RimWellAllocationPlot* plot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellAllocationPlot>();

    return plot != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowCumulativePhasePlotFeature::onActionTriggered( bool isChecked )
{
    RimWellAllocationPlot* plot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellAllocationPlot>();

    if ( !plot ) return;

    RimFlowPlotCollection* flowPlotColl = nullptr;
    plot->firstAncestorOrThisOfType( flowPlotColl );

    if ( !flowPlotColl ) return;

    RimWellDistributionPlotCollection* wdp = flowPlotColl->wellDistributionPlotCollection();
    wdp->setData( plot->rimCase(), plot->wellName(), plot->timeStep() );
    wdp->setShowWindow( true );
    wdp->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowCumulativePhasePlotFeature::setupActionLook( QAction* actionToSetup )
{
    // actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText( "Show Cumulative Phase Distribution Plot" );
}
