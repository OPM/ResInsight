/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicSetSourceSteppingSummaryCurveFeature.h"
#include "RicClearSourceSteppingEnsembleCurveSetFeature.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicSetSourceSteppingSummaryCurveFeature, "RicSetSourceSteppingSummaryCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSetSourceSteppingSummaryCurveFeature::isCommandEnabled() const
{
    const auto summaryCurves = caf::SelectionManager::instance()->objectsByType<RimSummaryCurve>();

    if ( summaryCurves.size() == 1 )
    {
        auto c = summaryCurves[0];

        auto coll = c->firstAncestorOrThisOfTypeAsserted<RimSummaryCurveCollection>();
        if ( coll )
        {
            if ( coll->curveForSourceStepping() != c )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSetSourceSteppingSummaryCurveFeature::onActionTriggered( bool isChecked )
{
    const auto summaryCurves = caf::SelectionManager::instance()->objectsByType<RimSummaryCurve>();
    if ( summaryCurves.size() == 1 )
    {
        auto c = summaryCurves[0];

        auto summaryPlot = c->firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
        if ( summaryPlot )
        {
            RicClearSourceSteppingEnsembleCurveSetFeature::clearAllSourceSteppingInSummaryPlot( summaryPlot );
        }

        auto coll = c->firstAncestorOrThisOfTypeAsserted<RimSummaryCurveCollection>();
        if ( coll )
        {
            coll->setCurveForSourceStepping( c );
            c->updateConnectedEditors();

            RiuPlotMainWindowTools::refreshToolbars();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSetSourceSteppingSummaryCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Set as Source Stepping Curve" );
    actionToSetup->setIcon( QIcon( ":/StepUpDown16x16.png" ) );
}
