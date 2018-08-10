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

#include "RiaEclipseUnitTools.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <QString>

#include <vector>

class RigFractureGrid;
class MinMaxAccumulator;
class PosNegAccumulator;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigStimPlanResultFrames
{
public:
    RigStimPlanResultFrames() {}

    QString resultName;
    QString unit;

    // Vector for each time step, for each y and for each x-value
    std::vector< std::vector< std::vector<double> > > parameterValues;

};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigStimPlanFractureDefinition: public cvf::Object
{ 
    friend class RifStimPlanXmlReader;

public:
    static const double THRESHOLD_VALUE;

    RigStimPlanFractureDefinition();
    ~RigStimPlanFractureDefinition();

    RiaEclipseUnitTools::UnitSystem unitSet() const;
    size_t                          xCount() const;
    size_t                          yCount() const;
    double                          minDepth() const;
    double                          maxDepth() const;
    double                          topPerfTvd() const;
    double                          bottomPerfTvd() const;
    void                            setTvdToTopPerf(double topPerfTvd);
    void                            setTvdToBottomPerf(double bottomPerfTvd);
    
    cvf::ref<RigFractureGrid>       createFractureGrid(const QString&                      resultName,
                                                       int                                 activeTimeStepIndex,
                                                       double                              wellPathIntersectionAtFractureDepth,
                                                       RiaEclipseUnitTools::UnitSystem     requiredUnitSet) const;
    
    void                            createFractureTriangleGeometry(double                          wellPathIntersectionAtFractureDepth,
                                                                   const QString&                  fractureUserName,
                                                                   std::vector<cvf::Vec3f>*        vertices,
                                                                   std::vector<cvf::uint>*         triangleIndices) const;
    
    const std::vector<double>&               timeSteps() const;
    void                                     addTimeStep(double time);
    size_t                                   totalNumberTimeSteps() const;

    std::vector<std::pair<QString, QString>> getStimPlanPropertyNamesUnits() const;
    
    void                                     setDataAtTimeValue(QString resultName, 
                                                                QString unit, 
                                                                std::vector<std::vector<double>> data, 
                                                                double timeStepValue);

    const std::vector<std::vector<double>>& getDataAtTimeIndex(const QString& resultName,
                                                               const QString& unit,
                                                               size_t timeStepIndex) const;

    std::vector<double>                     fractureGridResults(const QString& resultName, const QString& unitName, size_t timeStepIndex) const;

    void                                    appendDataToResultStatistics(const QString&     resultName,
                                                                         const QString&     unit,
                                                                         MinMaxAccumulator& minMaxAccumulator,
                                                                         PosNegAccumulator& posNegAccumulator) const;

    QStringList                             conductivityResultNames() const;

private:
    bool                                    timeStepExists(double timeStepValue) const;
    size_t                                  getTimeStepIndex(double timeStepValue) const;
    size_t                                  resultIndex(const QString& resultName, const QString& unit) const;
    void                                    generateXsFromFileXs(bool xMirrorMode);
    std::vector<std::vector<double>>        generateDataLayoutFromFileDataLayout(std::vector<std::vector<double>> rawXYData) const;
    std::vector<double>                     adjustedYCoordsAroundWellPathPosition(double wellPathIntersectionAtFractureDepth) const;
    bool                                    numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep) const;
    double                                  minY() const;
    double                                  maxY() const;
    void                                    scaleXs(double scaleFactor);
    void                                    scaleYs(double scaleFactor, double wellPathIntersectionY);

    std::vector<std::vector<double>>        conductivityValuesAtTimeStep(const QString&                  resultName,
                                                                         int                             activeTimeStepIndex,
                                                                         RiaEclipseUnitTools::UnitSystem requiredUnitSet) const;

private:
    RiaEclipseUnitTools::UnitSystem         m_unitSet; // To be deleted
    std::vector<double>                     m_fileXs;
    std::vector<double>                     m_Ys;
    std::vector<double>                     m_timeSteps;

    std::vector<RigStimPlanResultFrames>    m_stimPlanResults;
    std::vector<double>                     m_Xs;
    bool                                    m_xMirrorMode;

    double                                  m_topPerfTvd;
    double                                  m_bottomPerfTvd;
};
