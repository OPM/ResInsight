/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiuPlotCurve.h"

#include "RiaCurveDataTools.h"
#include "RiaImageTools.h"
#include "RiaTimeTTools.h"

#include "RimPlotCurve.h"

#include "RiuQwtSymbol.h"

#include "qwt_date.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurve::RiuPlotCurve()
{
    m_symbolSkipPixelDistance = 10.0f;

    m_blackAndWhiteLegendIcon = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurve::~RiuPlotCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setSamplesValues( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    setSamplesInPlot( xValues, yValues, static_cast<int>( xValues.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setSamplesFromXValuesAndYValues( const std::vector<double>& xValues,
                                                    const std::vector<double>& yValues,
                                                    bool                       keepOnlyPositiveValues )
{
    computeValidIntervalsAndSetCurveData( xValues, yValues, keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                                  const std::vector<double>&    yValues,
                                                  bool                          keepOnlyPositiveValues )
{
    auto xValues = RiuPlotCurve::fromQDateTime( dateTimes );

    computeValidIntervalsAndSetCurveData( xValues, yValues, keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                                  const std::vector<double>& yValues,
                                                  bool                       keepOnlyPositiveValues )
{
    auto xValues = RiuPlotCurve::fromTime_t( dateTimes );

    computeValidIntervalsAndSetCurveData( xValues, yValues, keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setLineSegmentStartStopIndices( const std::vector<std::pair<size_t, size_t>>& lineSegmentStartStopIndices )
{
    m_polyLineStartStopIndices = lineSegmentStartStopIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setSymbolSkipPixelDistance( float distance )
{
    m_symbolSkipPixelDistance = distance >= 0.0f ? distance : 0.0f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setPerPointLabels( const std::vector<QString>& labels )
{
    m_perPointLabels = labels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setBlackAndWhiteLegendIcon( bool blackAndWhite )
{
    m_blackAndWhiteLegendIcon = blackAndWhite;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::computeValidIntervalsAndSetCurveData( const std::vector<double>& xValues,
                                                         const std::vector<double>& yValues,
                                                         bool                       keepOnlyPositiveValues )
{
    auto intervalsOfValidValues = RiaCurveDataTools::calculateIntervalsOfValidValues( yValues, keepOnlyPositiveValues );

    std::vector<double> validYValues;
    std::vector<double> validXValues;

    RiaCurveDataTools::getValuesByIntervals( yValues, intervalsOfValidValues, &validYValues );
    RiaCurveDataTools::getValuesByIntervals( xValues, intervalsOfValidValues, &validXValues );

    setSamplesInPlot( validXValues, validYValues, static_cast<int>( validXValues.size() ) );

    setLineSegmentStartStopIndices( RiaCurveDataTools::computePolyLineStartStopIndices( intervalsOfValidValues ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiuPlotCurve::fromQDateTime( const std::vector<QDateTime>& dateTimes )
{
    std::vector<double> doubleValues;

    if ( !dateTimes.empty() )
    {
        doubleValues.reserve( dateTimes.size() );

        for ( const auto& dt : dateTimes )
        {
            // TODO: remove Qwt usage here..
            doubleValues.push_back( QwtDate::toDouble( dt ) );
        }
    }

    return doubleValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiuPlotCurve::fromTime_t( const std::vector<time_t>& timeSteps )
{
    std::vector<double> doubleValues;

    if ( !timeSteps.empty() )
    {
        doubleValues.reserve( timeSteps.size() );
        for ( const auto& time : timeSteps )
        {
            doubleValues.push_back( RiaTimeTTools::toDouble( time ) );
        }
    }

    return doubleValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotCurve::setSamplesFromXYErrorValues( const std::vector<double>&   xValues,
                                                const std::vector<double>&   yValues,
                                                const std::vector<double>&   errorValues,
                                                bool                         keepOnlyPositiveValues,
                                                RiaCurveDataTools::ErrorAxis errorAxis )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurve* RiuPlotCurve::ownerRimCurve()
{
    return m_ownerRimCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimPlotCurve* RiuPlotCurve::ownerRimCurve() const
{
    return m_ownerRimCurve;
}
