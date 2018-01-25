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
#include "RiaEclipseUnitTools.h"
#include "cvfVector3.h"

class RigFractureGrid;
class MinMaxAccumulator;
class PosNegAccumulator;

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

    RiaEclipseUnitTools::UnitSystem           unitSet() const { return m_unitSet; }
    void                                      setUnitSet(RiaEclipseUnitTools::UnitSystem unitset) { m_unitSet = unitset;}

    size_t                                    gridXCount() const { return m_gridXs.size();}
    void                                      setGridXs(const std::vector<double>& gridXs) {  m_gridXs = gridXs; }


    //TODO: Consider removing gridYs or depths, 
    //In example file these are the same, but can there be examples where not all gridY values are included in depths?
    
    void                                      setGridYs(const std::vector<double>& gridYs) {  m_gridYs = gridYs; }

    double                                    minDepth()   const { return depths[0]; }
    double                                    maxDepth()   const { return depths.back(); }
    size_t                                    depthCount() const { return depths.size(); }

    // Grid Geometry

    std::vector<double>                       getNegAndPosXcoords() const;
    std::vector<double>                       adjustedDepthCoordsAroundWellPathPosition(double wellPathDepthAtFracture) const;
                                              
    cvf::ref<RigFractureGrid>                 createFractureGrid(const QString& resultName,
                                                                 int m_activeTimeStepIndex,
                                                                 RiaEclipseUnitTools::UnitSystemType fractureTemplateUnit,
                                                                 double m_wellPathDepthAtFracture);
                                              

    void                                      createFractureTriangleGeometry(double m_wellPathDepthAtFracture,
                                                                             RiaEclipseUnitTools::UnitSystem neededUnit,
                                                                             const QString& fractureUserName,
                                                                             std::vector<cvf::Vec3f>* vertices,
                                                                             std::vector<cvf::uint>* triangleIndices);
                                              
    std::vector<cvf::Vec3f>                   createFractureBorderPolygon(const QString& resultName,
                                                                          const QString& resultUnit,
                                                                          int m_activeTimeStepIndex,
                                                                          double m_wellPathDepthAtFracture,
                                                                          RiaEclipseUnitTools::UnitSystem neededUnit,
                                                                          const QString& fractureUserName);
    // Result Access                                          
    
    const std::vector<double>&                timeSteps() const { return m_timeSteps; }
    void                                      addTimeStep(double time) { if (!timeStepExisist(time)) m_timeSteps.push_back(time); }

    std::vector<std::pair<QString, QString> > getStimPlanPropertyNamesUnits() const;
    bool                                      numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep);
    size_t                                    totalNumberTimeSteps();
    void                                      setDataAtTimeValue(QString resultName, QString unit, std::vector<std::vector<double>> data, double timeStepValue);
    const std::vector<std::vector<double>>&   getDataAtTimeIndex(const QString& resultName, const QString& unit, size_t timeStepIndex) const;
    std::vector<double>                       fractureGridResults(const QString& resultName, 
                                                                  const QString& unitName, 
                                                                  size_t timeStepIndex) const;

    void                                    appendDataToResultStatistics(const QString& resultName, const QString& unit, 
                                                                          MinMaxAccumulator& minMaxAccumulator,
                                                                          PosNegAccumulator& posNegAccumulator) const;

    QStringList                               conductivityResultNames() const;
    
    // Setup                          
    void                                      reorderYgridToDepths();

private:                                      
    bool                                      timeStepExisist(double timeStepValue);
    size_t                                    getTimeStepIndex(double timeStepValue);
    size_t                                    resultIndex(const QString& resultName, const QString& unit) const;
    size_t                                    mirroredGridXCount() const { return m_gridXs.size() ? m_gridXs.size() + m_gridXs.size() - 1  : 0 ;}

    std::vector<std::vector<double>>          getMirroredDataAtTimeIndex(const QString& resultName,
                                                                        const QString& unitName,
                                                                        size_t timeStepIndex) const;
                                              

private:
    RiaEclipseUnitTools::UnitSystem           m_unitSet;
    std::vector<double>                       m_gridXs;
    std::vector<double>                       m_gridYs;
    std::vector<double>                       depths;

    std::vector<double>                       m_timeSteps;
    std::vector<RigStimPlanResultFrames>      m_stimPlanResults;
};


