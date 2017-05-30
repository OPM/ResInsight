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
#include "RigFracture.h"

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
    static double convertConductivtyValue(double Kw, RimDefines::UnitSystem fromUnit, RimDefines::UnitSystem toUnit);

private:
    RimEclipseCase*         m_case;
    RimFracture*            m_fracture;
    RimDefines::UnitSystem  m_unitForCalculation;

};
