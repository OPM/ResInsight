/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
TEST(RigActiveCellInfo, BasicTest)
{
    RigActiveCellInfo rigActiveCellInfo;

    size_t globalActiveCellCount = 10;
    rigActiveCellInfo.setGlobalCellCount(globalActiveCellCount);

    for (size_t i = 0; i < globalActiveCellCount; i++)
    {
        EXPECT_TRUE(rigActiveCellInfo.activeIndexInMatrixModel(i) == cvf::UNDEFINED_SIZE_T);
        EXPECT_FALSE(rigActiveCellInfo.isActiveInMatrixModel(i));

        EXPECT_TRUE(rigActiveCellInfo.activeIndexInFractureModel(i) == cvf::UNDEFINED_SIZE_T);
        EXPECT_FALSE(rigActiveCellInfo.isActiveInFractureModel(i));
    }

    rigActiveCellInfo.setActiveIndexInMatrixModel(3, 1);
    EXPECT_TRUE(rigActiveCellInfo.activeIndexInMatrixModel(3) == 1);

    rigActiveCellInfo.setActiveIndexInFractureModel(9, 3);
    EXPECT_TRUE(rigActiveCellInfo.activeIndexInFractureModel(9) == 3);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigActiveCellInfo, GridCellCounts)
{
    {
        RigActiveCellInfo rigActiveCellInfo;
        rigActiveCellInfo.setGridCount(3);
        rigActiveCellInfo.setGridActiveCellCounts(0, 0, 0);
        rigActiveCellInfo.setGridActiveCellCounts(1, 1, 0);
        rigActiveCellInfo.setGridActiveCellCounts(2, 2, 0);
        rigActiveCellInfo.computeDerivedData();

        EXPECT_TRUE(rigActiveCellInfo.globalMatrixModelActiveCellCount() == 3);
        EXPECT_TRUE(rigActiveCellInfo.globalFractureModelActiveCellCount() == 0);
    }

    {
        RigActiveCellInfo rigActiveCellInfo;
        rigActiveCellInfo.setGridCount(3);
        rigActiveCellInfo.setGridActiveCellCounts(0, 0, 3);
        rigActiveCellInfo.setGridActiveCellCounts(1, 1, 4);
        rigActiveCellInfo.setGridActiveCellCounts(2, 2, 5);
        rigActiveCellInfo.computeDerivedData();

        EXPECT_TRUE(rigActiveCellInfo.globalMatrixModelActiveCellCount() == 3);
        EXPECT_TRUE(rigActiveCellInfo.globalFractureModelActiveCellCount() == 12);
    }


}

