/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigStimPlanFractureDefinition.h"

#include <QDebug>

#include "cvfMath.h"
#include "RivWellFracturePartMgr.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanResultFrames::RigStimPlanResultFrames()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::RigStimPlanFractureDefinition() : unitSet(RimUnitSystem::UNITS_UNKNOWN)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::~RigStimPlanFractureDefinition()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RigStimPlanFractureDefinition::getMirroredDataAtTimeIndex(const QString& resultName, const QString& unitName, size_t timeStepIndex) const
{
    std::vector<std::vector<double>> notMirrordedData = this->getDataAtTimeIndex(resultName, unitName, timeStepIndex);
    std::vector<std::vector<double>> mirroredData;

    for ( std::vector<double> depthData : notMirrordedData )
    {
        std::vector<double> mirrordDepthData = RivWellFracturePartMgr::mirrorDataAtSingleDepth(depthData);
        mirroredData.push_back(mirrordDepthData);
    }

    return mirroredData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigStimPlanFractureDefinition::timeStepExisist(double timeStepValueToCheck)
{
    for (double timeStep : timeSteps)
    {
        if (abs(timeStepValueToCheck - timeStep) < 1e-5) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::reorderYgridToDepths()
{
    std::vector<double> depthsInIncreasingOrder;
    for (double gridYvalue : gridYs)
    {
        depthsInIncreasingOrder.insert(depthsInIncreasingOrder.begin(), gridYvalue);
    }
    depths = depthsInIncreasingOrder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::getTimeStepIndex(double timeStepValue)
{
    size_t index = 0;
    while (index < timeSteps.size())
    {
        if (abs(timeSteps[index] - timeStepValue) < 1e-4)
        {
            return index;
        }
        index++;
    }
    return -1; //returns -1 if not found
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::totalNumberTimeSteps()
{
    return timeSteps.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigStimPlanFractureDefinition::resultIndex(const QString& resultName, const QString& unit) const
{
    
    for (size_t i = 0; i < stimPlanData.size(); i++)
    {
        if (stimPlanData[i].resultName == resultName && stimPlanData[i].unit == unit)
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::setDataAtTimeValue(QString resultName, QString unit, std::vector<std::vector<double>> data, double timeStepValue)
{
    size_t resIndex = resultIndex(resultName, unit);

    if (resIndex != cvf::UNDEFINED_SIZE_T)
    {
        stimPlanData[resIndex].parameterValues[getTimeStepIndex(timeStepValue)] = data;
    }
    else
    {
        RigStimPlanResultFrames resultData;

        resultData.resultName = resultName;
        resultData.unit = unit;

        std::vector<std::vector<std::vector<double>>>  values(timeSteps.size());
        resultData.parameterValues = values;
        resultData.parameterValues[getTimeStepIndex(timeStepValue)] = data;

        stimPlanData.push_back(resultData);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RigStimPlanFractureDefinition::getDataAtTimeIndex(const QString& resultName, const QString& unit, size_t timeStepIndex) const
{
    size_t resIndex = resultIndex(resultName, unit);

    if (resIndex != cvf::UNDEFINED_SIZE_T)
    {
        if (timeStepIndex < stimPlanData[resIndex].parameterValues.size())
        {
            return stimPlanData[resIndex].parameterValues[timeStepIndex];
        }
    }

    qDebug() << "ERROR: Requested parameter does not exists in stimPlan data";
    std::vector<std::vector<double>> emptyVector;
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigStimPlanFractureDefinition::computeMinMax(const QString& resultName, const QString& unit, double* minValue, double* maxValue) const
{
    CVF_ASSERT(minValue && maxValue);

    size_t resIndex = resultIndex(resultName, unit);
    if (resIndex == cvf::UNDEFINED_SIZE_T) return;

    for (auto timeValues : stimPlanData[resIndex].parameterValues)
    {
        for (auto values : timeValues)
        {
            for (auto resultValue : values)
            {
                if (resultValue < *minValue)
                {
                    *minValue = resultValue;
                }

                if (resultValue > *maxValue)
                {
                    *maxValue = resultValue;
                }
            }
        }
    }
}

