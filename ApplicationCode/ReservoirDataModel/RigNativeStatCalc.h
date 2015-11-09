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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfCollection.h"

class RigHistogramCalculator;
class RigCaseCellResultsData;

//==================================================================================================
/// 
//==================================================================================================
class RigNativeStatCalc : public RigStatisticsCalculator
{
public:
    RigNativeStatCalc(RigCaseCellResultsData* cellResultsData, size_t scalarResultIndex);

    virtual void minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max);
    virtual void posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg);
    virtual void valueSumAndSampleCount(double& valueSum, size_t& sampleCount);

    virtual void addDataToHistogramCalculator(RigHistogramCalculator& histogramCalculator);
    virtual size_t  timeStepCount();

private:
    RigCaseCellResultsData* m_resultsData;
    size_t m_scalarResultIndex;
};
