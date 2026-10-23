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

#include "RicCreateDeclineCurvesFeature.h"

#include "RiaColorTables.h"
#include "Summary/RiaSummaryTools.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryDeclineCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cafAssert.h"

#include <QAction>

#include <algorithm>

CAF_CMD_SOURCE_INIT( RicCreateDeclineCurvesFeature, "RicCreateDeclineCurvesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateDeclineCurvesFeature::isCommandEnabled() const
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDeclineCurvesFeature::onActionTriggered( bool isChecked )
{
    RimSummaryCurve* curve = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCurve>();
    if ( curve )
    {
        std::vector<RimSummaryDeclineCurve::DeclineCurveType> declineCurveTypes = { RimSummaryDeclineCurve::DeclineCurveType::EXPONENTIAL,
                                                                                    RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC,
                                                                                    RimSummaryDeclineCurve::DeclineCurveType::HARMONIC };

        if ( auto plot = curve->firstAncestorOrThisOfType<RimSummaryPlot>() )
        {
            // Initialize the equation box position from the first decline curves: cumulative upper left, rate upper right
            if ( !plot->hasDeclineCurves() )
            {
                const bool isCumulative = curve->curveType() == RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED;
                plot->setEquationBoxPosition( isCumulative ? RimSummaryPlot::EquationBoxPosition::UPPER_LEFT
                                                           : RimSummaryPlot::EquationBoxPosition::UPPER_RIGHT );
            }
        }

        for ( auto declineCurveType : declineCurveTypes )
        {
            RimSummaryDeclineCurve* newCurve = createDeclineCurveAndAddToPlot( curve, declineCurveType );

            RiuPlotMainWindowTools::showPlotMainWindow();
            RiuPlotMainWindowTools::selectAsCurrentItem( newCurve );
        }

        // Re-attach all curves in project tree order to get a consistent legend order
        if ( auto curveCollection = curve->firstAncestorOrThisOfType<RimSummaryCurveCollection>() )
        {
            curveCollection->updateCurveOrder();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDeclineCurvesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Decline Curves" );
    actionToSetup->setIcon( QIcon( ":/decline-curve.svg" ) );
}

//--------------------------------------------------------------------------------------------------
/// Pick the next color in the default summary palette that is not used by any curve in the plot
//--------------------------------------------------------------------------------------------------
cvf::Color3f RicCreateDeclineCurvesFeature::createDistinctColor( RimSummaryPlot* summaryPlot, RimSummaryCurve* sourceCurve )
{
    const auto& palette = RiaColorTables::summaryCurveDefaultPaletteColors();

    std::vector<cvf::Color3f> usedColors = { sourceCurve->color() };
    for ( auto curve : summaryPlot->summaryCurves() )
    {
        usedColors.push_back( curve->color() );
    }

    const size_t startIndex = summaryPlot->curveCount();
    for ( size_t i = 0; i < palette.size(); i++ )
    {
        cvf::Color3f candidate = palette.cycledColor3f( startIndex + i );
        if ( std::find( usedColors.begin(), usedColors.end(), candidate ) == usedColors.end() ) return candidate;
    }

    return palette.cycledColor3f( startIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDeclineCurve* RicCreateDeclineCurvesFeature::createDeclineCurveAndAddToPlot( RimSummaryCurve* sourceCurve,
                                                                                       RimSummaryDeclineCurve::DeclineCurveType declineCurveType )
{
    RimSummaryPlot* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();

    RimSummaryDeclineCurve* newCurve = new RimSummaryDeclineCurve();
    CAF_ASSERT( newCurve );

    RiaSummaryTools::copyCurveDataSources( *newCurve, *sourceCurve );

    newCurve->setDeclineCurveType( declineCurveType );

    newCurve->setColor( createDistinctColor( summaryPlot, sourceCurve ) );
    newCurve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );

    summaryPlot->addCurveAndUpdate( newCurve );

    RiaSummaryTools::copyCurveAxisData( *newCurve, *sourceCurve );

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
    summaryPlot->zoomAll();

    return newCurve;
}
