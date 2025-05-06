/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimTimeAxisAnnotationUpdater.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryDeclineCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryRegressionAnalysisCurve.h"
#include "RimSummaryTimeAxisProperties.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTimeAxisAnnotation*> RimTimeAxisAnnotationUpdater::createTimeAnnotations( const RimSummaryPlot* summaryPlot )
{
    if ( !summaryPlot ) return {};

    std::vector<RimTimeAxisAnnotation*> timeAnnotations;

    for ( auto curve : summaryPlot->allCurves() )
    {
        if ( auto declineCurve = dynamic_cast<RimSummaryDeclineCurve*>( curve ) )
        {
            auto annotations = declineCurve->createTimeAnnotations();
            timeAnnotations.insert( timeAnnotations.end(), annotations.begin(), annotations.end() );
        }
        else if ( auto regressionCurve = dynamic_cast<RimSummaryRegressionAnalysisCurve*>( curve ) )
        {
            auto annotations = regressionCurve->createTimeAnnotations();
            timeAnnotations.insert( timeAnnotations.end(), annotations.begin(), annotations.end() );
        }
    }

    for ( auto curveSet : summaryPlot->curveSets() )
    {
        auto annotations = curveSet->createTimeAnnotations();
        timeAnnotations.insert( timeAnnotations.end(), annotations.begin(), annotations.end() );
    }

    return timeAnnotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotationUpdater::updateTimeAnnotationObjects( RimSummaryPlot* summaryPlot )
{
    if ( !summaryPlot ) return;

    if ( auto axisProps = summaryPlot->timeAxisProperties() )
    {
        axisProps->removeAllAnnotations();

        for ( auto annotation : createTimeAnnotations( summaryPlot ) )
        {
            axisProps->appendAnnotation( annotation );
        }
        summaryPlot->updateAxes();
    }
}
