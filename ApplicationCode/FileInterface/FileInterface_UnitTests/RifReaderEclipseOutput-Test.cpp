

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

#include "RiaStdInclude.h"
#include "gtest/gtest.h"

#include "RigCaseData.h"

#include "RifReaderEclipseOutput.h"
#include "ecl_file.h"
#include "RifEclipseOutputFileTools.h"
#include "RigCaseCellResultsData.h"





#if 0


TEST(RigReservoirTest, FileOutputToolsTest)
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

//    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");
    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");

    ecl_file_type* ertFile = ecl_file_open(filename.toAscii().data());
    EXPECT_TRUE(ertFile);


    QStringList keywords;
    std::vector<size_t> keywordDataItemCounts;
    RifEclipseOutputFileTools::findKeywordsAndDataItemCounts(ertFile, &keywords, &keywordDataItemCounts);

    EXPECT_TRUE(keywords.size() == keywordDataItemCounts.size());


    qDebug() << "Keyword - Number of data items";
    for (int i = 0; i < keywords.size(); i++)
    {
        QString paddedKeyword = QString("%1").arg(keywords[i], 8);
        qDebug() << paddedKeyword << " - " << keywordDataItemCounts[i];
    }

    ecl_file_close(ertFile);
    ertFile = NULL;
}


void buildResultInfoString(RigReservoir* reservoir, RifReaderInterface::PorosityModelResultType porosityModel, RimDefines::ResultCatType resultType)
{
    RigCaseCellResultsData* matrixResults = reservoir->results(porosityModel);
    {
        QStringList resultNames = matrixResults->resultNames(resultType);

        for (size_t i = 0 ; i < resultNames.size(); i++)
        {
            std::vector<double> values;
            size_t scalarResultIndex = matrixResults->findOrLoadScalarResult(resultType, resultNames[i]);
            EXPECT_TRUE(scalarResultIndex != cvf::UNDEFINED_SIZE_T);

            QString resultText = QString("%1").arg(resultNames[i], 8);
            std::vector< std::vector<double> > & resultValues = matrixResults->cellScalarResults(scalarResultIndex);
            for (size_t timeStepIdx = 0; timeStepIdx < matrixResults->timeStepCount(scalarResultIndex); timeStepIdx++)
            {
                size_t resultValueCount = resultValues[timeStepIdx].size();
                resultText += QString(" %1").arg(resultValueCount);
            }

            qDebug() << resultText;
        }
        qDebug() << "Number of items : " << resultNames.size();
    }
}

TEST(RigReservoirTest, DualPorosityTest)
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

    QString filename("d:/Models/Statoil/DualProperty/DUALPORO.EGRID");
    //QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");


    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);


    qDebug() << "\n\n" << 
        "Matrix porosities, DYNAMIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RifReaderInterface::MATRIX_RESULTS, RimDefines::DYNAMIC_NATIVE);

    qDebug() << "\n\n" << 
        "Matrix porosities, STATIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RifReaderInterface::MATRIX_RESULTS, RimDefines::STATIC_NATIVE);

    qDebug() << "\n\n" << 
        "Fracture porosities, DYNAMIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RifReaderInterface::FRACTURE_RESULTS, RimDefines::DYNAMIC_NATIVE);

    qDebug() << "\n\n" << 
        "Fracture porosities, STATIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RifReaderInterface::FRACTURE_RESULTS, RimDefines::STATIC_NATIVE);
}














//#include "RifEclipseUnifiedRestartFileAccess.h"

/*
TEST(RigReservoirTest, UnifiedTestFile)
{

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
#else
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
#endif

    size_t numActiveCells = 11125;
    size_t numGrids = 2;

    {
        cvf::ref<RifEclipseUnifiedRestartFileAccess> restartFile = new RifEclipseUnifiedRestartFileAccess(numGrids, numActiveCells);

        QStringList fileNameList;
        fileNameList << filename;
        restartFile->open(fileNameList);

        QStringList resultNames = restartFile->resultNames();
        qDebug() << "Result names\n" << resultNames;

        QStringList timeStepText = restartFile->timeStepsText();
        qDebug() << "Time step texts\n" << timeStepText;

        std::vector<double> resultValues;
        size_t timeStep = 0;
        restartFile->results(resultNames[0], timeStep, &resultValues);

        size_t i;
        for (i = 0; i < 500; i++)
        {
            qDebug() <<  resultValues[i];
        }

    }


}
*/

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, BasicTest)
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename("TEST10K_FLT_LGR_NNC.EGRID");
#else
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");
#endif

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
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigCaseData> reservoir = new RigCaseData;

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename("TEST10K_FLT_LGR_NNC.EGRID");
#else
    QString filename("/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");
#endif

    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);

    cvf::UByteArray* mainGridWellCells = reservoir->wellCellsInGrid(0);
    EXPECT_TRUE(mainGridWellCells->size() == reservoir->mainGrid()->cellCount());
}



#endif

