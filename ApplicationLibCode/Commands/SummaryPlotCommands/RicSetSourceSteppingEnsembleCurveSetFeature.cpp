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

#include "RicSetSourceSteppingEnsembleCurveSetFeature.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include "RicClearSourceSteppingEnsembleCurveSetFeature.h"
#include "RimSummaryPlot.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicSetSourceSteppingEnsembleCurveSetFeature, "RicSetSourceSteppingEnsembleCurveSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSetSourceSteppingEnsembleCurveSetFeature::isCommandEnabled() const
{
    const auto ensembleCurveSets = caf::SelectionManager::instance()->objectsByType<RimEnsembleCurveSet>();
    if ( ensembleCurveSets.size() == 1 )
    {
        auto c = ensembleCurveSets[0];

        auto coll = c->firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>();
        if ( coll )
        {
            if ( coll->curveSetForSourceStepping() != c )
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
void RicSetSourceSteppingEnsembleCurveSetFeature::onActionTriggered( bool isChecked )
{
    const auto objects = caf::SelectionManager::instance()->objectsByType<RimEnsembleCurveSet>();
    if ( objects.size() == 1 )
    {
        auto c = objects[0];

        auto summaryPlot = c->firstAncestorOrThisOfType<RimSummaryPlot>();
        if ( summaryPlot )
        {
            RicClearSourceSteppingEnsembleCurveSetFeature::clearAllSourceSteppingInSummaryPlot( summaryPlot );
        }

        auto coll = c->firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>();
        if ( coll )
        {
            coll->setCurveSetForSourceStepping( c );
            c->updateConnectedEditors();

            RiuPlotMainWindowTools::refreshToolbars();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSetSourceSteppingEnsembleCurveSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Set as Source Stepping Curve Set" );
    actionToSetup->setIcon( QIcon( ":/StepUpDown16x16.png" ) );
}
