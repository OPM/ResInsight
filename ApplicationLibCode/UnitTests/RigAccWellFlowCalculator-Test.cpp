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

#include "Well/RigAccWellFlowCalculator.h"
#include "Well/RigWellResultPoint.h"

//--------------------------------------------------------------------------------------------------
/// The well result points of the connections must be reported in the same order as the connection numbers
//--------------------------------------------------------------------------------------------------
TEST( RigAccWellFlowCalculatorTest, ConnectionWellResultPointsMatchConnectionNumbers )
{
    const std::vector<size_t> cellIndicesFromTop = { 10, 20, 30 };

    std::vector<cvf::Vec3d>         clCoords;
    std::vector<RigWellResultPoint> resultPoints;
    double                          z = 0.0;
    for ( size_t cellIndex : cellIndicesFromTop )
    {
        RigWellResultPoint resultPoint;
        resultPoint.setGridIndex( 0 );
        resultPoint.setGridCellIndex( cellIndex );
        resultPoint.setFlowData( 1.0, 1.0, 0.0, 0.0 );

        // Two centerline points per cell, entering and leaving the cell
        for ( int i = 0; i < 2; ++i )
        {
            clCoords.emplace_back( 0.0, 0.0, z );
            resultPoints.push_back( resultPoint );
            z -= 10.0;
        }
    }

    RigAccWellFlowCalculator calculator( { clCoords }, { resultPoints }, 0.0 );

    const std::vector<double>              connectionNumbers     = calculator.connectionNumbersFromTop( 0 );
    const std::vector<RigWellResultPoint>& connectionResultCells = calculator.connectionWellResultPoints( 0 );

    ASSERT_EQ( cellIndicesFromTop.size(), connectionNumbers.size() );
    ASSERT_EQ( connectionNumbers.size(), connectionResultCells.size() );

    for ( size_t i = 0; i < connectionNumbers.size(); ++i )
    {
        const size_t connectionIndexFromTop = static_cast<size_t>( connectionNumbers[i] ) - 1;
        EXPECT_EQ( cellIndicesFromTop[connectionIndexFromTop], connectionResultCells[i].cellIndex() );
    }
}
