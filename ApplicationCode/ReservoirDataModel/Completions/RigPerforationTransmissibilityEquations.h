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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigPerforationTransmissibilityEquations
{
public:
    static double effectivePermeability(double permeability,
                                        double krFactor);

    static double betaFactor(double intertialCoefficient,
                             double effectivePermeability,
                             double permeabilityScalingFactor,
                             double porosity,
                             double porosityScalingFactor);

    static double dFactor(double unitConstant,
                          double betaFactor,
                          double effectivePermeability,
                          double perforationLengthInCell,
                          double wellRadius,
                          double gasDensity,
                          double gasViscosity);

    static double kh(double effectivePermeability,
                     double perforationLengthInCell);

private:
    static const double EPSILON;
};
