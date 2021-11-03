/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimPlotAxisLogRangeCalculator.h"

#include "RiaDefines.h"

#include "cvfVector2.h"

#include <cmath>

#include <qwt_plot_curve.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisLogRangeCalculator::RimPlotAxisLogRangeCalculator( QwtPlot::Axis                           axis,
                                                              const std::vector<const QwtPlotCurve*>& qwtCurves )
    : m_axis( axis )
    , m_curves( qwtCurves )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisLogRangeCalculator::computeAxisRange( double* minPositive, double* max ) const
{
    double minPosValue = HUGE_VAL;
    double maxValue    = -HUGE_VAL;

    for ( const QwtPlotCurve* curve : m_curves )
    {
        double minPosCurveValue = HUGE_VAL;
        double maxCurveValue    = -HUGE_VAL;

        if ( curveValueRange( curve, &minPosCurveValue, &maxCurveValue ) )
        {
            if ( minPosCurveValue < minPosValue )
            {
                CVF_ASSERT( minPosCurveValue > 0.0 );
                minPosValue = minPosCurveValue;
            }

            if ( maxCurveValue > maxValue )
            {
                maxValue = maxCurveValue;
            }
        }
    }

    if ( minPosValue == HUGE_VAL )
    {
        minPosValue = RiaDefines::minimumDefaultLogValuePlot();
        maxValue    = RiaDefines::maximumDefaultValuePlot();
    }

    *minPositive = minPosValue;
    *max         = maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisLogRangeCalculator::curveValueRange( const QwtPlotCurve* qwtCurve, double* minPositive, double* max ) const
{
    if ( !qwtCurve ) return false;

    if ( qwtCurve->data()->size() < 1 )
    {
        return false;
    }

    float minPosF = std::numeric_limits<float>::infinity();
    float maxF    = -std::numeric_limits<float>::infinity();

    int axisValueIndex = 0;
    if ( m_axis == QwtPlot::yLeft || m_axis == QwtPlot::yRight )
    {
        axisValueIndex = 1;
    }

    for ( size_t i = 0; i < qwtCurve->dataSize(); ++i )
    {
        QPointF    sample = qwtCurve->sample( (int)i );
        cvf::Vec2f vec( sample.x(), sample.y() );
        float      value = vec[axisValueIndex];
        if ( value == HUGE_VALF ) continue;

        maxF = std::max( maxF, value );
        if ( value > 0.0f && value < minPosF )
        {
            minPosF = value;
        }
    }

    *minPositive = minPosF;
    *max         = maxF;

    return true;
}
