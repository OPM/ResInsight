/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include <vector>
#include <cmath>
#include <cstddef>

class RigStatisticsMath
{
public:
    static void calculateBasicStatistics(const std::vector<double>& values, double* min, double* max, double* range, double* mean, double* dev);
    static std::vector<double> calculateNearestRankPercentiles(const std::vector<double> & inputValues, const std::vector<double>& pValPositions);
    static std::vector<double> calculateInterpolatedPercentiles(const std::vector<double> & inputValues, const std::vector<double>& pValPositions);
};

//==================================================================================================
/// Class to calculate a histogram, and histogram based p-value estimates
//==================================================================================================

class RigHistogramCalculator
{
public:
    RigHistogramCalculator(double min, double max, size_t nBins, std::vector<size_t>* histogram);

    void addData(const std::vector<double>& data);

    /// Calculates the estimated percentile from the histogram. 
    /// the percentile is the domain value at which pVal of the observations are below it.
    /// Will only consider observed values between min and max, as all other values are discarded from the histogram

    double calculatePercentil(double pVal);

private:
    size_t maxIndex;
    double m_range;
    double m_min;
    size_t m_observationCount;
    std::vector<size_t>* m_histogram;
};
