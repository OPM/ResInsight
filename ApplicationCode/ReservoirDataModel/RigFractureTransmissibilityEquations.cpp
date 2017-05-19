/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigFractureTransmissibilityEquations.h"
#include "cvfBase.h"
#include "cvfMath.h"
#include <cmath>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::computeStimPlanCellTransmissibilityInFracture(double conductivity, double sideLengthParallellTrans, double sideLengthNormalTrans)
{
    double transmissibility = conductivity * sideLengthNormalTrans / (sideLengthParallellTrans / 2);
    return transmissibility;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::computeRadialTransmissibilityToWellinStimPlanCell(double stimPlanCellConductivity, 
                                                                               double stimPlanCellSizeX,
                                                                               double stimPlanCellSizeZ,
                                                                               double wellRadius, 
                                                                               double skinFactor, 
                                                                               double cDarcyForRelevantUnit)
{
    double ro = 0.14 * cvf::Math::sqrt(
        pow(stimPlanCellSizeX, 2.0) + pow(stimPlanCellSizeZ, 2));

    double Tc = 2 * cvf::PI_D * cDarcyForRelevantUnit * stimPlanCellConductivity /
        (log(ro / wellRadius) + skinFactor );

    return Tc;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::computeLinearTransmissibilityToWellinStimPlanCell(double stimPlanConductivity, 
                                                                               double stimPlanCellSizeX,
                                                                               double stimPlanCellSizeZ, 
                                                                               double perforationLengthVertical,
                                                                               double perforationLengthHorizontal,
                                                                               double perforationEfficiency, 
                                                                               double skinfactor, 
                                                                               double cDarcyForRelevantUnit)
{
    double TcPrefix = 8 * cDarcyForRelevantUnit * stimPlanConductivity;

    double DzPerf = perforationLengthVertical * perforationEfficiency;
    double DxPerf = perforationLengthHorizontal * perforationEfficiency;

    double TcZ = TcPrefix * DzPerf /
        (stimPlanCellSizeX + skinfactor * DzPerf / cvf::PI_D);

    double TcX = TcPrefix * DxPerf /
        (stimPlanCellSizeZ + skinfactor* DxPerf / cvf::PI_D);

    double Tc = cvf::Math::sqrt(pow(TcX, 2) + pow(TcZ, 2));
    return Tc;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::calculateMatrixTransmissibility(double perm, 
                                                                             double NTG, 
                                                                             double A, 
                                                                             double cellSizeLength, 
                                                                             double skinfactor, 
                                                                             double fractureAreaWeightedlength, 
                                                                             double cDarcy)
{
    double transmissibility;

    double slDivPi = (skinfactor * fractureAreaWeightedlength) / cvf::PI_D;
    transmissibility = 8 * cDarcy * (perm * NTG) * A / (cellSizeLength + slDivPi);

    return transmissibility;
}
