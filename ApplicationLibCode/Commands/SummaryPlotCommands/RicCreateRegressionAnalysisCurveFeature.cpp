/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RicCreateRegressionAnalysisCurveFeature.h"

#include "RiaSummaryTools.h"

#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimSummaryRegressionAnalysisCurve.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateRegressionAnalysisCurveFeature, "RicCreateRegressionAnalysisCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateRegressionAnalysisCurveFeature::isCommandEnabled() const
{
    RimSummaryPlot* selectedPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();
    return ( selectedPlot && !RiaSummaryTools::isSummaryCrossPlot( selectedPlot ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRegressionAnalysisCurveFeature::onActionTriggered( bool isChecked )
{
    RimSummaryCurve* curve = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCurve>();
    if ( curve )
    {
        RimSummaryRegressionAnalysisCurve* newCurve = createRegressionAnalysisCurveAndAddToPlot( curve );

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::selectAsCurrentItem( newCurve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRegressionAnalysisCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Regression Analysis Curve" );
    actionToSetup->setIcon( QIcon( ":/regression-curve.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryRegressionAnalysisCurve*
    RicCreateRegressionAnalysisCurveFeature::createRegressionAnalysisCurveAndAddToPlot( RimSummaryCurve* sourceCurve )
{
    RimSummaryPlot* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();

    RimSummaryRegressionAnalysisCurve* newCurve = new RimSummaryRegressionAnalysisCurve();
    CVF_ASSERT( newCurve );

    newCurve->setSummaryCaseX( sourceCurve->summaryCaseX() );
    newCurve->setSummaryAddressX( sourceCurve->summaryAddressX() );

    newCurve->setSummaryCaseY( sourceCurve->summaryCaseY() );
    newCurve->setSummaryAddressY( sourceCurve->summaryAddressY() );

    newCurve->setColor( sourceCurve->color() );
    newCurve->setSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_RECT );
    newCurve->setSymbolSkipDistance( 50 );

    summaryPlot->addCurveAndUpdate( newCurve );

    newCurve->updateDefaultValues();
    newCurve->loadDataAndUpdate( true );
    newCurve->updateConnectedEditors();

    RimSummaryMultiPlot* summaryMultiPlot = summaryPlot->firstAncestorOrThisOfType<RimSummaryMultiPlot>();
    if ( summaryMultiPlot )
    {
        summaryMultiPlot->updatePlotTitles();
    }
    else
    {
        summaryPlot->updatePlotTitle();
    }

    summaryPlot->updateAllRequiredEditors();

    return newCurve;
}
