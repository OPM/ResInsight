

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

#include "RIStdInclude.h"
#include "gtest/gtest.h"

#include "RigReservoir.h"

#include "RifReaderInterfaceEcl.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, BasicTest)
{
    cvf::ref<RifReaderInterfaceECL> readerInterfaceEcl = new RifReaderInterfaceECL;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");

    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);

    {
        QStringList staticResults = readerInterfaceEcl->staticResults();
        EXPECT_EQ(42, staticResults.size());
        qDebug() << "Static results\n" << staticResults;

        QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
        EXPECT_EQ(23, dynamicResults.size());
        qDebug() << "Dynamic results\n" << dynamicResults;

        int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
        EXPECT_EQ(9, numTimeSteps);

        QStringList timeStepText = readerInterfaceEcl->timeStepText();
        EXPECT_EQ(numTimeSteps, timeStepText.size());
        qDebug() << "Time step texts\n" << timeStepText;
    }


    readerInterfaceEcl->close();

    {
        QStringList staticResults = readerInterfaceEcl->staticResults();
        EXPECT_EQ(0, staticResults.size());

        QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
        EXPECT_EQ(0, dynamicResults.size());

        int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
        EXPECT_EQ(0, numTimeSteps);

        QStringList timeStepText = readerInterfaceEcl->timeStepText();
        EXPECT_EQ(numTimeSteps, timeStepText.size());
    }

}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, WellTest)
{
    cvf::ref<RifReaderInterfaceECL> readerInterfaceEcl = new RifReaderInterfaceECL;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");

    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);

    cvf::UByteArray* mainGridWellCells = reservoir->wellCellsInGrid(0);
    EXPECT_TRUE(mainGridWellCells->size() == reservoir->mainGrid()->cellCount());


}
