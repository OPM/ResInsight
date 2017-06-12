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


#include "RimUnitSystem.h"

#include "cafAppEnum.h"

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfVector3.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include <vector>
#include <QString>
#include "RigFracture.h"

//==================================================================================================
/// 
//==================================================================================================

class RigFracturedEclipseCellExportData
{
public:
    RigFracturedEclipseCellExportData() {};

    // Compdat export data
    size_t reservoirCellIndex; 
    double transmissibility; // Total cell to well transmissibility finally used in COMPDAT keyword
    bool cellIsActive;

    // General intermediate results
    double NTG;
    cvf::Vec3d permeabilities;
    double skinFactor;

    // Elipse fracture related values
    cvf::Vec3d transmissibilities; //matrixToFractureTransmissibilitiesXYZ
    double totalArea; // Elipse cell overlap area
    double fractureLenght;
    cvf::Vec3d projectedAreas;

    cvf::Vec3d cellSizes;

    //TODO: Used for upscaling - should be moved?
    double upscaledStimPlanValueHA;
    double upscaledStimPlanValueAH;
};


class RimFracture;
class RimEclipseCase;
class RigFractureCell;
class RimStimPlanFractureTemplate;

//==================================================================================================
/// 
//==================================================================================================
class RigFractureTransCalc
{
public:
    explicit RigFractureTransCalc(RimEclipseCase* caseToApply, RimFracture* fracture);

    // Calculations based on fracture polygon and eclipse grid cells
    std::vector<RigFracturedEclipseCellExportData> computeTransmissibilityFromPolygonWithInfiniteConductivityInFracture();
   
    static bool                 planeCellIntersectionPolygons(cvf::Vec3d hexCorners[8],
                                                             cvf::Mat4f transformMatrixForPlane,
                                                             std::vector<std::vector<cvf::Vec3d> > & polygons);

private: 
    static double convertConductivtyValue(double Kw, RimUnitSystem::UnitSystem fromUnit, RimUnitSystem::UnitSystem toUnit);

private:
    RimEclipseCase*         m_case;
    RimFracture*            m_fracture;
    RimUnitSystem::UnitSystem  m_unitForCalculation;

};
