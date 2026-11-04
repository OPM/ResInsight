

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

#include "RigEclipseCaseData.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigGridManager, BasicTest )
{
    cvf::ref<RigMainGrid> mainGridA = new RigMainGrid;

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( nullptr );
    eclipseCase->setMainGrid( mainGridA.p() );

    EXPECT_EQ( mainGridA->refCount(), 2 );

    RigGridManager gridCollection;
    gridCollection.addCase( eclipseCase.p() );
    EXPECT_EQ( mainGridA->refCount(), 2 );

    cvf::ref<RigMainGrid> mainGridB = mainGridA;
    EXPECT_EQ( mainGridA->refCount(), 3 );

    cvf::ref<RigMainGrid> existingGrid = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.notNull() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigGridManager, EqualTests )
{
    cvf::ref<RigMainGrid> mainGridA = new RigMainGrid;
    mainGridA->setNodes( { cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 0, 0, 1 ), cvf::Vec3d( 0, 0, 2 ) } );

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( nullptr );
    eclipseCase->setMainGrid( mainGridA.p() );

    RigGridManager gridCollection;
    gridCollection.addCase( eclipseCase.p() );

    cvf::ref<RigMainGrid> mainGridB    = new RigMainGrid;
    cvf::ref<RigMainGrid> existingGrid = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.isNull() );

    mainGridB->appendNodes( { cvf::Vec3d( 0, 0, 0 ) } );
    existingGrid = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.isNull() );

    // Insert nodes in opposite direction
    mainGridB->appendNodes( { cvf::Vec3d( 0, 0, 2 ), cvf::Vec3d( 0, 0, 1 ) } );
    existingGrid = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.isNull() );

    // Overwrite to match the node structure of mainGridA
    mainGridB->setNode( 1, cvf::Vec3d( 0, 0, 1 ) );
    mainGridB->setNode( 2, cvf::Vec3d( 0, 0, 2 ) );
    existingGrid = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.notNull() );
}

//--------------------------------------------------------------------------------------------------
/// The well cell arrays are cached, and must be recomputed if the grid changes size after the first
/// computation. A stale array is shorter than the cell count of the grid it is used with.
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseCaseDataTest, WellCellsInGridIsRecomputedWhenGridSizeChanges )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( nullptr );
    eclipseCase->setMainGrid( mainGrid.p() );

    const cvf::UByteArray* wellCells = eclipseCase->wellCellsInGrid( 0 );
    ASSERT_TRUE( wellCells != nullptr );
    EXPECT_EQ( mainGrid->cellCount(), wellCells->size() );

    // Grow the grid after the well cell arrays have been cached
    mainGrid->setCellCounts( cvf::Vec3st( 3, 3, 3 ) );

    wellCells = eclipseCase->wellCellsInGrid( 0 );
    ASSERT_TRUE( wellCells != nullptr );
    EXPECT_EQ( mainGrid->cellCount(), wellCells->size() );
}

//--------------------------------------------------------------------------------------------------
/// A pick on an intersection can report a grid index from another case than the one in the view
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridTest, GridByIndexOutOfRangeReturnsNull )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );

    EXPECT_EQ( mainGrid.p(), mainGrid->gridByIndex( 0 ) );
    EXPECT_EQ( nullptr, mainGrid->gridByIndex( 1 ) );
    EXPECT_EQ( nullptr, mainGrid->gridByIndex( cvf::UNDEFINED_SIZE_T ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridTest, NodeTableAccess )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    EXPECT_EQ( 0u, mainGrid->nodeCount() );

    mainGrid->setNodes( { cvf::Vec3d( 1, 2, 3 ), cvf::Vec3d( 4, 5, 6 ) } );
    ASSERT_EQ( 2u, mainGrid->nodeCount() );
    EXPECT_EQ( cvf::Vec3d( 4, 5, 6 ), mainGrid->node( 1 ) );

    mainGrid->appendNodes( { cvf::Vec3d( 7, 8, 9 ) } );
    ASSERT_EQ( 3u, mainGrid->nodeCount() );
    EXPECT_EQ( cvf::Vec3d( 1, 2, 3 ), mainGrid->node( 0 ) );
    EXPECT_EQ( cvf::Vec3d( 7, 8, 9 ), mainGrid->node( 2 ) );

    mainGrid->setNode( 0, cvf::Vec3d( -1, -2, -3 ) );
    EXPECT_EQ( cvf::Vec3d( -1, -2, -3 ), mainGrid->node( 0 ) );

    mainGrid->transformNodes( cvf::Mat4d::fromTranslation( cvf::Vec3d( 10, 0, 0 ) ) );
    EXPECT_EQ( cvf::Vec3d( 9, -2, -3 ), mainGrid->node( 0 ) );
    EXPECT_EQ( cvf::Vec3d( 17, 8, 9 ), mainGrid->node( 2 ) );

    RigMainGridNodes view( *mainGrid );
    EXPECT_EQ( 3u, view.size() );
    EXPECT_EQ( cvf::Vec3d( 14, 5, 6 ), view[1] );

    mainGrid->setNodes( {} );
    EXPECT_EQ( 0u, mainGrid->nodeCount() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridTest, RemoveDuplicateNodes )
{
    RigReservoirBuilder builder;
    builder.setIJKCount( cvf::Vec3st( 3, 2, 2 ) );
    builder.setWorldCoordinates( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( 3.0, 2.0, -2.0 ) );

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( nullptr );
    builder.createGridsAndCells( eclipseCase.p() );
    RigMainGrid* mainGrid = eclipseCase->mainGrid();

    const size_t cellCount = mainGrid->cellCount();
    ASSERT_EQ( 12u, cellCount );
    ASSERT_EQ( 8u * cellCount, mainGrid->nodeCount() );

    std::vector<std::array<cvf::Vec3d, 8>> cornersBefore;
    for ( size_t cellIndex = 0; cellIndex < cellCount; cellIndex++ )
    {
        cornersBefore.push_back( mainGrid->cellCornerVertices( cellIndex ) );
    }

    // A 3 x 2 x 2 grid without faults has 4 x 3 x 3 unique corner nodes
    EXPECT_EQ( 96u - 36u, mainGrid->removeDuplicateNodes() );
    EXPECT_EQ( 36u, mainGrid->nodeCount() );
    EXPECT_EQ( 0u, mainGrid->removeDuplicateNodes() );

    for ( size_t cellIndex = 0; cellIndex < cellCount; cellIndex++ )
    {
        EXPECT_EQ( cornersBefore[cellIndex], mainGrid->cellCornerVertices( cellIndex ) );
        for ( size_t nodeIndex : mainGrid->cell( cellIndex ).cornerIndices() )
        {
            EXPECT_LT( nodeIndex, mainGrid->nodeCount() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridTest, RemoveDuplicateNodesKeepsFaultedNodesSeparate )
{
    RigReservoirBuilder builder;
    builder.setIJKCount( cvf::Vec3st( 2, 1, 1 ) );
    builder.setWorldCoordinates( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( 2.0, 1.0, -1.0 ) );

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( nullptr );
    builder.createGridsAndCells( eclipseCase.p() );
    RigMainGrid* mainGrid = eclipseCase->mainGrid();

    // Shift the second cell down by half a cell, so no corners coincide with the first cell
    for ( size_t nodeIndex : mainGrid->cell( 1 ).cornerIndices() )
    {
        mainGrid->setNode( nodeIndex, mainGrid->node( nodeIndex ) - cvf::Vec3d( 0.0, 0.0, 0.5 ) );
    }

    EXPECT_EQ( 0u, mainGrid->removeDuplicateNodes() );
    EXPECT_EQ( 16u, mainGrid->nodeCount() );

    // Changing a node makes deduplication run again
    for ( size_t nodeIndex : mainGrid->cell( 1 ).cornerIndices() )
    {
        mainGrid->setNode( nodeIndex, mainGrid->node( nodeIndex ) + cvf::Vec3d( 0.0, 0.0, 0.5 ) );
    }

    EXPECT_EQ( 4u, mainGrid->removeDuplicateNodes() );
    EXPECT_EQ( 12u, mainGrid->nodeCount() );
}

/*

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, BasicTest)
{
    cvf::ref<RigSingleWellResultsData> wellCellTimeHistory = new RigSingleWellResultsData;

    QDateTime wellStartTime = QDateTime::currentDateTime();

    int wellTimeStepCount = 5;
    wellCellTimeHistory->m_wellCellsTimeSteps.resize(wellTimeStepCount);

    int i;
    for (i = 0; i < wellTimeStepCount; i++)
    {
        wellCellTimeHistory->m_wellCellsTimeSteps[i].m_timestamp = QDateTime(wellStartTime).addYears(i);
    }

    int resultTimeStepCount = 2 * wellTimeStepCount;
    std::vector<QDateTime> resultTimes;
    for (i = 0; i < resultTimeStepCount; i++)
    {
        resultTimes.push_back(QDateTime(wellStartTime).addMonths(i * 6));
    }

    wellCellTimeHistory->computeMappingFromResultTimeIndicesToWellTimeIndices(resultTimes);

    for (i = 0; i < resultTimeStepCount; i++)
    {
        qDebug() << "Index" << i << "is " << wellCellTimeHistory->m_resultTimeStepIndexToWellTimeStepIndex[i];

        const RigWellResultFrame& wellCells =
wellCellTimeHistory->wellResultFrame(wellCellTimeHistory->m_resultTimeStepIndexToWellTimeStepIndex[i]); qDebug() <<
wellCells.m_timestamp;
    }

}


*/
