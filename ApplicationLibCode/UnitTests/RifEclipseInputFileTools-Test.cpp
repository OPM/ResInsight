#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifEclipseInputFileTools.h"
#include "RifEclipseInputPropertyLoader.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, FaultFaces )
{
    {
        QStringList faceTexts;
        faceTexts << "X" << "X+" << "I" << "I+" << "x" << "x+" << "i" << "i+";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_I, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "X-" << "I-" << "x-" << "i-";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NEG_I, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y" << "Y+" << "J" << "J+" << "y" << "y+" << "j" << "j+";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_J, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y-" << "J-" << "y-" << "j-";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NEG_J, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z" << "Z+" << "K" << "k+" << "z" << "z+" << "k" << "k+";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_K, faceType );
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z-" << "K-" << "z-" << "k-";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NEG_K, faceType );
        }
    }

    // Improved parsing handling some special cases
    {
        QStringList faceTexts;
        faceTexts << "Z--" << "z--" << "z/" << " y /";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_NE( cvf::StructGridInterface::NO_FACE, faceType );
        }
    }

    // Invalid faces
    {
        QStringList faceTexts;
        faceTexts << "-k-" << " -k " << "   +k-  ";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::NO_FACE, faceType );
        }
    }

    // Valid cases with whitespace
    {
        QStringList faceTexts;
        faceTexts << " X" << " X+ " << " I " << " i+  ";

        cvf::StructGridInterface::FaceEnum faceType;
        for ( const QString& text : faceTexts )
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText( text );
            EXPECT_EQ( cvf::StructGridInterface::POS_I, faceType );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, EquilData )
{
    static const QString testDataRootFolder = QString( "%1/ParsingOfDataKeywords/" ).arg( TEST_DATA_DIR );

    {
        QString fileName = testDataRootFolder + "simulation/MY_CASE.DATA";

        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        RifEclipseInputFileTools::parseAndReadPathAliasKeyword( fileName, &pathAliasDefinitions );

        const QString        keyword( "EQUIL" );
        const QString        keywordToStopParsing;
        const qint64         startPositionInFile = 0;
        QStringList          keywordContent;
        std::vector<QString> fileNamesContainingKeyword;
        bool                 isStopParsingKeywordDetected = false;
        const QString        includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   pathAliasDefinitions,
                                                                                   includeStatementAbsolutePathPrefix,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   keywordContent,
                                                                                   fileNamesContainingKeyword,
                                                                                   isStopParsingKeywordDetected );
        EXPECT_EQ( (int)10, keywordContent.size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, FaultData )
{
    static const QString testDataRootFolder = QString( "%1/ParsingOfDataKeywords/" ).arg( TEST_DATA_DIR );

    {
        QString fileName = testDataRootFolder + "simulation/MY_CASE.DATA";

        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        RifEclipseInputFileTools::parseAndReadPathAliasKeyword( fileName, &pathAliasDefinitions );

        const QString        keyword( "FAULTS" );
        const QString        keywordToStopParsing;
        const qint64         startPositionInFile = 0;
        QStringList          keywordContent;
        std::vector<QString> fileNamesContainingKeyword;
        bool                 isStopParsingKeywordDetected = false;
        const QString        includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   pathAliasDefinitions,
                                                                                   includeStatementAbsolutePathPrefix,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   keywordContent,
                                                                                   fileNamesContainingKeyword,
                                                                                   isStopParsingKeywordDetected );

        EXPECT_EQ( (int)977, keywordContent.size() );

        /*
                for (const auto& s : keywordContent)
                {
                    qDebug() << s;
                }
        */
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, FaultDataPflotran )
{
    static const QString testDataRootFolder = QString( "%1/RifEclipseInputFileTools/pflotran" ).arg( TEST_DATA_DIR );

    {
        QString fileName = testDataRootFolder + "/model/P_FLT_TEST_NO-KH.in";

        cvf::Collection<RigFault> faults;

        RifEclipseInputFileTools::parsePflotranInputFile( fileName, &faults );

        EXPECT_EQ( (size_t)4, faults.size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, StopAtKeyword )
{
    static const QString testDataRootFolder = QString( "%1/ParsingOfDataKeywords/" ).arg( TEST_DATA_DIR );
    QString              fileName           = testDataRootFolder + "simulation/MY_CASE_2.DATA";

    QFile data( fileName );
    if ( !data.open( QFile::ReadOnly ) )
    {
        return;
    }

    {
        const QString                            keyword( "FAULTS" );
        const QString                            keywordToStopParsing( "EDIT" );
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   pathAliasDefinitions,
                                                                                   includeStatementAbsolutePathPrefix,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   keywordContent,
                                                                                   fileNamesContainingKeyword,
                                                                                   isStopParsingKeywordDetected );

        EXPECT_TRUE( isStopParsingKeywordDetected );
        EXPECT_TRUE( keywordContent.isEmpty() );
    }

    {
        const QString                            keyword( "EQUIL" );
        const QString                            keywordToStopParsing( "SCHEDULE" );
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   pathAliasDefinitions,
                                                                                   includeStatementAbsolutePathPrefix,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   keywordContent,
                                                                                   fileNamesContainingKeyword,
                                                                                   isStopParsingKeywordDetected );
        EXPECT_TRUE( isStopParsingKeywordDetected );
        EXPECT_TRUE( keywordContent.isEmpty() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, DISABLED_FindFilesWithVfp )
{
    QString fileName = "d:/gitroot-ceesol/ResInsight-regression-test/ModelData/norne/NORNE_ATW2013.DATA";

    {
        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        const QString                            keyword( "VFPPROD" );
        const QString                            keywordToStopParsing;
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   pathAliasDefinitions,
                                                                                   includeStatementAbsolutePathPrefix,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   keywordContent,
                                                                                   fileNamesContainingKeyword,
                                                                                   isStopParsingKeywordDetected );

        //        EXPECT_TRUE( isStopParsingKeywordDetected );
        //        EXPECT_TRUE( keywordContent.isEmpty() );
    }

    {
        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            return;
        }

        const QString                            keyword( "VFPINJ" );
        const QString                            keywordToStopParsing;
        const qint64                             startPositionInFile = 0;
        std::vector<std::pair<QString, QString>> pathAliasDefinitions;
        QStringList                              keywordContent;
        std::vector<QString>                     fileNamesContainingKeyword;
        bool                                     isStopParsingKeywordDetected = false;
        const QString                            includeStatementAbsolutePathPrefix;

        RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                                   keywordToStopParsing,
                                                                                   pathAliasDefinitions,
                                                                                   includeStatementAbsolutePathPrefix,
                                                                                   data,
                                                                                   startPositionInFile,
                                                                                   keywordContent,
                                                                                   fileNamesContainingKeyword,
                                                                                   isStopParsingKeywordDetected );

        //      EXPECT_TRUE( isStopParsingKeywordDetected );
        //        EXPECT_TRUE( keywordContent.isEmpty() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportGrid )
{
    QString   testDataFile = QString( "%1/reek/reek_box_grid_w_props.grdecl" ).arg( TEST_MODEL_DIR );
    QFileInfo testFileInfo( testDataFile );
    ASSERT_TRUE( testFileInfo.exists() ) << "Test data file not found: " << testDataFile.toStdString();

    auto eclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );

    QString errorMessages;
    bool    success = RifEclipseInputFileTools::openGridFile( testDataFile, eclipseCase.get(), false, &errorMessages );

    ASSERT_TRUE( success ) << "Failed to load test grid file: " << errorMessages.toStdString();
    ASSERT_NE( nullptr, eclipseCase->mainGrid() );

    RigMainGrid* mainGrid = eclipseCase->mainGrid();
    EXPECT_GT( mainGrid->cellCount(), 0u );
    EXPECT_GT( mainGrid->cellCountI(), 0u );
    EXPECT_GT( mainGrid->cellCountJ(), 0u );
    EXPECT_GT( mainGrid->cellCountK(), 0u );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFileName = tempDir.path() + "/exported_grid.grdecl";

    bool exportSuccess = RifEclipseInputFileTools::exportGrid( exportFileName, eclipseCase.get(), false );

    EXPECT_TRUE( exportSuccess ) << "Failed to export grid";

    QFileInfo exportedFileInfo( exportFileName );
    EXPECT_TRUE( exportedFileInfo.exists() ) << "Exported file does not exist";
    EXPECT_GT( exportedFileInfo.size(), 0 ) << "Exported file is empty";

    QFile exportedFile( exportFileName );
    ASSERT_TRUE( exportedFile.open( QIODevice::ReadOnly | QIODevice::Text ) );

    QString content = exportedFile.readAll();
    EXPECT_TRUE( content.contains( "SPECGRID" ) ) << "Exported file should contain SPECGRID keyword";
    EXPECT_TRUE( content.contains( "COORD" ) ) << "Exported file should contain COORD keyword";
    EXPECT_TRUE( content.contains( "ZCORN" ) ) << "Exported file should contain ZCORN keyword";

    exportedFile.close();

    // Read the grid back in and verify cell counts
    auto    reloadedEclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );
    QString reloadErrorMessages;
    bool reloadSuccess = RifEclipseInputFileTools::openGridFile( exportFileName, reloadedEclipseCase.get(), false, &reloadErrorMessages );

    ASSERT_TRUE( reloadSuccess ) << "Failed to reload exported grid file: " << reloadErrorMessages.toStdString();
    ASSERT_NE( nullptr, reloadedEclipseCase->mainGrid() );

    RigMainGrid* reloadedGrid = reloadedEclipseCase->mainGrid();

    // Verify dimensions match the original
    EXPECT_EQ( mainGrid->cellCountI(), reloadedGrid->cellCountI() ) << "I dimension should match original";
    EXPECT_EQ( mainGrid->cellCountJ(), reloadedGrid->cellCountJ() ) << "J dimension should match original";
    EXPECT_EQ( mainGrid->cellCountK(), reloadedGrid->cellCountK() ) << "K dimension should match original";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportGridWithRefinement )
{
    QString   testDataFile = QString( "%1/reek/reek_box_grid_w_props.grdecl" ).arg( TEST_MODEL_DIR );
    QFileInfo testFileInfo( testDataFile );
    ASSERT_TRUE( testFileInfo.exists() ) << "Test data file not found: " << testDataFile.toStdString();

    auto eclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );

    QString errorMessages;
    bool    success = RifEclipseInputFileTools::openGridFile( testDataFile, eclipseCase.get(), false, &errorMessages );

    ASSERT_TRUE( success ) << "Failed to load test grid file: " << errorMessages.toStdString();

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFileName = tempDir.path() + "/exported_grid_refined.grdecl";

    cvf::Vec3st refinement( 2, 2, 1 );

    bool exportSuccess =
        RifEclipseInputFileTools::exportGrid( exportFileName, eclipseCase.get(), false, nullptr, cvf::Vec3st::ZERO, cvf::Vec3st::UNDEFINED, refinement );

    EXPECT_TRUE( exportSuccess ) << "Failed to export refined grid";

    QFileInfo exportedFileInfo( exportFileName );
    EXPECT_TRUE( exportedFileInfo.exists() ) << "Exported refined file does not exist";
    EXPECT_GT( exportedFileInfo.size(), 0 ) << "Exported refined file is empty";

    // Read the grid back in and verify refined cell counts
    auto    reloadedEclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );
    QString reloadErrorMessages;
    bool reloadSuccess = RifEclipseInputFileTools::openGridFile( exportFileName, reloadedEclipseCase.get(), false, &reloadErrorMessages );

    ASSERT_TRUE( reloadSuccess ) << "Failed to reload exported refined grid file: " << reloadErrorMessages.toStdString();
    ASSERT_NE( nullptr, reloadedEclipseCase->mainGrid() );

    RigMainGrid* reloadedGrid = reloadedEclipseCase->mainGrid();
    RigMainGrid* originalGrid = eclipseCase->mainGrid();

    // Verify dimensions match the refined grid (2x2x1 refinement)
    EXPECT_EQ( originalGrid->cellCountI() * refinement.x(), reloadedGrid->cellCountI() ) << "I dimension should be refined by 2x";
    EXPECT_EQ( originalGrid->cellCountJ() * refinement.y(), reloadedGrid->cellCountJ() ) << "J dimension should be refined by 2x";
    EXPECT_EQ( originalGrid->cellCountK() * refinement.z(), reloadedGrid->cellCountK() ) << "K dimension should be refined by 1x";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportGridSubset )
{
    QString   testDataFile = QString( "%1/reek/reek_box_grid_w_props.grdecl" ).arg( TEST_MODEL_DIR );
    QFileInfo testFileInfo( testDataFile );
    ASSERT_TRUE( testFileInfo.exists() ) << "Test data file not found: " << testDataFile.toStdString();

    auto eclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );

    QString errorMessages;
    bool    success = RifEclipseInputFileTools::openGridFile( testDataFile, eclipseCase.get(), false, &errorMessages );

    ASSERT_TRUE( success ) << "Failed to load test grid file: " << errorMessages.toStdString();

    RigMainGrid* mainGrid = eclipseCase->mainGrid();

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( std::min( mainGrid->cellCountI() - 1, static_cast<size_t>( 5u ) ),
                     std::min( mainGrid->cellCountJ() - 1, static_cast<size_t>( 5u ) ),
                     std::min( mainGrid->cellCountK() - 1, static_cast<size_t>( 2u ) ) );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFileName = tempDir.path() + "/exported_grid_subset.grdecl";

    bool exportSuccess = RifEclipseInputFileTools::exportGrid( exportFileName, eclipseCase.get(), false, nullptr, min, max );

    EXPECT_TRUE( exportSuccess ) << "Failed to export grid subset";

    QFileInfo exportedFileInfo( exportFileName );
    EXPECT_TRUE( exportedFileInfo.exists() ) << "Exported subset file does not exist";
    EXPECT_GT( exportedFileInfo.size(), 0 ) << "Exported subset file is empty";

    // Read the grid back in and verify subset cell counts
    auto    reloadedEclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );
    QString reloadErrorMessages;
    bool reloadSuccess = RifEclipseInputFileTools::openGridFile( exportFileName, reloadedEclipseCase.get(), false, &reloadErrorMessages );

    ASSERT_TRUE( reloadSuccess ) << "Failed to reload exported subset grid file: " << reloadErrorMessages.toStdString();
    ASSERT_NE( nullptr, reloadedEclipseCase->mainGrid() );

    RigMainGrid* reloadedGrid = reloadedEclipseCase->mainGrid();

    // Verify dimensions match the subset (max - min + 1)
    size_t expectedI = max.x() - min.x() + 1;
    size_t expectedJ = max.y() - min.y() + 1;
    size_t expectedK = max.z() - min.z() + 1;

    EXPECT_EQ( expectedI, reloadedGrid->cellCountI() ) << "I dimension should match subset size: " << expectedI;
    EXPECT_EQ( expectedJ, reloadedGrid->cellCountJ() ) << "J dimension should match subset size: " << expectedJ;
    EXPECT_EQ( expectedK, reloadedGrid->cellCountK() ) << "K dimension should match subset size: " << expectedK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportKeywords )
{
    QString   testDataFile = QString( "%1/reek/reek_box_grid_w_props.grdecl" ).arg( TEST_MODEL_DIR );
    QFileInfo testFileInfo( testDataFile );
    ASSERT_TRUE( testFileInfo.exists() ) << "Test data file not found: " << testDataFile.toStdString();

    auto eclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );

    QString errorMessages;
    bool    success = RifEclipseInputFileTools::openGridFile( testDataFile, eclipseCase.get(), false, &errorMessages );

    ASSERT_TRUE( success ) << "Failed to load test grid file: " << errorMessages.toStdString();

    RigCaseCellResultsData* cellResultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( nullptr, cellResultsData );

    auto allResults = cellResultsData->existingResults();
    ASSERT_FALSE( allResults.empty() ) << "No properties found in test data file";

    std::vector<QString> keywordsToExport;
    for ( const auto& result : allResults )
    {
        keywordsToExport.push_back( result.resultName() );
        if ( keywordsToExport.size() >= 2 ) break;
    }

    ASSERT_FALSE( keywordsToExport.empty() ) << "No exportable keywords found";

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFileName = tempDir.path() + "/exported_keywords.txt";

    bool exportSuccess = RifEclipseInputFileTools::exportKeywords( exportFileName, eclipseCase.get(), keywordsToExport, true );

    EXPECT_TRUE( exportSuccess ) << "Failed to export keywords";

    QFileInfo exportedFileInfo( exportFileName );
    EXPECT_TRUE( exportedFileInfo.exists() ) << "Exported keywords file does not exist";
    EXPECT_GT( exportedFileInfo.size(), 0 ) << "Exported keywords file is empty";

    QFile exportedFile( exportFileName );
    ASSERT_TRUE( exportedFile.open( QIODevice::ReadOnly | QIODevice::Text ) );

    QString content = exportedFile.readAll();
    EXPECT_TRUE( content.contains( "NOECHO" ) ) << "Exported file should contain NOECHO keyword";
    EXPECT_TRUE( content.contains( "ECHO" ) ) << "Exported file should contain ECHO keyword";

    for ( const QString& keyword : keywordsToExport )
    {
        EXPECT_TRUE( content.contains( keyword ) ) << "Exported file should contain keyword: " << keyword.toStdString();
    }

    exportedFile.close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportKeywordsWithSubset )
{
    QString   testDataFile = QString( "%1/reek/reek_box_grid_w_props.grdecl" ).arg( TEST_MODEL_DIR );
    QFileInfo testFileInfo( testDataFile );
    ASSERT_TRUE( testFileInfo.exists() ) << "Test data file not found: " << testDataFile.toStdString();

    auto eclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );

    QString errorMessages;
    bool    success = RifEclipseInputFileTools::openGridFile( testDataFile, eclipseCase.get(), false, &errorMessages );

    ASSERT_TRUE( success ) << "Failed to load test grid file: " << errorMessages.toStdString();

    RigCaseCellResultsData* cellResultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( nullptr, cellResultsData );

    auto allResults = cellResultsData->existingResults();
    ASSERT_FALSE( allResults.empty() ) << "No properties found in test data file";

    std::vector<QString> keywordsToExport;
    for ( const auto& result : allResults )
    {
        keywordsToExport.push_back( result.resultName() );
        if ( keywordsToExport.size() >= 1 ) break;
    }

    ASSERT_FALSE( keywordsToExport.empty() ) << "No exportable keywords found";

    RigMainGrid* mainGrid = eclipseCase->mainGrid();

    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( std::min( mainGrid->cellCountI() - 1, static_cast<size_t>( 3u ) ),
                     std::min( mainGrid->cellCountJ() - 1, static_cast<size_t>( 3u ) ),
                     std::min( mainGrid->cellCountK() - 1, static_cast<size_t>( 1u ) ) );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFileName = tempDir.path() + "/exported_keywords_subset.txt";

    bool exportSuccess = RifEclipseInputFileTools::exportKeywords( exportFileName, eclipseCase.get(), keywordsToExport, false, min, max );

    EXPECT_TRUE( exportSuccess ) << "Failed to export keywords with subset";

    QFileInfo exportedFileInfo( exportFileName );
    EXPECT_TRUE( exportedFileInfo.exists() ) << "Exported keywords subset file does not exist";
    EXPECT_GT( exportedFileInfo.size(), 0 ) << "Exported keywords subset file is empty";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportKeywordsWithRefinement )
{
    QString   testDataFile = QString( "%1/reek/reek_box_grid_w_props.grdecl" ).arg( TEST_MODEL_DIR );
    QFileInfo testFileInfo( testDataFile );
    ASSERT_TRUE( testFileInfo.exists() ) << "Test data file not found: " << testDataFile.toStdString();

    auto eclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );

    QString errorMessages;
    bool    success = RifEclipseInputFileTools::openGridFile( testDataFile, eclipseCase.get(), false, &errorMessages );

    ASSERT_TRUE( success ) << "Failed to load test grid file: " << errorMessages.toStdString();

    RigCaseCellResultsData* cellResultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( nullptr, cellResultsData );

    // Look specifically for PORO property
    std::vector<QString> keywordsToExport;
    auto                 allResults = cellResultsData->existingResults();
    bool                 foundPoro  = false;

    for ( const auto& result : allResults )
    {
        if ( result.resultName() == "PORO" )
        {
            keywordsToExport.push_back( result.resultName() );
            foundPoro = true;
            break;
        }
    }

    ASSERT_TRUE( foundPoro ) << "PORO property not found in test data file";

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    cvf::Vec3st refinement( 2, 2, 1 );
    QString     exportFileName = tempDir.path() + "/exported_keywords_refined.txt";

    bool exportSuccess = RifEclipseInputFileTools::exportKeywords( exportFileName,
                                                                   eclipseCase.get(),
                                                                   keywordsToExport,
                                                                   false,
                                                                   cvf::Vec3st::ZERO,
                                                                   cvf::Vec3st::UNDEFINED,
                                                                   refinement );

    EXPECT_TRUE( exportSuccess ) << "Failed to export keywords with refinement";

    QFileInfo exportedFileInfo( exportFileName );
    EXPECT_TRUE( exportedFileInfo.exists() ) << "Exported keywords refined file does not exist";
    EXPECT_GT( exportedFileInfo.size(), 0 ) << "Exported keywords refined file is empty";

    // Verify the exported file contains PORO keyword
    QFile exportedFile( exportFileName );
    ASSERT_TRUE( exportedFile.open( QIODevice::ReadOnly | QIODevice::Text ) );

    QString content = exportedFile.readAll();
    EXPECT_TRUE( content.contains( "PORO" ) ) << "Exported file should contain PORO keyword";
    exportedFile.close();

    // Create a temporary grid file for the refined case to read the property back
    QString gridFileName = tempDir.path() + "/refined_grid.grdecl";
    bool    gridExportSuccess =
        RifEclipseInputFileTools::exportGrid( gridFileName, eclipseCase.get(), false, nullptr, cvf::Vec3st::ZERO, cvf::Vec3st::UNDEFINED, refinement );
    ASSERT_TRUE( gridExportSuccess ) << "Failed to export refined grid for validation";

    // Read the refined grid and exported properties back
    auto refinedEclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );
    bool refinedGridSuccess = RifEclipseInputFileTools::openGridFile( gridFileName, refinedEclipseCase.get(), false, &errorMessages );
    ASSERT_TRUE( refinedGridSuccess ) << "Failed to reload refined grid file: " << errorMessages.toStdString();

    RigMainGrid* originalGrid = eclipseCase->mainGrid();
    RigMainGrid* refinedGrid  = refinedEclipseCase->mainGrid();

    // Verify refined grid dimensions
    EXPECT_EQ( originalGrid->cellCountI() * refinement.x(), refinedGrid->cellCountI() ) << "Refined I dimension incorrect";
    EXPECT_EQ( originalGrid->cellCountJ() * refinement.y(), refinedGrid->cellCountJ() ) << "Refined J dimension incorrect";
    EXPECT_EQ( originalGrid->cellCountK() * refinement.z(), refinedGrid->cellCountK() ) << "Refined K dimension incorrect";

    // Try to import the exported keywords back into the refined grid case
    // This test is expected to FAIL due to a bug in the export function:
    // The export outputs values for active cells only, but the refined grid has more total cells
    auto propertyMap = RifEclipseInputPropertyLoader::readProperties( exportFileName, refinedEclipseCase.get() );

    // This test documents the bug: the export should create the same number of values as refined cells
    // Currently it exports 25830 values but refined grid has 27048 cells
    EXPECT_FALSE( propertyMap.empty() ) << "BUG: Should have imported properties but export doesn't match refined grid cell count";
    EXPECT_TRUE( propertyMap.find( "PORO" ) != propertyMap.end() )
        << "BUG: Should have imported PORO property but cell count mismatch prevents import";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportGridInvalidParams )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFileName = tempDir.path() + "/invalid_export.grdecl";

    bool exportSuccess = RifEclipseInputFileTools::exportGrid( exportFileName, nullptr, false );

    EXPECT_FALSE( exportSuccess ) << "Should fail when passing nullptr eclipseCase";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseInputFileToolsTest, ExportKeywordsInvalidParams )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // Create a valid eclipse case for the test
    auto eclipseCase = std::make_unique<RigEclipseCaseData>( nullptr );

    // Test with empty keywords vector
    QString              exportFileName = tempDir.path() + "/invalid_export.txt";
    std::vector<QString> emptyKeywords;

    bool exportSuccess = RifEclipseInputFileTools::exportKeywords( exportFileName, eclipseCase.get(), emptyKeywords, false );

    EXPECT_TRUE( exportSuccess ) << "Export should succeed even with empty keywords vector (creates empty file)";

    // Test with empty/invalid file name
    std::vector<QString> validKeywords  = { "PORO" };
    bool                 exportSuccess2 = RifEclipseInputFileTools::exportKeywords( "", eclipseCase.get(), validKeywords, false );

    EXPECT_FALSE( exportSuccess2 ) << "Should fail when passing empty file name";
}
