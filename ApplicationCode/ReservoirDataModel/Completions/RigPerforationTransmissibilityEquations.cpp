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

#include "RigPerforationTransmissibilityEquations.h"

#include "cvfBase.h"
#include "cvfMath.h"

#include <cmath>

const double RigPerforationTransmissibilityEquations::EPSILON = 1.0e-9;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigPerforationTransmissibilityEquations::betaFactor(double intertialCoefficient,
                                                           double effectivePermeability,
                                                           double permeabilityScalingFactor,
                                                           double porosity,
                                                           double porosityScalingFactor)
{
    const double scaledEffectivePermeability = std::pow(effectivePermeability, permeabilityScalingFactor);
    const double scaledPorosity              = std::pow(porosity, porosityScalingFactor);

    return intertialCoefficient * scaledEffectivePermeability * scaledPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigPerforationTransmissibilityEquations::dFactor(double unitConstant,
                                                        double betaFactor,
                                                        double effectivePermeability,
                                                        double perforationLengthInCell,
                                                        double wellRadius,
                                                        double gasDenity,
                                                        double gasViscosity)
{
    // clang-format off
    //      
    //                      Ke    1    gasDensity  
    //  D = alpha * beta *  -- * -- *  ------------
    //                       h   rw    gasViscosity
    //
    // clang-format on

    const double keOverH                    = effectivePermeability / perforationLengthInCell;
    const double oneOverRw                  = 1.0 / wellRadius;
    const double gasDensityOverGasViscosity = gasDenity / gasViscosity;

    const double D = unitConstant * betaFactor * keOverH * oneOverRw * gasDensityOverGasViscosity;

    return D;
}
