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

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RimEnsembleCurveSet.h"
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
    return caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateRegressionAnalysisCurveFeature::onActionTriggered( bool isChecked )
{
    RimSummaryRegressionAnalysisCurve* newCurve = nullptr;

    if ( auto summaryCurve = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCurve>() )
    {
        newCurve = createRegressionAnalysisCurveAndAddToPlot( summaryCurve );
    }

    if ( auto curveSet = caf::firstAncestorOfTypeFromSelectedObject<RimEnsembleCurveSet>() )
    {
        newCurve = createRegressionAnalysisCurveAndAddToPlot( curveSet );
    }

    if ( newCurve )
    {
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
    auto* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();

    auto newCurve = new RimSummaryRegressionAnalysisCurve();
    RiaSummaryTools::copyCurveDataSources( *newCurve, *sourceCurve );

    auto candidates    = RiaColorTables::summaryCurveDefaultPaletteColors();
    auto contrastColor = RiaColorTools::selectContrastColorFromCandiates( sourceCurve->color(), candidates.color3fArray() );

    newCurve->setColor( contrastColor );
    newCurve->setSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_RECT );
    newCurve->setSymbolSkipDistance( 50 );

    summaryPlot->addCurveAndUpdate( newCurve );

    RiaSummaryTools::copyCurveAxisData( *newCurve, *sourceCurve );

    newCurve->loadDataAndUpdate( true );
    newCurve->updateConnectedEditors();

    auto* summaryMultiPlot = summaryPlot->firstAncestorOrThisOfType<RimSummaryMultiPlot>();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryRegressionAnalysisCurve*
    RicCreateRegressionAnalysisCurveFeature::createRegressionAnalysisCurveAndAddToPlot( RimEnsembleCurveSet* sourceCurveSet )
{
    auto* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();

    auto newCurve = new RimSummaryRegressionAnalysisCurve();

    newCurve->setEnsembleCurveSet( sourceCurveSet );

    auto color = RiaColorTools::fromQColorTo3f( sourceCurveSet->mainEnsembleColor() );

    auto candidates    = RiaColorTables::summaryCurveDefaultPaletteColors();
    auto contrastColor = RiaColorTools::selectContrastColorFromCandiates( color, candidates.color3fArray() );

    newCurve->setColor( contrastColor );
    newCurve->setSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_RECT );
    newCurve->setSymbolSkipDistance( 50 );

    summaryPlot->addCurveAndUpdate( newCurve );

    newCurve->setAxisTypeX( sourceCurveSet->xAxisType() );
    newCurve->setTopOrBottomAxisX( sourceCurveSet->axisX() );
    newCurve->setLeftOrRightAxisY( sourceCurveSet->axisY() );

    newCurve->loadDataAndUpdate( true );
    newCurve->updateConnectedEditors();

    auto* summaryMultiPlot = summaryPlot->firstAncestorOrThisOfType<RimSummaryMultiPlot>();
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
