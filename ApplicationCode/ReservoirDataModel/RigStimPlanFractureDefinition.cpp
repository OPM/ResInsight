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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanData::RigStimPlanData()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::RigStimPlanFractureDefinition()
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
void RigStimPlanFractureDefinition::setDataAtTimeValue(QString resultName, QString unit, std::vector<std::vector<double>> data, double timeStepValue)
{
    bool resultNameExists = false;
    for (RigStimPlanData& resultData : stimPlanData)
    {
        if (resultData.resultName == resultName)
        {
            resultNameExists = true;
            resultData.parameterValues[getTimeStepIndex(timeStepValue)] = data;
            return;
        }
    }

    if (!resultNameExists)
    {
        RigStimPlanData resultData;

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
std::vector<std::vector<double>> RigStimPlanFractureDefinition::getDataAtTimeIndex(QString resultName, size_t timeStepIndex)
{
    for (RigStimPlanData& resultData : stimPlanData)
    {
        if (resultData.resultName == resultName)
        {
            return resultData.parameterValues[timeStepIndex];
        }
    }

    qDebug() << "ERROR: Requested parameter does not exists in stimPlan data";
    std::vector<std::vector<double>> emptyVector;
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigStimPlanFractureDefinition::resultNames() const
{
    std::vector<QString> names;

    names.push_back("Conductivity");
    names.push_back("Permeability");
    names.push_back("Width");

    return names;
}
