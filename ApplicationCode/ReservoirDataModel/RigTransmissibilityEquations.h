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

#pragma once

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfVector3.h"

class RigTransmissibilityEquations
{
public:
    // Calculations are assuming an orthogonal coordinate system

    static double wellBoreTransmissibilityComponent(double cellPerforationVectorComponent,
                                                    double permeabilityNormalDirection1,
                                                    double permeabilityNormalDirection2,
                                                    double cellSizeNormalDirection1,
                                                    double cellSizeNormalDirection2,
                                                    double wellRadius,
                                                    double skinFactor,
                                                    double cDarcyForRelevantUnit);

    static double totalConnectionFactor(double transX, double transY, double transZ);

    static double totalPermeability(double            cellPermX,
                                    double            cellPermY,
                                    double            cellPermZ,
                                    const cvf::Vec3d& internalCellLengths,
                                    double            lateralNtg,
                                    double            ntg);

    static double permeability(const double conductivity, const double width);

private:
    // If using wellBoreTransmissibilityComponent to calculate Tx (transmissibility in x direction),
    // perforationVectorComponent is the x component (in the cell local coordinate system) of the perforation vector
    // permeability and cell size for Z and Y are to be specified as "normal directions" 1 and 2
    // but normal directions 1 and 2 are interchangeable (so Z=1, Y=2 and Z=2, Y=1 gives same result)

    static double peacemanRadius(double permeabilityNormalDirection1,
                                 double permeabilityNormalDirection2,
                                 double cellSizeNormalDirection1,
                                 double cellSizeNormalDirection2);
};
