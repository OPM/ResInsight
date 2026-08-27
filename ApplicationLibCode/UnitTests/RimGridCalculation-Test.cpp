/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimGridCalculation.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, ReplaceInvalidValuesWithDefaultValue )
{
    const double        defaultValue = 0.0;
    std::vector<double> values =
        { 1.0, HUGE_VAL, 2.0, -HUGE_VAL, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -3.0, 0.0 };

    size_t replacedCount = RimGridCalculation::replaceInvalidValuesWithDefaultValue( defaultValue, values );

    EXPECT_EQ( 4u, replacedCount );

    const std::vector<double> expected = { 1.0, 0.0, 2.0, 0.0, 0.0, 0.0, -3.0, 0.0 };
    ASSERT_EQ( expected.size(), values.size() );
    for ( size_t i = 0; i < expected.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( expected[i], values[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGridCalculationTest, ReplaceInvalidValuesWithDefaultValueNoInvalidValues )
{
    const std::vector<double> original = { 1.0, 2.0, -3.0, 0.0 };

    std::vector<double> values        = original;
    size_t              replacedCount = RimGridCalculation::replaceInvalidValuesWithDefaultValue( 42.0, values );

    EXPECT_EQ( 0u, replacedCount );
    EXPECT_EQ( original, values );

    std::vector<double> emptyValues;
    EXPECT_EQ( 0u, RimGridCalculation::replaceInvalidValuesWithDefaultValue( 42.0, emptyValues ) );
}
