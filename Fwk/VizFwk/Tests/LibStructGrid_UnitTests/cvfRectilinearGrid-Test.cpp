//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfRectilinearGrid.h"

#include "gtest/gtest.h"

#include <set>

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, AllocateGrid)
{
    RectilinearGrid grid;
    EXPECT_EQ(0, grid.gridPointCountI());
    EXPECT_EQ(0, grid.gridPointCountJ());
    EXPECT_EQ(0, grid.gridPointCountK());
    EXPECT_EQ(0, (cvf::uint)grid.coordinatesI().size());
    EXPECT_EQ(0, (cvf::uint)grid.coordinatesJ().size());
    EXPECT_EQ(0, (cvf::uint)grid.coordinatesK().size());

    grid.allocateGrid(10, 20, 30);
    EXPECT_EQ(10, grid.gridPointCountI());
    EXPECT_EQ(20, grid.gridPointCountJ());
    EXPECT_EQ(30, grid.gridPointCountK());
    EXPECT_EQ(10, (cvf::uint)grid.coordinatesI().size());
    EXPECT_EQ(20, (cvf::uint)grid.coordinatesJ().size());
    EXPECT_EQ(30, (cvf::uint)grid.coordinatesK().size());

    grid.allocateGrid(2, 2, 2);
    EXPECT_EQ(2, grid.gridPointCountI());
    EXPECT_EQ(2, grid.gridPointCountJ());
    EXPECT_EQ(2, grid.gridPointCountK());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(RectilinearGridDeathTest, AllocateGridWithIllegalSize)
{
    RectilinearGrid grid;
    EXPECT_DEATH(grid.allocateGrid(0, 1, 2), "Assertion");
    EXPECT_DEATH(grid.allocateGrid(2, 1, 2), "Assertion");
    EXPECT_DEATH(grid.allocateGrid(2, 2, 1), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, IsLegalGridPoint)
{
    RectilinearGrid grid;
    grid.allocateGrid(2, 3, 4);

    EXPECT_TRUE(grid.isLegalGridPoint(0, 0, 0));
    EXPECT_TRUE(grid.isLegalGridPoint(1, 2, 3));

    EXPECT_FALSE(grid.isLegalGridPoint(0, 0, 4));
    EXPECT_FALSE(grid.isLegalGridPoint(0, 3, 0));
    EXPECT_FALSE(grid.isLegalGridPoint(2, 0, 0));
    EXPECT_FALSE(grid.isLegalGridPoint(3, 2, 1));

    EXPECT_FALSE(grid.isLegalGridPoint(static_cast<cvf::uint>(-1), 0, 0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, CellCounts)
{
    RectilinearGrid grid;
    EXPECT_EQ(0, grid.cellCountI());
    EXPECT_EQ(0, grid.cellCountJ());
    EXPECT_EQ(0, grid.cellCountK());
    EXPECT_EQ(0, (cvf::uint)grid.cellCount());

    grid.allocateGrid(2, 2, 2);
    EXPECT_EQ(1, grid.cellCountI());
    EXPECT_EQ(1, grid.cellCountJ());
    EXPECT_EQ(1, grid.cellCountK());
    EXPECT_EQ(1, (cvf::uint)grid.cellCount());

    grid.allocateGrid(11, 21, 31);
    EXPECT_EQ(10, grid.cellCountI());
    EXPECT_EQ(20, grid.cellCountJ());
    EXPECT_EQ(30, grid.cellCountK());
    EXPECT_EQ(6000, (cvf::uint)grid.cellCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, IsLegalCell)
{
    RectilinearGrid grid;
    grid.allocateGrid(2, 3, 4);

    EXPECT_TRUE(grid.isLegalCell(0, 0, 0));
    EXPECT_TRUE(grid.isLegalCell(0, 1, 2));

    EXPECT_FALSE(grid.isLegalCell(0, 0, 3));
    EXPECT_FALSE(grid.isLegalCell(0, 2, 0));
    EXPECT_FALSE(grid.isLegalCell(1, 0, 0));
    EXPECT_FALSE(grid.isLegalCell(2, 1, 0));

    EXPECT_FALSE(grid.isLegalCell(static_cast<cvf::uint>(-1), 0, 0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, Coordinates)
{
    RectilinearGrid grid;
    grid.allocateGrid(2, 3, 4);
    EXPECT_EQ(2u, grid.coordinatesI().size());
    EXPECT_EQ(3u, grid.coordinatesJ().size());
    EXPECT_EQ(4u, grid.coordinatesK().size());

    DoubleArray iVals;
    iVals.reserve(2);
    iVals.add(1.0);
    iVals.add(2.0);

    DoubleArray jVals;
    jVals.reserve(3);
    jVals.add(10.0);
    jVals.add(20.0);
    jVals.add(30.0);

    DoubleArray kVals;
    kVals.reserve(4);
    kVals.add(100.0);
    kVals.add(200.0);
    kVals.add(300.0);
    kVals.add(400.0);

    grid.setCoordinatesI(iVals);
    grid.setCoordinatesJ(jVals);
    grid.setCoordinatesK(kVals);
    EXPECT_EQ(2u, grid.coordinatesI().size());
    EXPECT_EQ(3u, grid.coordinatesJ().size());
    EXPECT_EQ(4u, grid.coordinatesK().size());

    EXPECT_EQ(1.0, grid.coordinatesI()[0]);
    EXPECT_EQ(2.0, grid.coordinatesI()[1]);
    EXPECT_EQ(10.0, grid.coordinatesJ()[0]);
    EXPECT_EQ(30.0, grid.coordinatesJ()[2]);
    EXPECT_EQ(100.0, grid.coordinatesK()[0]);
    EXPECT_EQ(400.0, grid.coordinatesK()[3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, CoordinatesToRegularGrid)
{
    RectilinearGrid grid;
    grid.allocateGrid(3, 3, 3);

    grid.setCoordinatesToRegularGrid(Vec3d(10, 20, 30), Vec3d(2, 3, 4));
    EXPECT_EQ(3u, grid.coordinatesI().size());
    EXPECT_EQ(3u, grid.coordinatesJ().size());
    EXPECT_EQ(3u, grid.coordinatesK().size());

    EXPECT_EQ(10.0, grid.coordinatesI()[0]);
    EXPECT_EQ(12.0, grid.coordinatesI()[1]);
    EXPECT_EQ(14.0, grid.coordinatesI()[2]);

    EXPECT_EQ(20.0, grid.coordinatesJ()[0]);
    EXPECT_EQ(23.0, grid.coordinatesJ()[1]);
    EXPECT_EQ(26.0, grid.coordinatesJ()[2]);

    EXPECT_EQ(30.0, grid.coordinatesK()[0]);
    EXPECT_EQ(34.0, grid.coordinatesK()[1]);
    EXPECT_EQ(38.0, grid.coordinatesK()[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, MinMaxCoordinates)
{
    RectilinearGrid grid;
    grid.allocateGrid(3, 3, 3);
    grid.setCoordinatesToRegularGrid(Vec3d(10, 20, 30), Vec3d(2, 3, 4));

    Vec3d cMin = grid.minCoordinate();
    EXPECT_EQ(10, cMin.x());
    EXPECT_EQ(20, cMin.y());
    EXPECT_EQ(30, cMin.z());

    Vec3d cMax = grid.maxCoordinate();
    EXPECT_EQ(14, cMax.x());
    EXPECT_EQ(26, cMax.y());
    EXPECT_EQ(38, cMax.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, CellIndexFromIJK)
{
    RectilinearGrid grid;
    grid.allocateGrid(3, 4, 5);
    ASSERT_EQ(24, (cvf::uint)grid.cellCount());

    EXPECT_EQ(0, (cvf::uint)grid.cellIndexFromIJK(0, 0, 0));
    EXPECT_EQ(1, (cvf::uint)grid.cellIndexFromIJK(1, 0, 0));
    EXPECT_EQ(4, (cvf::uint)grid.cellIndexFromIJK(0, 2, 0));
    EXPECT_EQ(5, (cvf::uint)grid.cellIndexFromIJK(1, 2, 0));

    EXPECT_EQ(6,  (cvf::uint)grid.cellIndexFromIJK(0, 0, 1));
    EXPECT_EQ(7,  (cvf::uint)grid.cellIndexFromIJK(1, 0, 1));
    EXPECT_EQ(10, (cvf::uint)grid.cellIndexFromIJK(0, 2, 1));
    EXPECT_EQ(11, (cvf::uint)grid.cellIndexFromIJK(1, 2, 1));

    EXPECT_EQ(18, (cvf::uint)grid.cellIndexFromIJK(0, 0, 3));
    EXPECT_EQ(23, (cvf::uint)grid.cellIndexFromIJK(1, 2, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, IJKFromCellIndex)
{
    RectilinearGrid grid;
    grid.allocateGrid(3, 4, 5);
    ASSERT_EQ(24, (cvf::uint)grid.cellCount());

    Vector3<cvf::uint> ijk;
    grid.ijkFromCellIndex(0, &ijk.x(), &ijk.y(), &ijk.z());    EXPECT_TRUE(ijk == Vector3<cvf::uint>(0, 0, 0));
    grid.ijkFromCellIndex(1, &ijk.x(), &ijk.y(), &ijk.z());    EXPECT_TRUE(ijk == Vector3<cvf::uint>(1, 0, 0));
    grid.ijkFromCellIndex(4, &ijk.x(), &ijk.y(), &ijk.z());    EXPECT_TRUE(ijk == Vector3<cvf::uint>(0, 2, 0));
    grid.ijkFromCellIndex(5, &ijk.x(), &ijk.y(), &ijk.z());    EXPECT_TRUE(ijk == Vector3<cvf::uint>(1, 2, 0));

    grid.ijkFromCellIndex(6,  &ijk.x(), &ijk.y(), &ijk.z());   EXPECT_TRUE(ijk == Vector3<cvf::uint>(0, 0, 1));
    grid.ijkFromCellIndex(7,  &ijk.x(), &ijk.y(), &ijk.z());   EXPECT_TRUE(ijk == Vector3<cvf::uint>(1, 0, 1));
    grid.ijkFromCellIndex(10, &ijk.x(), &ijk.y(), &ijk.z());   EXPECT_TRUE(ijk == Vector3<cvf::uint>(0, 2, 1));
    grid.ijkFromCellIndex(11, &ijk.x(), &ijk.y(), &ijk.z());   EXPECT_TRUE(ijk == Vector3<cvf::uint>(1, 2, 1));

    grid.ijkFromCellIndex(18, &ijk.x(), &ijk.y(), &ijk.z());   EXPECT_TRUE(ijk == Vector3<cvf::uint>(0, 0, 3));
    grid.ijkFromCellIndex(23, &ijk.x(), &ijk.y(), &ijk.z());   EXPECT_TRUE(ijk == Vector3<cvf::uint>(1, 2, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, CellNeighbor)
{
    // 1 cell, no neighbors
    {
        RectilinearGrid grid;
        grid.allocateGrid(2, 2, 2);
        ASSERT_EQ(1, (cvf::uint)grid.cellCount());
    
        size_t n;
        EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::BOTTOM,  &n));
        EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::TOP,     &n));
        EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::FRONT,   &n));
        EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::RIGHT,   &n));
        EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::BACK,    &n));
        EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::LEFT,    &n));
    }

    // 3x3x3 cells
    {
        RectilinearGrid grid;
        grid.allocateGrid(4, 4, 4);
        ASSERT_EQ(27, (cvf::uint)grid.cellCount());

        {
            size_t n;
            EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::BOTTOM,  &n));
            EXPECT_TRUE (grid.cellNeighbor(0, RectilinearGrid::TOP,     &n));    EXPECT_EQ(9, n);
            EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::FRONT,   &n));
            EXPECT_TRUE (grid.cellNeighbor(0, RectilinearGrid::RIGHT,   &n));    EXPECT_EQ(1, n);
            EXPECT_TRUE (grid.cellNeighbor(0, RectilinearGrid::BACK,    &n));    EXPECT_EQ(3, n);
            EXPECT_FALSE(grid.cellNeighbor(0, RectilinearGrid::LEFT,    &n));
        }

        {
            size_t n;
            EXPECT_TRUE (grid.cellNeighbor(26, RectilinearGrid::BOTTOM,  &n));    EXPECT_EQ(17, n);
            EXPECT_FALSE(grid.cellNeighbor(26, RectilinearGrid::TOP,     &n));
            EXPECT_TRUE (grid.cellNeighbor(26, RectilinearGrid::FRONT,   &n));    EXPECT_EQ(23, n);
            EXPECT_FALSE(grid.cellNeighbor(26, RectilinearGrid::RIGHT,   &n));
            EXPECT_FALSE(grid.cellNeighbor(26, RectilinearGrid::BACK,    &n));
            EXPECT_TRUE (grid.cellNeighbor(26, RectilinearGrid::LEFT,    &n));    EXPECT_EQ(25, n);
        }

        // Center cell
        {
            size_t n;
            EXPECT_TRUE (grid.cellNeighbor(13, RectilinearGrid::BOTTOM,  &n));    EXPECT_EQ(4, n);
            EXPECT_TRUE (grid.cellNeighbor(13, RectilinearGrid::TOP,     &n));    EXPECT_EQ(22, n);
            EXPECT_TRUE (grid.cellNeighbor(13, RectilinearGrid::FRONT,   &n));    EXPECT_EQ(10, n);
            EXPECT_TRUE (grid.cellNeighbor(13, RectilinearGrid::RIGHT,   &n));    EXPECT_EQ(14, n);
            EXPECT_TRUE (grid.cellNeighbor(13, RectilinearGrid::BACK,    &n));    EXPECT_EQ(16, n);
            EXPECT_TRUE (grid.cellNeighbor(13, RectilinearGrid::LEFT,    &n));    EXPECT_EQ(12, n);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, GridPointNeighborCells1)
{
    RectilinearGrid grid;
    grid.allocateGrid(4, 4, 2);
    ASSERT_EQ(9, (cvf::uint)grid.cellCount());

    // Cell indices
    // *----*----*----*
    // |  6 |  7 |  8 |
    // *----*----*----*
    // |  3 |  4 |  5 |
    // *----*----*----*  j
    // |  0 |  1 |  2 |  |
    // *----*----*----*  *--i
    // 0    1    2    3

    size_t cellIndices[8];

    {
        cvf::uint numCells = grid.gridPointNeighborCells(0, 0, 0, cellIndices);
        ASSERT_EQ(1, numCells);

        std::set<size_t> cellSet(cellIndices, cellIndices + numCells);
        EXPECT_TRUE(cellSet.find(0) != cellSet.end());
    }

    {
        cvf::uint numCells = grid.gridPointNeighborCells(3, 0, 0, cellIndices);
        ASSERT_EQ(1, numCells);

        std::set<size_t> cellSet(cellIndices, cellIndices + numCells);
        EXPECT_TRUE(cellSet.find(2) != cellSet.end());
    }

    {
        cvf::uint numCells = grid.gridPointNeighborCells(3, 3, 1, cellIndices);
        ASSERT_EQ(1, numCells);

        std::set<size_t> cellSet(cellIndices, cellIndices + numCells);
        EXPECT_TRUE(cellSet.find(8) != cellSet.end());
    }

    {
        cvf::uint numCells = grid.gridPointNeighborCells(1, 0, 0, cellIndices);
        ASSERT_EQ(2, numCells);

        std::set<size_t> cellSet(cellIndices, cellIndices + numCells);
        EXPECT_TRUE(cellSet.find(0) != cellSet.end());
        EXPECT_TRUE(cellSet.find(1) != cellSet.end());
    }

    {
        cvf::uint numCells = grid.gridPointNeighborCells(2, 2, 1, cellIndices);
        ASSERT_EQ(4, numCells);

        std::set<size_t> cellSet(cellIndices, cellIndices + numCells);
        EXPECT_TRUE(cellSet.find(4) != cellSet.end());
        EXPECT_TRUE(cellSet.find(5) != cellSet.end());
        EXPECT_TRUE(cellSet.find(7) != cellSet.end());
        EXPECT_TRUE(cellSet.find(8) != cellSet.end());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, GridPointNeighborCells2)
{
    RectilinearGrid grid;
    grid.allocateGrid(3, 3, 3);
    ASSERT_EQ(8, (cvf::uint)grid.cellCount());

    // Cell indices
    // *----*----*   *----*----*
    // |  2 |  3 |   |  6 |  7 |
    // *----*----*   *----*----*  j
    // |  0 |  1 |   |  4 |  5 |  |
    // *----*----*   *----*----*  *--i
    //   cellK=0       cellK=1

    size_t cellIndices[8];

    {
        cvf::uint numCells = grid.gridPointNeighborCells(0, 0, 1, cellIndices);
        ASSERT_EQ(2, numCells);

        std::set<size_t> cellSet(cellIndices, cellIndices + numCells);
        EXPECT_TRUE(cellSet.find(0) != cellSet.end());
        EXPECT_TRUE(cellSet.find(4) != cellSet.end());
    }

    {
        cvf::uint numCells = grid.gridPointNeighborCells(1, 1, 1, cellIndices);
        ASSERT_EQ(8, numCells);

        std::set<size_t> cellSet(cellIndices, cellIndices + numCells);
        EXPECT_TRUE(cellSet.find(0) != cellSet.end());
        EXPECT_TRUE(cellSet.find(1) != cellSet.end());
        EXPECT_TRUE(cellSet.find(2) != cellSet.end());
        EXPECT_TRUE(cellSet.find(3) != cellSet.end());
        EXPECT_TRUE(cellSet.find(4) != cellSet.end());
        EXPECT_TRUE(cellSet.find(5) != cellSet.end());
        EXPECT_TRUE(cellSet.find(6) != cellSet.end());
        EXPECT_TRUE(cellSet.find(7) != cellSet.end());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, GridPointScalar)
{
    RectilinearGrid grid;
    grid.allocateGrid(3, 3, 2);
    ASSERT_EQ(4, (cvf::uint)grid.cellCount());

    // Cell scalars
    // *----*----* 
    // |  3 |  4 | 
    // *----*----*  j
    // |  1 |  2 |  |
    // *----*----*  *--i
    DoubleArray* sclArr = new DoubleArray;
    sclArr->reserve(4);
    sclArr->add(1.0);
    sclArr->add(2.0);
    sclArr->add(3.0);
    sclArr->add(4.0);

    grid.addScalarSet(sclArr);

    EXPECT_DOUBLE_EQ(1.0, grid.gridPointScalar(0, 0, 0, 0));
    EXPECT_DOUBLE_EQ(4.0, grid.gridPointScalar(0, 2, 2, 0));
                                                            
    EXPECT_DOUBLE_EQ(1.5, grid.gridPointScalar(0, 1, 0, 0));
    EXPECT_DOUBLE_EQ(2.0, grid.gridPointScalar(0, 0, 1, 0));
                                                            
    EXPECT_DOUBLE_EQ(2.5, grid.gridPointScalar(0, 1, 1, 1));

    ref<DoubleArray> avrRes = grid.computeGridPointScalarSet(0);
    EXPECT_EQ(18, avrRes->size() );
    EXPECT_DOUBLE_EQ(1.0, avrRes->get(0 + 0*3 +  0*3*3));
    EXPECT_DOUBLE_EQ(4.0, avrRes->get(2 + 2*3 +  0*3*3));
    EXPECT_DOUBLE_EQ(1.5, avrRes->get(1 + 0*3 +  0*3*3));
    EXPECT_DOUBLE_EQ(2.0, avrRes->get(0 + 1*3 +  0*3*3));
    EXPECT_DOUBLE_EQ(2.5, avrRes->get(1 + 1*3 +  1*3*3));

    (*avrRes.p())[(grid.gridPointIndexFromIJK(0, 0, 0))] = 3.3;
    (*avrRes.p())[(grid.gridPointIndexFromIJK(2, 2, 0))] = 4.4;
    (*avrRes.p())[(grid.gridPointIndexFromIJK(1, 0, 0))] = 5.5;
    (*avrRes.p())[(grid.gridPointIndexFromIJK(0, 1, 0))] = 6.6;
    (*avrRes.p())[(grid.gridPointIndexFromIJK(1, 1, 1))] = 7.7;

    grid.setGridPointScalarSet(0, avrRes.p());

    EXPECT_DOUBLE_EQ(3.3, grid.gridPointScalar(0, 0, 0, 0));
    EXPECT_DOUBLE_EQ(4.4, grid.gridPointScalar(0, 2, 2, 0));
    EXPECT_DOUBLE_EQ(5.5, grid.gridPointScalar(0, 1, 0, 0));
    EXPECT_DOUBLE_EQ(6.6, grid.gridPointScalar(0, 0, 1, 0));
    EXPECT_DOUBLE_EQ(7.7, grid.gridPointScalar(0, 1, 1, 1));

}                                      

/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, PointScalar)
{
    RectilinearGrid grid;
    grid.allocateGrid(3, 3, 2);
    ASSERT_EQ(4, (cvf::uint)grid.cellCount());

    // Cell scalars
    // *----*----* 
    // |  3 |  4 | 
    // *----*----*  j
    // |  1 |  2 |  |
    // *----*----*  *--i
    DoubleArray iVals;
    iVals.reserve(3);
    iVals.add(0.0);
    iVals.add(2.0);
    iVals.add(4.0);


    DoubleArray jVals;
    jVals.reserve(3);
    jVals.add(0.0);
    jVals.add(2.0);
    jVals.add(4.0);

    DoubleArray kVals;
    kVals.reserve(2);
    kVals.add(0.0);
    kVals.add(2.0);

    grid.setCoordinatesI(iVals);
    grid.setCoordinatesJ(jVals);
    grid.setCoordinatesK(kVals);

    DoubleArray* sclArr = new DoubleArray;
    sclArr->reserve(4);
    sclArr->add(1.0);
    sclArr->add(2.0);
    sclArr->add(3.0);
    sclArr->add(4.0);

    grid.addScalarSet(sclArr);

    double value;
    grid.pointScalar(0, Vec3d(0, 0, 0), value);
    EXPECT_DOUBLE_EQ(1.0, value);
    grid.pointScalar(0, Vec3d(1, 1, 1), value);
    EXPECT_DOUBLE_EQ(1.75, value);



}                                      
*/

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectilinearGrid, FilteredVectorResult)
{
    RectilinearGrid grid;
    grid.allocateGrid(7, 6, 5);
    
    size_t cellCount = grid.cellCount();

    ref<Vec3dArray> vectorResult = new Vec3dArray;
    vectorResult->reserve(cellCount);
    
    size_t i; 
    for (i = 0; i < cellCount; i++)
    {
        vectorResult->add(Vec3d(static_cast<double>(i), static_cast<double>(i), static_cast<double>(i)));
    }

    grid.addVectorSet(vectorResult.p());
    EXPECT_EQ(1, grid.vectorSetCount());

    {
        ref<Vec3dArray> filteredPositions = new Vec3dArray;
        ref<Vec3dArray> filteredVectorResult = new Vec3dArray;
    
        grid.filteredCellCenterResultVectors(*filteredPositions, *filteredVectorResult, 0, 0, 0);
        EXPECT_EQ(120, filteredPositions->size());
        EXPECT_EQ(120, filteredVectorResult->size());
    }

    {
        ref<Vec3dArray> filteredPositions = new Vec3dArray;
        ref<Vec3dArray> filteredVectorResult = new Vec3dArray;

        grid.filteredCellCenterResultVectors(*filteredPositions, *filteredVectorResult, 0, 1, 0);
        EXPECT_EQ(60, filteredPositions->size());
        EXPECT_EQ(60, filteredVectorResult->size());
    }

    {
        ref<Vec3dArray> filteredPositions = new Vec3dArray;
        ref<Vec3dArray> filteredVectorResult = new Vec3dArray;

        grid.filteredCellCenterResultVectors(*filteredPositions, *filteredVectorResult, 0, 2, 0);
        EXPECT_EQ(40, filteredPositions->size());
        EXPECT_EQ(40, filteredVectorResult->size());
    }

    {
        ref<Vec3dArray> filteredPositions = new Vec3dArray;
        ref<Vec3dArray> filteredVectorResult = new Vec3dArray;

        double length = Vec3d(100, 100, 100).length();

        grid.filteredCellCenterResultVectors(*filteredPositions, *filteredVectorResult, 0, 0, length);
        EXPECT_EQ(20, filteredPositions->size());
        EXPECT_EQ(20, filteredVectorResult->size());
    }

}                                      
