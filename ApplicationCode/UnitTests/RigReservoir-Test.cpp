

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
TEST(RigGridManager, BasicTest)
{
    cvf::ref<RigMainGrid> mainGridA = new RigMainGrid;

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData;
    eclipseCase->setMainGrid(mainGridA.p());

    EXPECT_EQ(mainGridA->refCount(), 2);

    RigGridManager gridCollection;
    gridCollection.addCase(eclipseCase.p());
    EXPECT_EQ(mainGridA->refCount(), 2);

    cvf::ref<RigMainGrid> mainGridB = mainGridA;
    EXPECT_EQ(mainGridA->refCount(), 3);

    cvf::ref<RigMainGrid> existingGrid = gridCollection.findEqualGrid(mainGridB.p());
    EXPECT_TRUE(existingGrid.notNull());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigGridManager, EqualTests)
{
    cvf::ref<RigMainGrid> mainGridA = new RigMainGrid;
    mainGridA->nodes().push_back(cvf::Vec3d(0, 0, 0));
    mainGridA->nodes().push_back(cvf::Vec3d(0, 0, 1));
    mainGridA->nodes().push_back(cvf::Vec3d(0, 0, 2));

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData;
    eclipseCase->setMainGrid(mainGridA.p());


    RigGridManager gridCollection;
    gridCollection.addCase(eclipseCase.p());


    cvf::ref<RigMainGrid> mainGridB = new RigMainGrid;
    cvf::ref<RigMainGrid> existingGrid = gridCollection.findEqualGrid(mainGridB.p());
    EXPECT_TRUE(existingGrid.isNull());

    mainGridB->nodes().push_back(cvf::Vec3d(0, 0, 0));
    existingGrid = gridCollection.findEqualGrid(mainGridB.p());
    EXPECT_TRUE(existingGrid.isNull());

    // Insert nodes in opposite direction
    mainGridB->nodes().push_back(cvf::Vec3d(0, 0, 2));
    mainGridB->nodes().push_back(cvf::Vec3d(0, 0, 1));
    existingGrid = gridCollection.findEqualGrid(mainGridB.p());
    EXPECT_TRUE(existingGrid.isNull());

    // Overwrite to match the node structure of mainGridA
    mainGridB->nodes()[1] = cvf::Vec3d(0, 0, 1);
    mainGridB->nodes()[2] = cvf::Vec3d(0, 0, 2);
    existingGrid = gridCollection.findEqualGrid(mainGridB.p());
    EXPECT_TRUE(existingGrid.notNull());

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

        const RigWellResultFrame& wellCells = wellCellTimeHistory->wellResultFrame(wellCellTimeHistory->m_resultTimeStepIndexToWellTimeStepIndex[i]);
        qDebug() << wellCells.m_timestamp;
    }

}


*/
