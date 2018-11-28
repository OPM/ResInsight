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

#include "RigTransmissibilityEquations.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityEquations::wellBoreTransmissibilityComponent(double cellPerforationVectorComponent,
                                                                       double permeabilityNormalDirection1,
                                                                       double permeabilityNormalDirection2,
                                                                       double cellSizeNormalDirection1,
                                                                       double cellSizeNormalDirection2,
                                                                       double wellRadius,
                                                                       double skinFactor,
                                                                       double cDarcyForRelevantUnit)
{
    double K = cvf::Math::sqrt(permeabilityNormalDirection1 * permeabilityNormalDirection2);

    double nominator = cDarcyForRelevantUnit * 2 * cvf::PI_D * K * cellPerforationVectorComponent;

    double peaceManRad = peacemanRadius(
        permeabilityNormalDirection1, permeabilityNormalDirection2, cellSizeNormalDirection1, cellSizeNormalDirection2);

    double denominator = log(peaceManRad / wellRadius) + skinFactor;

    double trans = nominator / denominator;
    return trans;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityEquations::totalConnectionFactor(double transX, double transY, double transZ)
{
    return cvf::Math::sqrt(pow(transX, 2.0) + pow(transY, 2.0) + pow(transZ, 2.0));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityEquations::totalKh(double            cellPermX,
                                             double            cellPermY,
                                             double            cellPermZ,
                                             const cvf::Vec3d& internalCellLengths,
                                             double            lateralNtg,
                                             double            ntg)
{
    // Compute kh for each local grid cell axis
    // Use permeability values for the two other axis
    double khx = sqrt(cellPermY * cellPermZ) * internalCellLengths.x() * lateralNtg;
    double khy = sqrt(cellPermX * cellPermZ) * internalCellLengths.y() * lateralNtg;
    double khz = sqrt(cellPermX * cellPermY) * internalCellLengths.z() * ntg;

    const double totKh = cvf::Math::sqrt(khx * khx + khy * khy + khz * khz);

    return totKh;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityEquations::effectiveK(double            cellPermX,
                                                double            cellPermY,
                                                double            cellPermZ,
                                                const cvf::Vec3d& internalCellLengths,
                                                double            lateralNtg,
                                                double            ntg)
{
    // Compute kh for each local grid cell axis
    // Use permeability values for the two other axis

    double lx = internalCellLengths.x() * lateralNtg;
    double ly = internalCellLengths.y() * lateralNtg;
    double lz = internalCellLengths.z() * ntg;

    double khx = sqrt(cellPermY * cellPermZ) * lx;
    double khy = sqrt(cellPermX * cellPermZ) * ly;
    double khz = sqrt(cellPermX * cellPermY) * lz;

    double nominator   = khx + khy + khz;
    double denominator = lx + ly + lz;

    const double effK = nominator / denominator;

    return effK;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityEquations::effectiveH(const cvf::Vec3d& internalCellLengths, double lateralNtg, double ntg)
{
    double lx = internalCellLengths.x() * lateralNtg;
    double ly = internalCellLengths.y() * lateralNtg;
    double lz = internalCellLengths.z() * ntg;

    double effH = cvf::Math::sqrt(lx*lx + ly*ly + lz*lz);

    return effH;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityEquations::permeability(const double conductivity, const double width)
{
    double threshold = 1e-7;

    if (std::fabs(width) > threshold)
    {
        double perm = conductivity / width;

        return perm;
    }
    else
    {
        return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityEquations::peacemanRadius(double permeabilityNormalDirection1,
                                                    double permeabilityNormalDirection2,
                                                    double cellSizeNormalDirection1,
                                                    double cellSizeNormalDirection2)
{
    double numerator = cvf::Math::sqrt(
        pow(cellSizeNormalDirection2, 2.0) * pow(permeabilityNormalDirection1 / permeabilityNormalDirection2, 0.5) +
        pow(cellSizeNormalDirection1, 2.0) * pow(permeabilityNormalDirection2 / permeabilityNormalDirection1, 0.5));

    double denominator = pow((permeabilityNormalDirection1 / permeabilityNormalDirection2), 0.25) +
                         pow((permeabilityNormalDirection2 / permeabilityNormalDirection1), 0.25);

    double r0 = 0.28 * numerator / denominator;

    return r0;
}
