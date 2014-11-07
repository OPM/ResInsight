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
#include "cvfObject.h"
#include "cvfCollection.h"

#include <vector>

class RigHistogramCalculator;
class RigCaseCellResultsData;

//==================================================================================================
/// 
//==================================================================================================
class RigStatisticsCalculator : public cvf::Object
{
public:
    virtual void	minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max) = 0;
	virtual void	posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg) = 0;
	
	void	        meanCellScalarValue(double& meanValue);
	virtual void	valueSumAndSampleCount(double& valueSum, size_t& sampleCount) = 0;
	virtual void	addDataToHistogramCalculator(RigHistogramCalculator& histogramCalculator) = 0;

    virtual size_t  timeStepCount() = 0;
};


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


//==================================================================================================
/// 
//==================================================================================================
class RigMultipleDatasetStatCalc : public RigStatisticsCalculator
{
public:
    RigMultipleDatasetStatCalc();
    void addStatisticsCalculator(RigStatisticsCalculator* statisticsCalculator);
    void addNativeStatisticsCalculator(RigCaseCellResultsData* cellResultsData, size_t scalarResultIndices);

    virtual void minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max);
    virtual void posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg);
    
    virtual void valueSumAndSampleCount(double& valueSum, size_t& sampleCount);
    virtual void addDataToHistogramCalculator(RigHistogramCalculator& histogramCalculator);

    virtual size_t  timeStepCount();

private:
    std::vector<RigStatisticsCalculator*> m_nativeStatisticsCalculators;
};
