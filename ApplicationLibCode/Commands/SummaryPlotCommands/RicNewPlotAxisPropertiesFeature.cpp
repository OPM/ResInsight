/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RicNewPlotAxisPropertiesFeature.h"

#include "RiaPlotDefines.h"

#include "RimPlotAxisProperties.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPlotAxisPropertiesFeature, "RicNewPlotAxisPropertiesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPlotAxisPropertiesFeature::isCommandEnabled()
{
    auto* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot*>();
    return ( summaryPlot != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPlotAxisPropertiesFeature::onActionTriggered( bool isChecked )
{
    auto* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot*>();
    if ( !summaryPlot ) return;

    RimPlotAxisProperties* newPlotAxisProperties =
        summaryPlot->addNewAxisProperties( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, "New Axis" );
    summaryPlot->plotWidget()->ensureAxisIsCreated( newPlotAxisProperties->plotAxisType() );
    newPlotAxisProperties->setNameForUnusedAxis();

    summaryPlot->updateConnectedEditors();
    RiuPlotMainWindowTools::selectAsCurrentItem( newPlotAxisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPlotAxisPropertiesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Plot Axis" );
    actionToSetup->setIcon( QIcon( ":/LeftAxis16x16.png" ) );
}
