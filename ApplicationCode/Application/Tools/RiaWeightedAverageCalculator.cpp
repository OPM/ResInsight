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

#include "RiaWeightedAverageCalculator.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaWeightedAverageCalculator::RiaWeightedAverageCalculator()
    : m_aggregatedValue(0.0)
    , m_aggregatedWeight(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaWeightedAverageCalculator::addValueAndWeight(double value, double weight)
{
    CVF_ASSERT(weight >= 0.0);

    m_aggregatedValue += value * weight;
    m_aggregatedWeight += weight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWeightedAverageCalculator::weightedAverage() const
{
    if (m_aggregatedWeight > 1e-7)
    {
        return m_aggregatedValue / m_aggregatedWeight;
    }

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWeightedAverageCalculator::aggregatedWeight() const
{
    return m_aggregatedWeight;
}
