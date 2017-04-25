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
class RigStimPlanCell;
class RimStimPlanFractureTemplate;
class RigFractureStimPlanCellData;

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
    double                      computeHAupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigStimPlanCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers);
    double                      computeAHupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigStimPlanCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers);
    static double               arithmeticAverage(std::vector<double> values);

    // Calculations based on StimPlan grid
    void                        calculateStimPlanCellsMatrixTransmissibility(RigStimPlanCell* stimPlanCell, RigFractureStimPlanCellData* fracStimPlanCellData);
    static void                 computeStimPlanCellTransmissibilityInFracture(const RigStimPlanCell&  stimPlanCell);
    double                      computeRadialTransmissibilityToWellinStimPlanCell(const RigStimPlanCell&  stimPlanCell);
    double                      computeLinearTransmissibilityToWellinStimPlanCell(const RigStimPlanCell&  stimPlanCell, double perforationLengthVertical, double perforationLengthHorizontal);

    static std::vector<RigStimPlanCell*>    getRowOfStimPlanCells(std::vector<RigStimPlanCell>& allStimPlanCells, size_t i);
    static std::vector<RigStimPlanCell*>    getColOfStimPlanCells(std::vector<RigStimPlanCell>& allStimPlanCells, size_t j);

private: 
    double convertConductivtyValue(double Kw, RimDefines::UnitSystem fromUnit, RimDefines::UnitSystem toUnit);

private:
    RimEclipseCase*         m_case;
    RimFracture*            m_fracture;
    RimDefines::UnitSystem  m_unitForCalculation;

    double                  calculateMatrixTransmissibility(double permX, double NTG, double Ay, double dx, double skinfactor, double fractureAreaWeightedlength);
    double                  cDarcy();
};

