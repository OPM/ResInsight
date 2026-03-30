/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigActiveCellInfo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigActiveCellInfo, BasicTest )
{
    RigActiveCellInfo rigActiveCellInfo;

    size_t globalActiveCellCount = 10;
    rigActiveCellInfo.setReservoirCellCount( globalActiveCellCount );

    for ( size_t i = 0; i < globalActiveCellCount; i++ )
    {
        EXPECT_TRUE( rigActiveCellInfo.cellResultIndex( ReservoirCellIndex( i ) ).value() == cvf::UNDEFINED_SIZE_T );
        EXPECT_FALSE( rigActiveCellInfo.isActive( ReservoirCellIndex( i ) ) );
    }

    rigActiveCellInfo.setCellResultIndex( ReservoirCellIndex( 3 ), ActiveCellIndex( 1 ) );
    EXPECT_TRUE( rigActiveCellInfo.cellResultIndex( ReservoirCellIndex( 3 ) ).value() == 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigActiveCellInfo, GridCellCounts )
{
    {
        RigActiveCellInfo rigActiveCellInfo;
        rigActiveCellInfo.setGridCount( 3 );
        rigActiveCellInfo.setGridActiveCellCounts( 0, 0 );
        rigActiveCellInfo.setGridActiveCellCounts( 1, 1 );
        rigActiveCellInfo.setGridActiveCellCounts( 2, 2 );
        rigActiveCellInfo.computeDerivedData();

        EXPECT_TRUE( rigActiveCellInfo.reservoirActiveCellCount() == 3 );
    }

    {
        RigActiveCellInfo rigActiveCellInfo;
        rigActiveCellInfo.setGridCount( 3 );
        rigActiveCellInfo.setGridActiveCellCounts( 0, 3 );
        rigActiveCellInfo.setGridActiveCellCounts( 1, 4 );
        rigActiveCellInfo.setGridActiveCellCounts( 2, 5 );
        rigActiveCellInfo.computeDerivedData();

        EXPECT_TRUE( rigActiveCellInfo.reservoirActiveCellCount() == 12 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigActiveCellInfo, IJKBoundingBox )
{
    RigActiveCellInfo rigActiveCellInfo;

    // Test initial state
    const auto& bbox = rigActiveCellInfo.ijkBoundingBox();
    EXPECT_EQ( caf::VecIjk0( 0, 0, 0 ), bbox.min() );
    EXPECT_EQ( caf::VecIjk0( 0, 0, 0 ), bbox.max() );
    EXPECT_TRUE( bbox.isValid() );

    // Test setting bounding box
    rigActiveCellInfo.setIjkBoundingBox( RigBoundingBoxIjk<caf::VecIjk0>( caf::VecIjk0( 5, 10, 15 ), caf::VecIjk0( 20, 30, 40 ) ) );
    const auto& bbox2 = rigActiveCellInfo.ijkBoundingBox();
    EXPECT_EQ( caf::VecIjk0( 5, 10, 15 ), bbox2.min() );
    EXPECT_EQ( caf::VecIjk0( 20, 30, 40 ), bbox2.max() );

    // Test clear resets bounding box
    rigActiveCellInfo.clear();
    const auto& bbox3 = rigActiveCellInfo.ijkBoundingBox();
    EXPECT_EQ( caf::VecIjk0( 0, 0, 0 ), bbox3.min() );
    EXPECT_EQ( caf::VecIjk0( 0, 0, 0 ), bbox3.max() );
}
