/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include <cstddef>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RigHistogramData
{
public:
    RigHistogramData();

    double              min;
    double              max;
    double              p10;
    double              p90;
    double              mean;
    double              sum;
    double              weightedMean;
    std::vector<size_t> histogram;

    bool isMinMaxValid() const;
    bool isHistogramVectorValid() const;

private:
    bool isValid( double parameter ) const;
};
