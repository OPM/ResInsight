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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include <QString>

#include <vector>

class RigStimPlanResultFrames
{
public:
    RigStimPlanResultFrames();

    QString resultName;
    QString unit;
    std::vector<std::vector<std::vector<double>>> parameterValues;
    //Vector for each time step, for each depth and for each x-value

};

class RigStimPlanFractureDefinition: public cvf::Object
{ 
public:
    RigStimPlanFractureDefinition();
    ~RigStimPlanFractureDefinition();

    std::vector<double>     gridXs;
    std::vector<double>     gridYs;
    //TODO: Consider removing gridYs or depths, 
    //In example file these are the same, but can there be examples where not all gridY values are included in depths?

    std::vector<double>     timeSteps;
    std::vector<double>     depths;

    
    std::vector<RigStimPlanResultFrames>        stimPlanData;

    bool                                timeStepExisist(double timeStepValue);
    void                                reorderYgridToDepths();
    size_t                              getTimeStepIndex(double timeStepValue);
    size_t                              totalNumberTimeSteps();

    void                                setDataAtTimeValue(QString resultName, QString unit, std::vector<std::vector<double>> data, double timeStepValue);
    std::vector<std::vector<double>>    getDataAtTimeIndex(const QString& resultName, const QString& unit, size_t timeStepIndex) const;
    void                                computeMinMax(const QString& resultName, const QString& unit, double* minValue, double* maxValue) const;

private:
    size_t                              resultIndex(const QString& resultName, const QString& unit) const;

};


