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

#include "RiaSummaryTools.h"

#include "RimSummaryCurve.h"
#include "RimSummaryDeclineCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateDeclineCurvesFeature, "RicCreateDeclineCurvesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateDeclineCurvesFeature::isCommandEnabled()
{
    RimSummaryPlot* selectedPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot*>();
    return ( selectedPlot && !RiaSummaryTools::isSummaryCrossPlot( selectedPlot ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDeclineCurvesFeature::onActionTriggered( bool isChecked )
{
    RimSummaryCurve* curve = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCurve*>();
    if ( curve )
    {
        std::vector<RimSummaryDeclineCurve::DeclineCurveType> declineCurveTypes = { RimSummaryDeclineCurve::DeclineCurveType::EXPONENTIAL,
                                                                                    RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC,
                                                                                    RimSummaryDeclineCurve::DeclineCurveType::HARMONIC };

        for ( auto declineCurveType : declineCurveTypes )
        {
            RimSummaryDeclineCurve* newCurve = createDeclineCurveAndAddToPlot( curve, declineCurveType );

            RiuPlotMainWindowTools::showPlotMainWindow();
            RiuPlotMainWindowTools::selectAsCurrentItem( newCurve );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDeclineCurvesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Decline Curves" );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDeclineCurve* RicCreateDeclineCurvesFeature::createDeclineCurveAndAddToPlot( RimSummaryCurve* sourceCurve,
                                                                                       RimSummaryDeclineCurve::DeclineCurveType declineCurveType )
{
    auto mapToLineStyle = []( RimSummaryDeclineCurve::DeclineCurveType t )
    {
        if ( t == RimSummaryDeclineCurve::DeclineCurveType::HARMONIC ) return RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DOT;
        if ( t == RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC ) return RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH;
        return RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH_DOT;
    };

    RimSummaryPlot* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot*>();

    RimSummaryDeclineCurve* newCurve = new RimSummaryDeclineCurve();
    CVF_ASSERT( newCurve );

    newCurve->setSummaryCaseX( sourceCurve->summaryCaseX() );
    newCurve->setSummaryAddressX( sourceCurve->summaryAddressX() );

    newCurve->setSummaryCaseY( sourceCurve->summaryCaseY() );
    newCurve->setSummaryAddressY( sourceCurve->summaryAddressY() );

    newCurve->setDeclineCurveType( declineCurveType );

    newCurve->setColor( sourceCurve->color() );
    newCurve->setLineStyle( mapToLineStyle( declineCurveType ) );

    summaryPlot->addCurveAndUpdate( newCurve );

    newCurve->loadDataAndUpdate( true );
    newCurve->updateConnectedEditors();

    RimSummaryMultiPlot* summaryMultiPlot = nullptr;
    summaryPlot->firstAncestorOrThisOfType( summaryMultiPlot );
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
