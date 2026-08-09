/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "ResultAccessors/RigAllGridCellsResultAccessor.h"

#include <cmath>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// The grid is only used by cellScalar(), the global index accessors read the result vector directly.
//--------------------------------------------------------------------------------------------------
TEST( RigAllGridCellsResultAccessorTest, CellScalarGlobIdxReturnsValueForValidIndex )
{
    std::vector<double> values = { 1.0, 2.0, 3.0 };

    RigAllGridCellsResultAccessor accessor( nullptr, &values );

    EXPECT_DOUBLE_EQ( 1.0, accessor.cellScalarGlobIdx( 0 ) );
    EXPECT_DOUBLE_EQ( 3.0, accessor.cellScalarGlobIdx( 2 ) );
}

//--------------------------------------------------------------------------------------------------
/// A cell index beyond the result vector must not throw. The accessor is called from OpenMP loops,
/// where an escaping exception terminates the application.
//--------------------------------------------------------------------------------------------------
TEST( RigAllGridCellsResultAccessorTest, CellScalarGlobIdxIsUndefinedForIndexBeyondResultVector )
{
    std::vector<double> values = { 1.0, 2.0, 3.0 };

    RigAllGridCellsResultAccessor accessor( nullptr, &values );

    EXPECT_NO_THROW( accessor.cellScalarGlobIdx( 3 ) );
    EXPECT_DOUBLE_EQ( HUGE_VAL, accessor.cellScalarGlobIdx( 3 ) );
    EXPECT_DOUBLE_EQ( HUGE_VAL, accessor.cellScalarGlobIdx( 1000 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigAllGridCellsResultAccessorTest, CellScalarGlobIdxIsUndefinedForEmptyResultVector )
{
    std::vector<double> values;

    RigAllGridCellsResultAccessor accessor( nullptr, &values );

    EXPECT_DOUBLE_EQ( HUGE_VAL, accessor.cellScalarGlobIdx( 0 ) );
}
