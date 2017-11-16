/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfCollection.h"
#include "cvfObject.h"

#include <vector>

class RigHistogramCalculator;

//==================================================================================================
/// 
//==================================================================================================
class RigStatisticsCalculator : public cvf::Object
{
public:
    virtual void    minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max) = 0;
    virtual void    posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg) = 0;
    
    void            meanCellScalarValue(double& meanValue);
    void            meanCellScalarValue(size_t timeStepIndex, double& meanValue);

    void            valueSumAndSampleCount(double& valueSum, size_t& sampleCount);
    virtual void    valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount) = 0;

    void            addDataToHistogramCalculator(RigHistogramCalculator& histogramCalculator);
    virtual void    addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator) = 0;

    virtual void    uniqueValues(size_t timeStepIndex, std::set<int>& values) = 0;

    virtual size_t  timeStepCount() = 0;

    virtual void    mobileVolumeWeightedMean(size_t timeStepIndex, double& mean);

    static void     posNegClosestToZero(const std::vector<double>& values, double& pos, double& neg);
};
