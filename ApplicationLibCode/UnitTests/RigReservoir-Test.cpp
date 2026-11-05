

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
    cvf::ref<RigMainGrid> mainGridA  = new RigMainGrid;
    mainGridA->allocateNodes( 3 )[2] = cvf::Vec3d( 0, 0, 2 );
    mainGridA->modifiableNodes()[1]  = cvf::Vec3d( 0, 0, 1 );

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( nullptr );
    eclipseCase->setMainGrid( mainGridA.p() );

    RigGridManager gridCollection;
    gridCollection.addCase( eclipseCase.p() );

    cvf::ref<RigMainGrid> mainGridB    = new RigMainGrid;
    cvf::ref<RigMainGrid> existingGrid = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.isNull() );

    mainGridB->allocateNodes( 1 );
    existingGrid = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.isNull() );

    // Insert nodes in opposite direction
    auto appended = mainGridB->allocateNodes( 2 );
    appended[0]   = cvf::Vec3d( 0, 0, 2 );
    appended[1]   = cvf::Vec3d( 0, 0, 1 );
    existingGrid  = gridCollection.findEqualGrid( mainGridB.p() );
    EXPECT_TRUE( existingGrid.isNull() );

    // Overwrite to match the node structure of mainGridA
    std::swap( mainGridB->modifiableNodes()[1], mainGridB->modifiableNodes()[2] );
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
    EXPECT_EQ( 0u, mainGrid->nodes().size() );

    auto initial = mainGrid->allocateNodes( 2 );
    initial[0]   = cvf::Vec3d( 1, 2, 3 );
    initial[1]   = cvf::Vec3d( 4, 5, 6 );
    ASSERT_EQ( 2u, mainGrid->nodes().size() );
    EXPECT_EQ( cvf::Vec3d( 4, 5, 6 ), mainGrid->nodes()[1] );

    auto appended = mainGrid->allocateNodes( 1 );
    ASSERT_EQ( 1u, appended.size() );
    EXPECT_EQ( cvf::Vec3d::ZERO, appended[0] );
    appended[0] = cvf::Vec3d( 7, 8, 9 );
    ASSERT_EQ( 3u, mainGrid->nodes().size() );
    EXPECT_EQ( cvf::Vec3d( 1, 2, 3 ), mainGrid->nodes()[0] );
    EXPECT_EQ( cvf::Vec3d( 7, 8, 9 ), mainGrid->nodes()[2] );

    mainGrid->modifiableNodes()[0] = cvf::Vec3d( -1, -2, -3 );
    EXPECT_EQ( cvf::Vec3d( -1, -2, -3 ), mainGrid->nodes()[0] );

    mainGrid->clearNodes();
    EXPECT_EQ( 0u, mainGrid->nodes().size() );
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
