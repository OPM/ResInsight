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

#include "RigEclipseCaseData.h"

#include "ert/ecl/ecl_kw_magic.h"
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw.h>

#include "RiaStringEncodingTools.h"
#include "RiaTestDataDirectory.h"
#include "RifEclipseOutputFileTools.h"
#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifReaderEclipseOutput.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RimEclipseResultCase.h"

#include <QDebug>
#include <QDir>
#include <QTemporaryDir>

#include <limits>
#include <memory>
#include <vector>

using namespace RiaDefines;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigReservoirTest, BasicTest10k )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );

    {
        RigCaseCellResultsData* cellData = reservoir->results( PorosityModelType::MATRIX_MODEL );

        QStringList staticResults = cellData->resultNames( ResultCatType::STATIC_NATIVE );
        EXPECT_EQ( 0, staticResults.size() );
        // qDebug() << "Static results\n" << staticResults;

        QStringList dynamicResults = cellData->resultNames( ResultCatType::DYNAMIC_NATIVE );
        EXPECT_EQ( 0, dynamicResults.size() );
        // qDebug() << "Dynamic results\n" << dynamicResults;

        int numTimeSteps = static_cast<int>( cellData->maxTimeStepCount() );
        EXPECT_EQ( 0, numTimeSteps );
    }

    {
        cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
        bool                             result             = readerInterfaceEcl->open( filePath, reservoir.p() );
        EXPECT_TRUE( result );
        int numTimeSteps = static_cast<int>( readerInterfaceEcl->allTimeSteps().size() );
        EXPECT_EQ( 9, numTimeSteps );
    }

    {
        RigCaseCellResultsData* cellData = reservoir->results( PorosityModelType::MATRIX_MODEL );

        QStringList staticResults = cellData->resultNames( ResultCatType::STATIC_NATIVE );
        EXPECT_EQ( 44, staticResults.size() );
        // qDebug() << "Static results\n" << staticResults;

        QStringList dynamicResults = cellData->resultNames( ResultCatType::DYNAMIC_NATIVE );
        EXPECT_EQ( 23, dynamicResults.size() );
        // qDebug() << "Dynamic results\n" << dynamicResults;

        int numTimeSteps = static_cast<int>( cellData->maxTimeStepCount() );
        EXPECT_EQ( 9, numTimeSteps );
    }
}

//--------------------------------------------------------------------------------------------------
/// Regression guard for issue #14227. The depth-related geometry results
/// (DEPTH/DX/DY/DZ/TOPS/BOTTOM) are computed independently for the matrix and fracture porosity
/// models. They are derived purely from the shared grid geometry, so for a dual-porosity model a
/// cell that is active in both models must receive identical values in each model.
//--------------------------------------------------------------------------------------------------
TEST( RigReservoirTest, DualPorosityDepthResults )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "dualporo-testcase" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "DUALPORO-WELL-COMPLETION-DEFAULT.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             result             = readerInterfaceEcl->open( filePath, reservoir.p() );
    EXPECT_TRUE( result );

    RigCaseCellResultsData* matrixResults   = reservoir->results( PorosityModelType::MATRIX_MODEL );
    RigCaseCellResultsData* fractureResults = reservoir->results( PorosityModelType::FRACTURE_MODEL );
    ASSERT_TRUE( matrixResults != nullptr );
    ASSERT_TRUE( fractureResults != nullptr );

    RigActiveCellInfo* matrixActiveInfo   = reservoir->activeCellInfo( PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveInfo = reservoir->activeCellInfo( PorosityModelType::FRACTURE_MODEL );
    ASSERT_TRUE( matrixActiveInfo != nullptr );
    ASSERT_TRUE( fractureActiveInfo != nullptr );

    // This model must actually exercise both porosity models, otherwise the test is meaningless.
    EXPECT_GT( matrixActiveInfo->reservoirActiveCellCount(), size_t( 0 ) );
    EXPECT_GT( fractureActiveInfo->reservoirActiveCellCount(), size_t( 0 ) );

    // Compute depth-related geometry results for both porosity models.
    reservoir->computeDepthRelatedResults();

    const QStringList propertyNames = { "DEPTH", "DX", "DY", "DZ", "TOPS", "BOTTOM" };

    const double undefinedValue = std::numeric_limits<double>::max();
    const size_t totalCellCount = reservoir->mainGrid()->totalCellCount();

    for ( const QString& propertyName : propertyNames )
    {
        RigEclipseResultAddress resAddr( ResultCatType::STATIC_NATIVE, propertyName );

        EXPECT_TRUE( matrixResults->hasResultEntry( resAddr ) );
        EXPECT_TRUE( fractureResults->hasResultEntry( resAddr ) );

        const std::vector<double>& matrixValues   = matrixResults->cellScalarResults( resAddr, 0 );
        const std::vector<double>& fractureValues = fractureResults->cellScalarResults( resAddr, 0 );

        EXPECT_EQ( matrixValues.size(), matrixActiveInfo->reservoirActiveCellCount() );
        EXPECT_EQ( fractureValues.size(), fractureActiveInfo->reservoirActiveCellCount() );

        // For every reservoir cell active in both models, the geometry-derived value must match.
        size_t comparedCellCount = 0;
        for ( size_t cellIdx = 0; cellIdx < totalCellCount; cellIdx++ )
        {
            size_t matrixResultIdx   = matrixActiveInfo->cellResultIndex( ReservoirCellIndex( cellIdx ) ).value();
            size_t fractureResultIdx = fractureActiveInfo->cellResultIndex( ReservoirCellIndex( cellIdx ) ).value();
            if ( matrixResultIdx == cvf::UNDEFINED_SIZE_T || fractureResultIdx == cvf::UNDEFINED_SIZE_T ) continue;
            if ( matrixResultIdx >= matrixValues.size() || fractureResultIdx >= fractureValues.size() ) continue;

            const double matrixValue   = matrixValues[matrixResultIdx];
            const double fractureValue = fractureValues[fractureResultIdx];

            EXPECT_NE( matrixValue, undefinedValue );
            EXPECT_NE( fractureValue, undefinedValue );
            EXPECT_DOUBLE_EQ( matrixValue, fractureValue );
            comparedCellCount++;
        }

        EXPECT_GT( comparedCellCount, size_t( 0 ) );
    }
}

TEST( RigReservoirTest, OpenGridFileWithoutDimensKeyword )
{
    // Importing a GRID file without the DIMENS keyword threw an uncaught std::out_of_range from the
    // resdata keyword lookup and terminated the application. The import must fail gracefully instead.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString gridFileName = QDir( tempDir.path() ).filePath( "TEST.GRID" );
    {
        std::vector<int> data( 10, 0 );
        ecl_kw_fwrite_param( RiaStringEncodingTools::toNativeEncoded( gridFileName ).data(),
                             false,
                             "INTEHEAD",
                             ECL_INT,
                             (int)data.size(),
                             data.data() );
    }
    ASSERT_TRUE( QFile::exists( gridFileName ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          reservoir = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    EXPECT_FALSE( readerInterfaceEcl->open( gridFileName, reservoir.p() ) );
}

TEST( RigReservoirTest, BasicTest10kRestart )
{
    RifEclipseUnifiedRestartFileAccess unrstAccess;

    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.UNRST" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    QStringList filenames;
    filenames << filePath;
    unrstAccess.setRestartFiles( filenames );

    auto keywordValueCounts = unrstAccess.keywordValueCounts();
    EXPECT_EQ( (size_t)83, keywordValueCounts.size() );

    auto reportNums = unrstAccess.reportNumbers();
    EXPECT_EQ( (size_t)9, reportNums.size() );
}

TEST( RigReservoirTest, BasicTest10k_NativeECL )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    ecl_grid_type* grid = ecl_grid_alloc( RiaStringEncodingTools::toNativeEncoded( filePath ).data() );
    EXPECT_TRUE( grid );

    QString subDir( "RifReaderEclipseOutput" );
    QDir    dataDir( TEST_DATA_DIR );
    dataDir.mkdir( subDir );
    dataDir.cd( subDir );
    QString outFilePath = dataDir.absoluteFilePath( "TEST10K_FLT_LGR_NNC_OUT.GRDECL" );

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif
    FILE* filePtr = fopen( RiaStringEncodingTools::toNativeEncoded( outFilePath ).data(), "w" );
    EXPECT_TRUE( filePtr != nullptr );
    ecl_grid_fprintf_grdecl( grid, filePtr );
    fclose( filePtr );
    EXPECT_TRUE( QFile::exists( outFilePath ) );

#ifdef _WIN32
    QString expectedMd5( QByteArray::fromHex( "e993b85140568f13f4c3849604700a0f" ) );
#else
    QString expectedMd5( QByteArray::fromHex( "274e44fe51299c1f9ca6645c384e237d" ) );
#endif
    QByteArray generatedMd5 = RifEclipseOutputFileTools::md5sum( outFilePath );

    // Enable to produce text string so expectedMd5 can be updated
    // Qt4 doesn't take a parameter for toHex()
    // qDebug() << expectedMd5.toLatin1().toHex(0) << "     " << generatedMd5.toHex(0);
    EXPECT_TRUE( generatedMd5 == expectedMd5 );

#ifdef _MSC_VER
#pragma warning( pop )
#endif
    ecl_grid_free( grid );
}

TEST( RigReservoirTest, Test10k_ReadThenWriteToECL )
{
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
    EXPECT_TRUE( subFolderExists );
    QString filename( "TEST10K_FLT_LGR_NNC.EGRID" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    resultCase->setGridFileName( filePath );
    resultCase->importGridAndResultMetaData( false );

    QString subDir( "RifReaderEclipseOutput" );
    QDir    dataDir( TEST_DATA_DIR );
    dataDir.mkdir( subDir );
    dataDir.cd( subDir );
    QString outFilePath = dataDir.absoluteFilePath( "TEST10K_FLT_LGR_NNC_OUT_FROM_RES.GRDECL" );

    /* bool worked = RifReaderEclipseOutput::saveEclipseGrid(outFilePath, resultCase->eclipseCaseData());
    EXPECT_TRUE(worked);
    EXPECT_TRUE(QFile::exists(outFilePath));

    QString dataFilePath = dataDir.absoluteFilePath("TEST10K_FLT_LGR_NNC_OUT_FROM_RES.VARS");

    QStringList allStaticResults =
    resultCase->eclipseCaseData()->results(MATRIX_MODEL)->resultNames(RiaDefines::STATIC_NATIVE);

    std::vector<QString> keywords;
    for (QString keyword : allStaticResults)
    {
        keywords.push_back(keyword);
    }

    worked = RifReaderEclipseOutput::saveEclipseResults(dataFilePath, resultCase->eclipseCaseData(), keywords);
    EXPECT_TRUE(worked);
    EXPECT_TRUE(QFile::exists(dataFilePath)); */
}
#if 0



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigReservoirTest, DISABLED_BasicTest)
{

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigCaseData> reservoir = new RigCaseData;

    QString filename("d:/Models/Statoil/troll_MSW/T07-4A-W2012-16-F3.EGRID");

    RifReaderSettings readerSettings;
    readerInterfaceEcl->setReaderSetting(&readerSettings);

    bool result = readerInterfaceEcl->open(filename, reservoir.p());
    EXPECT_TRUE(result);

    {
//         QStringList staticResults = readerInterfaceEcl->staticResults();
//         EXPECT_EQ(42, staticResults.size());
//         qDebug() << "Static results\n" << staticResults;
// 
//         QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
//         EXPECT_EQ(23, dynamicResults.size());
//         qDebug() << "Dynamic results\n" << dynamicResults;
// 
//         int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
//         EXPECT_EQ(9, numTimeSteps);
// 
//         QStringList timeStepText = readerInterfaceEcl->timeStepText();
//         EXPECT_EQ(numTimeSteps, timeStepText.size());
//         qDebug() << "Time step texts\n" << timeStepText;
    }


    readerInterfaceEcl->close();

    {
//         QStringList staticResults = readerInterfaceEcl->staticResults();
//         EXPECT_EQ(0, staticResults.size());
// 
//         QStringList dynamicResults = readerInterfaceEcl->dynamicResults();
//         EXPECT_EQ(0, dynamicResults.size());
// 
//         int numTimeSteps = static_cast<int>(readerInterfaceEcl->numTimeSteps());
//         EXPECT_EQ(0, numTimeSteps);
// 
//         QStringList timeStepText = readerInterfaceEcl->timeStepText();
//         EXPECT_EQ(numTimeSteps, timeStepText.size());
    }

}



TEST(RigReservoirTest, DISABLED_FileOutputToolsTest)
{
    cvf::DebugTimer timer("test");


//    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID");
//    QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
//    QString filename("d:/Models/Statoil/troll_MSW/T07-4A-W2012-16-F3.UNRST");
    QString filename("c:/tmp/troll_MSW/T07-4A-W2012-16-F3.UNRST");
    

    ecl_file_type* ertFile = ecl_file_open(filename.toAscii().data(), ECL_FILE_CLOSE_STREAM);
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
    ertFile = nullptr;

    timer.reportTime();
    //qDebug() << timer.lapt;
}


TEST(RigReservoirTest, UnifiedTestFile)
{
    //QString filename("d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.UNRST");
    QString filename("d:/Models/Statoil/troll_MSW/T07-4A-W2012-16-F3.UNRST");

    {
        cvf::ref<RifEclipseUnifiedRestartFileAccess> restartFile = new RifEclipseUnifiedRestartFileAccess();

        QStringList fileNameList;
        fileNameList << filename;
        restartFile->setRestartFiles(fileNameList);
        restartFile->open();

        QStringList resultNames;
        std::vector<size_t> resultDataItemCounts;
        restartFile->resultNames(&resultNames, &resultDataItemCounts);

        qDebug() << "Result names\n";
        for (int i = 0; i < resultNames.size(); i++)
        {
            qDebug() << resultNames[i] << "\t" << resultDataItemCounts[i];
        }

        std::vector<QDateTime> tsteps = restartFile->timeSteps();

        qDebug() << "Time step texts\n";
        for (int i = 0; i < tsteps.size(); i++)
        {
            qDebug() << tsteps[i].toString();
        }

        /*
        std::vector<double> resultValues;
        size_t timeStep = 0;
        restartFile->results(resultNames[0], timeStep, &resultValues);

        size_t i;
        for (i = 0; i < 500; i++)
        {
            qDebug() <<  resultValues[i];
        }
        */
    }

}



void buildResultInfoString(RigReservoir* reservoir, RiaDefines::PorosityModelType porosityModel, RiaDefines::ResultCatType resultType)
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
    buildResultInfoString(reservoir.p(), RiaDefines::MATRIX_MODEL, RiaDefines::DYNAMIC_NATIVE);

    qDebug() << "\n\n" << 
        "Matrix porosities, STATIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RiaDefines::MATRIX_MODEL, RiaDefines::STATIC_NATIVE);

    qDebug() << "\n\n" << 
        "Fracture porosities, DYNAMIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RiaDefines::FRACTURE_MODEL, RiaDefines::DYNAMIC_NATIVE);

    qDebug() << "\n\n" << 
        "Fracture porosities, STATIC results" <<
        "----------------------------------";
    buildResultInfoString(reservoir.p(), RiaDefines::FRACTURE_MODEL, RiaDefines::STATIC_NATIVE);
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
    cvf::ref<RigEclipseCaseData> reservoir = new RigEclipseCaseData;

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RigReservoirTest, WellTest )
{
    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    cvf::ref<RigEclipseCaseData>     reservoir          = new RigEclipseCaseData( nullptr );

    // Location of test dataset received from Håkon Høgstøl in July 2011 with 10k active cells
#ifdef WIN32
    QString filename( "d:/Models/Statoil/soursim/PKMUNK_NOV_TEST_SS.GRID" );
    QString sourSim( "d:/Models/Statoil/soursim/result.sourres.00001" );
#else
    QString filename( "/mnt/hgfs/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.EGRID" );
    QString sourSim( "d:/Models/Statoil/soursim/result.sourres" );
#endif

    bool result = readerInterfaceEcl->open( filename, reservoir.p() );
    EXPECT_TRUE( result );

    readerInterfaceEcl->setHdf5FileName( sourSim );
}
