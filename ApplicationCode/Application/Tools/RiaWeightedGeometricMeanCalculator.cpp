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

#include "RiaWeightedGeometricMeanCalculator.h"

#include "cvfAssert.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaWeightedGeometricMeanCalculator::RiaWeightedGeometricMeanCalculator()
    : m_aggregatedWeightedValue(0.0)
    , m_aggregatedWeight(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaWeightedGeometricMeanCalculator::addValueAndWeight(double value, double weight)
{
    CVF_ASSERT(weight >= 0.0);

    // This can be a very big number, consider other algorithms if that becomes a problem
    m_aggregatedWeightedValue += (std::log(value) * weight);
    m_aggregatedWeight += weight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWeightedGeometricMeanCalculator::weightedMean() const
{
    if (m_aggregatedWeight > 1e-7)
    {
        return std::exp(m_aggregatedWeightedValue / m_aggregatedWeight);
    }

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWeightedGeometricMeanCalculator::aggregatedWeight() const
{
    return m_aggregatedWeight;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaWeightedGeometricMeanCalculator::validAggregatedWeight() const
{
    return m_aggregatedWeight > 1.0e-12;
}
