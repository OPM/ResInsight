/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RiaWeightedHarmonicMeanCalculator.h"

#include "cvfAssert.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaWeightedHarmonicMeanCalculator::RiaWeightedHarmonicMeanCalculator()
    : m_aggregatedWeightedValue(0.0)
    , m_aggregatedWeight(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaWeightedHarmonicMeanCalculator::addValueAndWeight(double value, double weight)
{
    CVF_ASSERT(weight > 1.0e-12 && std::abs(value) > 1.0e-12);

    m_aggregatedWeightedValue += weight / value;
    m_aggregatedWeight += weight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWeightedHarmonicMeanCalculator::weightedMean() const
{    
    if (validAggregatedWeight())
    {
        return m_aggregatedWeight / m_aggregatedWeightedValue;
    }
    CVF_ASSERT(false);
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWeightedHarmonicMeanCalculator::aggregatedWeight() const
{
    return m_aggregatedWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaWeightedHarmonicMeanCalculator::validAggregatedWeight() const
{
    return m_aggregatedWeight > 1.0e-12;
}
