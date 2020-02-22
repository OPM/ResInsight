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
        EXPECT_TRUE( rigActiveCellInfo.cellResultIndex( i ) == cvf::UNDEFINED_SIZE_T );
        EXPECT_FALSE( rigActiveCellInfo.isActive( i ) );
    }

    rigActiveCellInfo.setCellResultIndex( 3, 1 );
    EXPECT_TRUE( rigActiveCellInfo.cellResultIndex( 3 ) == 1 );
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
