/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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


#include "RimDefines.h"

#include "cafAppEnum.h"

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfVector3.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include <vector>
#include <QString>

class RimFracture;
class RimEclipseCase;
class RigStimPlanFracTemplateCell;
class RimStimPlanFractureTemplate;
class RigStimPlanFractureCell;

//==================================================================================================
/// 
//==================================================================================================
class RigFractureTransCalc
{
public:
    explicit RigFractureTransCalc(RimEclipseCase* caseToApply, RimFracture* fracture);

    // Calculations based on fracture polygon and eclipse grid cells
    void                        computeTransmissibilityFromPolygonWithInfiniteConductivityInFracture();
    bool                        planeCellIntersectionPolygons(size_t cellindex, std::vector<std::vector<cvf::Vec3d> > & polygons, cvf::Vec3d & localX, cvf::Vec3d & localY, cvf::Vec3d & localZ);

    // Functions needed for upscaling from StimPlan grid to Eclipse Grid, for transmissibility calculations on eclipse grid
    // Obsolete if final calculations will be done on the stimPlan grid
    void                        computeUpscaledPropertyFromStimPlan(QString resultName, QString resultUnit, size_t timeStepIndex);  
    std::pair<double, double>   flowAcrossLayersUpscaling(QString resultName, QString resultUnit, size_t timeStepIndex, RimDefines::UnitSystem unitSystem, size_t eclipseCellIndex);
    double                      computeHAupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigStimPlanFracTemplateCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers);
    double                      computeAHupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigStimPlanFracTemplateCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers);
    static double               arithmeticAverage(std::vector<double> values);

    // Calculations based on StimPlan grid
    static double               computeStimPlanCellTransmissibilityInFracture(double conductivity, double sideLengthParallellTrans, double sideLengthNormalTrans);

    void                        calculateStimPlanCellsMatrixTransmissibility(RigStimPlanFracTemplateCell* stimPlanCell, RigStimPlanFractureCell* fracStimPlanCellData);
    double                      computeRadialTransmissibilityToWellinStimPlanCell(const RigStimPlanFracTemplateCell&  stimPlanCell);
    double                      computeLinearTransmissibilityToWellinStimPlanCell(const RigStimPlanFracTemplateCell&  stimPlanCell, double perforationLengthVertical, double perforationLengthHorizontal);

    static std::vector<RigStimPlanFracTemplateCell*>    getRowOfStimPlanCells(std::vector<RigStimPlanFracTemplateCell>& allStimPlanCells, size_t i);
    static std::vector<RigStimPlanFracTemplateCell*>    getColOfStimPlanCells(std::vector<RigStimPlanFracTemplateCell>& allStimPlanCells, size_t j);

private: 
    double convertConductivtyValue(double Kw, RimDefines::UnitSystem fromUnit, RimDefines::UnitSystem toUnit);
    double calculateMatrixTransmissibility(double permX, double NTG, double Ay, double dx, double skinfactor, double fractureAreaWeightedlength);

private:
    RimEclipseCase*         m_case;
    RimFracture*            m_fracture;
    RimDefines::UnitSystem  m_unitForCalculation;

    double                  cDarcy();
};

class EclipseToStimPlanCellTransmissibilityCalculator
{
public:
    explicit EclipseToStimPlanCellTransmissibilityCalculator(RimEclipseCase* caseToApply,
                                                            cvf::Mat4f fractureTransform,
                                                            double skinFactor,
                                                            double cDarcy,
                                                            const RigStimPlanFracTemplateCell& stimPlanCell);

    const std::vector<size_t>&     globalIndeciesToContributingEclipseCells();
    const std::vector<double>&     contributingEclipseCellTransmissibilities();

private:
    void                        calculateStimPlanCellsMatrixTransmissibility();
    static std::vector<size_t>  getPotentiallyFracturedCellsForPolygon(std::vector<cvf::Vec3d> polygon);
    bool                        planeCellIntersectionPolygons(size_t cellindex, std::vector<std::vector<cvf::Vec3d> > & polygons,
                                                              cvf::Vec3d & localX, cvf::Vec3d & localY, cvf::Vec3d & localZ);
    double                      calculateMatrixTransmissibility(double permX, double NTG, double Ay, double dx, double skinfactor, double fractureAreaWeightedlength);


    RimEclipseCase*                     m_case;
    double                              m_cDarcy;
    double                              m_fractureSkinFactor;
    cvf::Mat4f                          m_fractureTransform;
    const RigStimPlanFracTemplateCell&  m_stimPlanCell;

    std::vector<size_t>                 m_globalIndeciesToContributingEclipseCells;
    std::vector<double>                 m_contributingEclipseCellTransmissibilities;

};