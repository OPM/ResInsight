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


#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <vector>
#include <QString>
#include "cafAppEnum.h"
#include "RimDefines.h"


class RimFracture;
class RimEclipseCase;

// class RigFractureData
// {
// public:
//     RigFractureData();
// 
//     size_t reservoirCellIndex;
//     double transmissibility;
//     cvf::Vec3d transmissibilities;
//     
//     double totalArea;
//     double fractureLenght;
//     cvf::Vec3d projectedAreas;
// 
//     cvf::Vec3d permeabilities;
//     cvf::Vec3d cellSizes;
//     double NTG;
//     double skinFactor;
// 
//     bool cellIsActive;
//     
//     //TODO: Used for upscaling - should be moved?
//     double upscaledAritmStimPlanValue;
//     double upscaledHarmStimPlanValue;
// 
// 
// };

//==================================================================================================
/// 
//==================================================================================================
class RigFractureTransCalc : public cvf::Object
{
public:
    RigFractureTransCalc();

    static void                     computeTransmissibility(RimEclipseCase* caseToApply, RimFracture* fracture);

   static  void                     computeUpscaledPropertyFromStimPlanForEclipseCell(double &upscaledAritmStimPlanValue, double &upscaledHarmStimPlanValue, RimFracture* fracture, RimEclipseCase* caseToApply, QString resultName, QString resultUnit, size_t timeStepIndex, caf::AppEnum< RimDefines::UnitSystem > unitSystem, size_t cellIndex);
    static double                   areaWeightedHarmonicAverage(std::vector<double> areaOfFractureParts, std::vector<double> valuesForFractureParts);
    static double                   areaWeightedArithmeticAverage(std::vector<double> areaOfFractureParts, std::vector<double> valuesForFractureParts);

    static void                     computeUpscaledPropertyFromStimPlan(RimEclipseCase* caseToApply, RimFracture* fracture, QString resultName, QString resultUnit, size_t timeStepIndex);

    static void                     computeFlowInFracture(RimEclipseCase* caseToApply, RimFracture* fracture);
    static void                     computeFlowIntoTransverseWell(RimEclipseCase* caseToApply, RimFracture* fracture);


private:

};

