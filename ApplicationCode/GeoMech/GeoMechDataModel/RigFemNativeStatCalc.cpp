/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemNativeStatCalc.h"
#include "RigFemScalarResultFrames.h"

#include <math.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemNativeStatCalc::RigFemNativeStatCalc(RigFemScalarResultFrames* cellResultsData)
{
    m_resultsData = cellResultsData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
  	std::vector<float>& values = m_resultsData->frameData(timeStepIndex);

	size_t i;
	for (i = 0; i < values.size(); i++)
	{
		if (values[i] == HUGE_VAL) // TODO
		{
			continue;
		}

		if (values[i] < min)
		{
			min = values[i];
		}

		if (values[i] > max)
		{
			max = values[i];
		}
	}  
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::valueSumAndSampleCount(double& valueSum, size_t& sampleCount)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemNativeStatCalc::addDataToHistogramCalculator(RigHistogramCalculator& histogramCalculator)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemNativeStatCalc::timeStepCount()
{
    return m_resultsData->frameCount();
}
