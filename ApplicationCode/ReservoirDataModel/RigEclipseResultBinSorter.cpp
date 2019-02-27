/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RigEclipseResultBinSorter.h"

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultBinSorter::RigEclipseResultBinSorter(const std::vector<std::vector<double>>& allDataValues, int binCount)
    : m_allDataValues(allDataValues)
    , m_binCount(binCount)
    , m_minValue(std::numeric_limits<double>::infinity())
    , m_maxValue(-std::numeric_limits<double>::infinity())
    , m_binSize(0.0)
{
    calculateRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigEclipseResultBinSorter::binNumber(double value) const
{
    double distFromMin = value - m_minValue;
    return std::min(m_binCount - 1, static_cast<int>(distFromMin / m_binSize));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RigEclipseResultBinSorter::binRange(int binNumber) const
{
    double minBinValue = m_minValue + m_binSize * binNumber;
    double maxBinBalue = minBinValue + m_binSize;
    return std::make_pair(minBinValue, maxBinBalue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseResultBinSorter::calculateRange()
{
    for (const std::vector<double>& doubleRange : m_allDataValues)
    {
        if (!doubleRange.empty())
        {
            for (double value : doubleRange)
            {
                if (value != std::numeric_limits<double>::infinity())
                {
                    m_minValue = std::min(m_minValue, value);
                    m_maxValue = std::max(m_maxValue, value);
                }
            }
        }
    }

    if (m_maxValue > m_minValue)
    {
        m_binSize = (m_maxValue - m_minValue) / m_binCount;
    }
}
