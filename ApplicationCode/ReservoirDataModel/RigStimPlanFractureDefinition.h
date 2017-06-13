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
#include "RimUnitSystem.h"
#include "cvfVector3.h"

class RigFractureGrid;

class RigStimPlanResultFrames
{
public:
    RigStimPlanResultFrames();

    QString resultName;
    QString unit;

    // Vector for each time step, for each depth and for each x-value
    std::vector< std::vector< std::vector<double> > > parameterValues;

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

    RimUnitSystem::UnitSystem unitSet;

    std::vector<double> getNegAndPosXcoords() const
    {
        std::vector<double> allXcoords;
        for (const double& xCoord : gridXs)
        {
            if (xCoord > 1e-5)
            {
                double negXcoord = -xCoord;
                allXcoords.insert(allXcoords.begin(), negXcoord);
            }
        }
        for (const double& xCoord : gridXs)
        {
            allXcoords.push_back(xCoord);
        }

        return allXcoords;
    }

    bool numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep)
    {
        size_t depths = this->depths.size();
        size_t gridXvalues = this->gridXs.size();

        if (propertyValuesAtTimestep.size() != depths)  return false;
        for (std::vector<double> valuesAtDepthVector : propertyValuesAtTimestep)
        {
            if (valuesAtDepthVector.size() != gridXvalues) return false;
        }

        return true;
    }

    std::vector<double>  adjustedDepthCoordsAroundWellPathPosition(double wellPathDepthAtFracture) const
    {
        std::vector<double> depthRelativeToWellPath;

        for (const double& depth : this->depths)
        {
            double adjustedDepth = depth - wellPathDepthAtFracture;
            adjustedDepth = -adjustedDepth;
            depthRelativeToWellPath.push_back(adjustedDepth);
        }
        return depthRelativeToWellPath;
    }

    std::vector<std::pair<QString, QString> > getStimPlanPropertyNamesUnits() const
    {
        std::vector<std::pair<QString, QString> >  propertyNamesUnits;
        {
            std::vector<RigStimPlanResultFrames > allStimPlanData = this->stimPlanData;
            for (RigStimPlanResultFrames stimPlanDataEntry : allStimPlanData)
            {
                propertyNamesUnits.push_back(std::make_pair(stimPlanDataEntry.resultName, stimPlanDataEntry.unit));
            }
        }
        return propertyNamesUnits;
    }

   
    std::vector<std::vector<double>> getMirroredDataAtTimeIndex(const QString& resultName, 
                                                                const QString& unitName, 
                                                                size_t timeStepIndex) const;


    cvf::ref<RigFractureGrid> createFractureGrid(const QString& resultNameFromColors,
                                                 const QString& resultUnitFromColors,
                                                 int m_activeTimeStepIndex,
                                                 RimUnitSystem::UnitSystemType fractureTemplateUnit,
                                                 double m_wellPathDepthAtFracture);

    void createFractureTriangleGeometry(double m_wellPathDepthAtFracture, 
                                        RimUnitSystem::UnitSystem neededUnit, 
                                        const QString& fractureUserName, 
                                        std::vector<cvf::Vec3f>* vertices, 
                                        std::vector<cvf::uint>* triangleIndices);

    std::vector<cvf::Vec3f> createFractureBorderPolygon(const QString& resultName, 
                                                        const QString& resultUnit, 
                                                        int m_activeTimeStepIndex, 
                                                        double m_wellPathDepthAtFracture, 
                                                        RimUnitSystem::UnitSystem neededUnit, 
                                                        const QString& fractureUserName);

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


