/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "gtest/gtest.h"

#include "RimWellLogCalculatedCurve.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWellLogCalculatedCurve, unionDepthValuesFromVectors_singleValuesInDepthVectors )
{
    // Depth vectors without duplicates
    const std::vector<double> depthValues1 = { 1.0, 2.0, 3.0, 4.0 };
    const std::vector<double> depthValues2 = { 2.0, 3.0, 4.0, 5.0 };

    // Expected duplicate occurrence of common depth values
    const std::vector<double> expectedUnionDepthValues = { 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0, 5.0 };

    // Call the function under test
    const std::vector<double> unionDepthValues = RimWellLogCalculatedCurve::unionDepthValuesFromVectors( depthValues1, depthValues2 );

    ASSERT_EQ( unionDepthValues, expectedUnionDepthValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWellLogCalculatedCurve, unionDepthValuesFromVectors_duplicateValuesInDepthVectors )
{
    // Depth vectors with duplicates
    const std::vector<double> depthValues1 = { 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0 };
    const std::vector<double> depthValues2 = { 2.0, 2.0, 3.0, 3.0, 4.0, 4.0, 5.0 };

    // Expected maximum 2 duplicate occurrences of common depth values
    const std::vector<double> expectedUnionDepthValues = { 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0, 5.0 };

    // Call the function under test
    const std::vector<double> unionDepthValues = RimWellLogCalculatedCurve::unionDepthValuesFromVectors( depthValues1, depthValues2 );

    ASSERT_EQ( unionDepthValues, expectedUnionDepthValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWellLogCalculatedCurve, calculateValue )
{
    ASSERT_DOUBLE_EQ( 7.0, RimWellLogCalculatedCurve::calculateValue( 5.0, 2.0, RimWellLogCalculatedCurve::Operators::ADD ) );
    ASSERT_DOUBLE_EQ( 3.0, RimWellLogCalculatedCurve::calculateValue( 5.0, 2.0, RimWellLogCalculatedCurve::Operators::SUBTRACT ) );
    ASSERT_DOUBLE_EQ( 2.5, RimWellLogCalculatedCurve::calculateValue( 5.0, 2.0, RimWellLogCalculatedCurve::Operators::DIVIDE ) );
    ASSERT_DOUBLE_EQ( 10.0, RimWellLogCalculatedCurve::calculateValue( 5.0, 2.0, RimWellLogCalculatedCurve::Operators::MULTIPLY ) );

    // Divide by zero
    ASSERT_DOUBLE_EQ( std::numeric_limits<double>::infinity(),
                      RimWellLogCalculatedCurve::calculateValue( 5.0, 0.0, RimWellLogCalculatedCurve::Operators::DIVIDE ) );
}
