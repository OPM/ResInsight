/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "gtest/gtest.h"

#include "RigPerforationTransmissibilityEquations.h"

#include <cmath>

TEST( RigPerforationTransmissibilityEquations, BetaFactor )
{
    // beta = inertialCoefficient * Ke^permScaling * phi^porosityScaling
    double beta = RigPerforationTransmissibilityEquations::betaFactor( 2.0, 100.0, 0.5, 0.25, 2.0 );
    EXPECT_NEAR( 2.0 * 10.0 * 0.0625, beta, 1.0e-12 );

    // Zero exponents leave only the inertial coefficient
    EXPECT_DOUBLE_EQ( 3.5, RigPerforationTransmissibilityEquations::betaFactor( 3.5, 123.0, 0.0, 0.3, 0.0 ) );
}

TEST( RigPerforationTransmissibilityEquations, BetaFactorNegativeExponents )
{
    // Typical non-Darcy correlation uses negative exponents: beta = C * Ke^-1.2 * phi^-0.5
    double beta = RigPerforationTransmissibilityEquations::betaFactor( 1.0e6, 100.0, -1.2, 0.25, -0.5 );
    EXPECT_NEAR( 1.0e6 * std::pow( 100.0, -1.2 ) * 2.0, beta, 1.0e-6 );

    // Unit permeability and porosity make the exponents irrelevant
    EXPECT_DOUBLE_EQ( 7.0, RigPerforationTransmissibilityEquations::betaFactor( 7.0, 1.0, -1.2, 1.0, -0.5 ) );

    // Linear in the inertial coefficient
    const double base = RigPerforationTransmissibilityEquations::betaFactor( 1.0, 50.0, -1.2, 0.2, -0.5 );
    EXPECT_NEAR( 3.0 * base, RigPerforationTransmissibilityEquations::betaFactor( 3.0, 50.0, -1.2, 0.2, -0.5 ), 1.0e-12 );
}

TEST( RigPerforationTransmissibilityEquations, DFactor )
{
    // D = alpha * beta * Ke / h * 1 / rw * rho / mu
    double d = RigPerforationTransmissibilityEquations::dFactor( 2.0, 3.0, 40.0, 5.0, 0.1, 0.8, 0.02 );
    EXPECT_NEAR( 2.0 * 3.0 * 8.0 * 10.0 * 40.0, d, 1.0e-9 );
}

TEST( RigPerforationTransmissibilityEquations, DFactorScaling )
{
    const double base = RigPerforationTransmissibilityEquations::dFactor( 1.0, 1.0, 10.0, 2.0, 0.1, 1.0, 1.0 );

    // Linear in unit constant, beta, permeability, density
    EXPECT_NEAR( 2.0 * base, RigPerforationTransmissibilityEquations::dFactor( 2.0, 1.0, 10.0, 2.0, 0.1, 1.0, 1.0 ), 1.0e-9 );
    EXPECT_NEAR( 2.0 * base, RigPerforationTransmissibilityEquations::dFactor( 1.0, 2.0, 10.0, 2.0, 0.1, 1.0, 1.0 ), 1.0e-9 );
    EXPECT_NEAR( 2.0 * base, RigPerforationTransmissibilityEquations::dFactor( 1.0, 1.0, 20.0, 2.0, 0.1, 1.0, 1.0 ), 1.0e-9 );
    EXPECT_NEAR( 2.0 * base, RigPerforationTransmissibilityEquations::dFactor( 1.0, 1.0, 10.0, 2.0, 0.1, 2.0, 1.0 ), 1.0e-9 );

    // Inversely proportional to length, well radius and viscosity
    EXPECT_NEAR( 0.5 * base, RigPerforationTransmissibilityEquations::dFactor( 1.0, 1.0, 10.0, 4.0, 0.1, 1.0, 1.0 ), 1.0e-9 );
    EXPECT_NEAR( 0.5 * base, RigPerforationTransmissibilityEquations::dFactor( 1.0, 1.0, 10.0, 2.0, 0.2, 1.0, 1.0 ), 1.0e-9 );
    EXPECT_NEAR( 0.5 * base, RigPerforationTransmissibilityEquations::dFactor( 1.0, 1.0, 10.0, 2.0, 0.1, 1.0, 2.0 ), 1.0e-9 );
}
