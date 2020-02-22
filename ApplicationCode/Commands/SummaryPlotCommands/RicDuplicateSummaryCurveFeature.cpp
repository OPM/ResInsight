/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicDuplicateSummaryCurveFeature.h"

#include "RiaApplication.h"
#include "RiaSummaryTools.h"

#include "RicPasteSummaryCurveFeature.h"

#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDuplicateSummaryCurveFeature, "RicDuplicateSummaryCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDuplicateSummaryCurveFeature::isCommandEnabled()
{
    RimSummaryPlot* selectedPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot*>();
    return ( selectedPlot && !RiaSummaryTools::isSummaryCrossPlot( selectedPlot ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryCurveFeature::onActionTriggered( bool isChecked )
{
    RimSummaryCurve* curve = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCurve*>();
    if ( curve )
    {
        RimSummaryCurve* newCurve = RicPasteSummaryCurveFeature::copyCurveAndAddToPlot( curve );
        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::selectAsCurrentItem( newCurve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Duplicate Summary Curve" );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}
