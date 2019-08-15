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

#include "RiaStatisticsTools.h"

#include <vector>
#include <set>
#include <cmath>
#include <cstddef>

class RigStatisticsMath
{
public:
    static void calculateBasicStatistics(const std::vector<double>& values, double* min, double* max, double* sum, double* range, double* mean, double* dev);
    static void calculateStatisticsCurves(const std::vector<double>& values, double* p10, double* p50, double* p90, double* mean);

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
    void addData(const std::vector<float>& data);

    void addValue(double value);

    /// Calculates the estimated percentile from the histogram. 
    /// the percentile is the domain value at which pVal of the observations are below it.
    /// Will only consider observed values between min and max, as all other values are discarded from the histogram

    double calculatePercentil(double pVal);

private:
    size_t m_maxIndex;
    double m_range;
    double m_min;
    size_t m_observationCount;
    std::vector<size_t>* m_histogram;
};


class MinMaxAccumulator
{
public:
    MinMaxAccumulator(double initMin = HUGE_VAL, double initMax = -HUGE_VAL): max(initMax), min(initMin) {}
    
    void addData(const std::vector<double>& values)
    {
        for ( double val : values )
        {
            addValue(val);
        }
    }

    void addData(const std::vector<float>& values)
    {
        for ( float val : values )
        {
            addValue(val);
        }
    }

    void addValue(double value)
    {
        if (RiaStatisticsTools::isValidNumber<double>(value))
        {
            if (value < min)
            {
                min = value;
            }

            if (value > max)
            {
                max = value;
            }
        }
    }

    double max;
    double min;
};


class PosNegAccumulator
{
public:
    PosNegAccumulator(double initPos = HUGE_VAL, double initNeg = -HUGE_VAL): pos(initPos), neg(initNeg) {}

    void addData(const std::vector<double>& values) 
    {
        for (double val : values)
        {
            addValue(val);
        }
    }

    void addData(const std::vector<float>& values)
    {
        for ( float val : values )
        {
            addValue(val);
        }
    }

    void addValue(double value)
    {
        if (RiaStatisticsTools::isValidNumber<double>(value))
        {
            if (value < pos && value > 0)
            {
                pos = value;
            }

            if (value > neg && value < 0)
            {
                neg = value;
            }
        }
    }

    double pos;
    double neg;
};


class SumCountAccumulator
{
public:
    SumCountAccumulator(double initSum = 0.0, size_t initCount = 0): valueSum(initSum), sampleCount(initCount) {}

    void addData(const std::vector<double>& values)
    {
        for ( double val : values )
        {
            addValue(val);
        }
    }

    void addData(const std::vector<float>& values)
    {
        for ( float val : values )
        {
            addValue(val);
        }
    }

    void addValue(double value)
    {
        if (RiaStatisticsTools::isValidNumber<double>(value))
        {
            valueSum += value;
            ++sampleCount;
        }
    }

    double valueSum;
    size_t sampleCount;
};


class UniqueValueAccumulator
{
public:
    UniqueValueAccumulator()
    {}

    void addData(const std::vector<double>& values)
    {
        for ( double val : values )
        {
            addValue(val);
        }
    }

    void addData(const std::vector<float>& values)
    {
        for ( float val : values )
        {
            addValue(val);
        }
    }

    void addValue(double value)
    {
        if (RiaStatisticsTools::isValidNumber<double>(value))
        {
            uniqueValues.insert(static_cast<int>(value));
        }
    }

    std::set<int> uniqueValues;
};
