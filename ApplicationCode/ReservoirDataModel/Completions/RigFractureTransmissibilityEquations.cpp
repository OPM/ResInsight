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
#include "cvfVector2.h"

#include <cmath>

const double RigFractureTransmissibilityEquations::EPSILON = 1.0e-9;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::centerToCenterFractureCellTrans(double conductivityCell1, 
                                                                             double sideLengthParallellTransCell1, 
                                                                             double sideLengthNormalTransCell1, 
                                                                             double conductivityCell2, 
                                                                             double sideLengthParallellTransCell2, 
                                                                             double sideLengthNormalTransCell2, 
                                                                             double cDarcyForRelevantUnit)
{
    double transCell1 = centerToEdgeFractureCellTrans(conductivityCell1, sideLengthParallellTransCell1, sideLengthNormalTransCell1, cDarcyForRelevantUnit);
    double transCell2 = centerToEdgeFractureCellTrans(conductivityCell2, sideLengthParallellTransCell2, sideLengthNormalTransCell2, cDarcyForRelevantUnit);

    double totalTrans = 1 / ( (1 / transCell1) + (1 / transCell2));

    return totalTrans;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::fractureCellToWellRadialTrans(double fractureCellConductivity, 
                                                                           double fractureCellSizeX,
                                                                           double fractureCellSizeZ,
                                                                           double wellRadius, 
                                                                           double skinFactor, 
                                                                           double cDarcyForRelevantUnit)
{
    double ro = 0.14 * cvf::Math::sqrt(
        pow(fractureCellSizeX, 2.0) + pow(fractureCellSizeZ, 2));

    if (ro < (wellRadius * 1.01))
    {
        ro = wellRadius * 1.01;
    }

    double Tc = 2 * cvf::PI_D * cDarcyForRelevantUnit * fractureCellConductivity /
        (log(ro / wellRadius) + skinFactor );

    CVF_TIGHT_ASSERT(Tc > 0);
    return Tc;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::fractureCellToWellLinearTrans(double fractureConductivity, 
                                                                           double fractureCellSizeX,
                                                                           double fractureCellSizeZ, 
                                                                           double perforationLengthVertical,
                                                                           double perforationLengthHorizontal,
                                                                           double perforationEfficiency, 
                                                                           double skinfactor, 
                                                                           double cDarcyForRelevantUnit,
                                                                           double wellRadius)
{
    const double invalidTrans = 1.0e9;
    const double epsilon      = 1.0e-8;

    double TcPrefix = 8 * cDarcyForRelevantUnit * fractureConductivity;

    cvf::Vec2d wellOrientation  = cvf::Vec2d(perforationLengthHorizontal, perforationLengthVertical).getNormalized();
    cvf::Vec2d wellRadialVector = wellOrientation.perpendicularVector() * wellRadius;

    double DzPerf = perforationLengthVertical   * perforationEfficiency;
    double DxPerf = perforationLengthHorizontal * perforationEfficiency;

    double TcZ = 0.0;
    if (DzPerf > epsilon)
    {
        double effectiveFlowLengthHorizontal = fractureCellSizeX - 4.0 * std::abs(wellRadialVector.x());
        double denominatorZ = effectiveFlowLengthHorizontal + skinfactor * DzPerf / cvf::PI_D;
        if (denominatorZ < epsilon)
        {
            return invalidTrans;
        }
        TcZ = TcPrefix * DzPerf / denominatorZ;
    }

    double TcX = 0.0;
    if (DxPerf > epsilon)
    {
        double effectiveFlowLengthVertical = fractureCellSizeZ - 4.0 * std::abs(wellRadialVector.y());
        double denominatorX = effectiveFlowLengthVertical + skinfactor * DxPerf / cvf::PI_D;
        if (denominatorX < epsilon)
        {
            return invalidTrans;
        }
        TcX = TcPrefix * DxPerf / denominatorX;
    }

    double Tc = cvf::Math::sqrt(pow(TcX, 2) + pow(TcZ, 2));
    return Tc;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::matrixToFractureTrans(double perm, 
                                                                   double NTG, 
                                                                   double A, 
                                                                   double cellSizeLength, 
                                                                   double skinfactor, 
                                                                   double fractureAreaWeightedlength, 
                                                                   double cDarcy)
{
    double transmissibility;

    double slDivPi = 0.0;
    if ( cvf::Math::abs(skinfactor) > EPSILON)
    {
        slDivPi = (skinfactor * fractureAreaWeightedlength) / cvf::PI_D;
    }

    transmissibility = 8 * cDarcy * (perm * NTG) * A / (cellSizeLength + slDivPi);

    CVF_ASSERT(transmissibility == transmissibility);
    return transmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::effectiveInternalFractureToWellTransPDDHC(double sumScaledMatrixToFractureTrans,
                                                                                       double scaledMatrixToWellTrans)
{
    double divisor = sumScaledMatrixToFractureTrans - scaledMatrixToWellTrans;
    if (cvf::Math::abs(divisor) > EPSILON)
    {
        return (sumScaledMatrixToFractureTrans * scaledMatrixToWellTrans) / divisor;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::effectiveMatrixToWellTransPDDHC(double sumOriginalMatrixToFractureTrans,
                                                                             double effectiveInternalFractureToWellTrans)
{
    double divisor = sumOriginalMatrixToFractureTrans + effectiveInternalFractureToWellTrans;
    if (cvf::Math::abs(divisor) > EPSILON)
    {
        return (sumOriginalMatrixToFractureTrans * effectiveInternalFractureToWellTrans) / divisor;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransmissibilityEquations::centerToEdgeFractureCellTrans(double conductivity, 
                                                                           double sideLengthParallellTrans, 
                                                                           double sideLengthNormalTrans, 
                                                                           double cDarcyForRelevantUnit)
{
    double transmissibility = cDarcyForRelevantUnit * conductivity * sideLengthNormalTrans / (sideLengthParallellTrans / 2);
    return transmissibility;
}


