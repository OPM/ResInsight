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

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<class T>
RiaWeightedAverageCalculator<T>::RiaWeightedAverageCalculator()
    : m_aggregatedValue(T{})
    , m_aggregatedWeight(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<class T>
void RiaWeightedAverageCalculator<T>::addValueAndWeight(T value, double weight)
{
    CVF_ASSERT(weight >= 0.0);

    m_aggregatedValue = m_aggregatedValue + value * weight;
    m_aggregatedWeight += weight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<class T>
T RiaWeightedAverageCalculator<T>::weightedAverage() const
{
    bool validWeights = validAggregatedWeight();
    CVF_TIGHT_ASSERT(validWeights);
    if (validWeights)
    {
        return m_aggregatedValue * (1.0 / m_aggregatedWeight);
    }
    return T{};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<class T>
double RiaWeightedAverageCalculator<T>::aggregatedWeight() const
{
    return m_aggregatedWeight;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<class T>
bool RiaWeightedAverageCalculator<T>::validAggregatedWeight() const
{
    return m_aggregatedWeight > 1.0e-12;
}