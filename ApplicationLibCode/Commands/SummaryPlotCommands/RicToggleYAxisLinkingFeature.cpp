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

#include "RicToggleYAxisLinkingFeature.h"

#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicToggleYAxisLinkingFeature, "RicToggleYAxisLinkingFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleYAxisLinkingFeature::isCommandEnabled()
{
    auto* summaryMultiPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryMultiPlot*>();
    return ( summaryMultiPlot != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleYAxisLinkingFeature::onActionTriggered( bool isChecked )
{
    auto* summaryMultiPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryMultiPlot*>();
    if ( !summaryMultiPlot ) return;

    bool linkedState = summaryMultiPlot->isSubPlotAxesLinked();
    summaryMultiPlot->setSubPlotAxesLinked( !linkedState );

    for ( auto plot : summaryMultiPlot->summaryPlots() )
    {
        plot->axisChanged.send( plot );

        plot->updateAxes();
    }

    summaryMultiPlot->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleYAxisLinkingFeature::setupActionLook( QAction* actionToSetup )
{
    QString text             = "Link";
    auto*   summaryMultiPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryMultiPlot*>();
    if ( summaryMultiPlot )
    {
        if ( summaryMultiPlot->isSubPlotAxesLinked() )
        {
            text = "Unlink";
        }
    }

    actionToSetup->setText( text + " Y Axis" );
    actionToSetup->setIcon( QIcon( ":/chain.png" ) );
}
