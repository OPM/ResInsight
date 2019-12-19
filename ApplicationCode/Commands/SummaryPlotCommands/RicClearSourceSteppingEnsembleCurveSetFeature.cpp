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

#include "RicClearSourceSteppingEnsembleCurveSetFeature.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicClearSourceSteppingEnsembleCurveSetFeature, "RicClearSourceSteppingEnsembleCurveSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicClearSourceSteppingEnsembleCurveSetFeature::isCommandEnabled()
{
    std::vector<caf::PdmObject*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    if ( objects.size() == 1 )
    {
        auto c = objects[0];

        RimSummaryPlot* summaryPlot = nullptr;
        c->firstAncestorOrThisOfTypeAsserted( summaryPlot );
        if ( summaryPlot )
        {
            if ( summaryPlot->ensembleCurveSetCollection()->curveSetForSourceStepping() ||
                 summaryPlot->summaryCurveCollection()->curveForSourceStepping() )
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
void RicClearSourceSteppingEnsembleCurveSetFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmObject*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    if ( objects.size() == 1 )
    {
        auto c = objects[0];

        RimSummaryPlot* summaryPlot = nullptr;
        c->firstAncestorOrThisOfType( summaryPlot );
        if ( summaryPlot )
        {
            clearAllSourceSteppingInSummaryPlot( summaryPlot );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicClearSourceSteppingEnsembleCurveSetFeature::clearAllSourceSteppingInSummaryPlot( const RimSummaryPlot* summaryPlot )
{
    RimEnsembleCurveSet* previousCurveSet = summaryPlot->ensembleCurveSetCollection()->curveSetForSourceStepping();
    summaryPlot->ensembleCurveSetCollection()->setCurveSetForSourceStepping( nullptr );

    RimSummaryCurve* previousCurve = summaryPlot->summaryCurveCollection()->curveForSourceStepping();
    summaryPlot->summaryCurveCollection()->setCurveForSourceStepping( nullptr );

    if ( previousCurveSet )
    {
        previousCurveSet->updateConnectedEditors();
    }

    if ( previousCurve )
    {
        previousCurve->updateConnectedEditors();
    }

    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicClearSourceSteppingEnsembleCurveSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Clear Source Stepping Curve Set" );
    actionToSetup->setIcon( QIcon( ":/StepUpDown16x16.png" ) );
}
