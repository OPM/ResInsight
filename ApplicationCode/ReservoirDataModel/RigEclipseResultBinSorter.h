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
#pragma once

#include <utility>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RigEclipseResultBinSorter
{
public:
    RigEclipseResultBinSorter(const std::vector<std::vector<double>>& allDataValues, int binCount);

    int                       binNumber(double value) const;
    std::pair<double, double> binRange(int binNumber) const;

private:
    void calculateRange();

private:
    const std::vector<std::vector<double>>& m_allDataValues;
    int                                     m_binCount;
    double                                  m_minValue;
    double                                  m_maxValue;
    double                                  m_binSize;
};

