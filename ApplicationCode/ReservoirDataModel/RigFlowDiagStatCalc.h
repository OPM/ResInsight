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

#include "RigStatisticsCalculator.h"
#include "RigFlowDiagResultAddress.h"

class RigHistogramCalculator;
class RigFlowDiagResults;

//==================================================================================================
/// 
//==================================================================================================

class RigFlowDiagStatCalc : public RigStatisticsCalculator
{
public:
    RigFlowDiagStatCalc(RigFlowDiagResults* femResultCollection, const RigFlowDiagResultAddress& resVarAddr);

    virtual void    minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max);
    virtual void    posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg);
    virtual void    valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount);
    virtual void    addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator);
    virtual void    uniqueValues(size_t timeStepIndex, std::set<int>& values);
    virtual size_t  timeStepCount();

private:
    RigFlowDiagResults*      m_resultsData;
    RigFlowDiagResultAddress m_resVarAddr;
};



