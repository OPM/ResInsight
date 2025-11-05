/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RiaDefines.h"
#include "RiaTestDataDirectory.h"
#include "RifEclipseInputFileTools.h"
#include "RifOpmFlowDeckFile.h"
#include "RifReaderEclipseOutput.h"
#include "RigEclipseResultTools.h"

#include "ProjectDataModel/Jobs/RimKeywordFactory.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RimEclipseResultCase.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>

//--------------------------------------------------------------------------------------------------
/// Test BCCON generation from border cells
///
/// This test verifies that:
/// 1. We can generate BORDNUM result identifying border and interior cells
/// 2. We can extract border cell faces that connect to interior cells
/// 3. We can write BCCON keyword to an OPM deck file
///
/// Test process:
/// 1. Load reek test model
/// 2. Create custom visibility for a subset of cells (a box region)
/// 3. Generate BORDNUM result
/// 4. Generate border cell faces
/// 5. Verify border cells and faces are correctly identified
/// 6. Create OPM deck and add BCCON keyword
/// 7. Save and verify deck file format
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseResultToolsTest, BorderCellBcconGeneration )
{
    // Setup test data directory - use BRUGGE model like other tests
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Test model directory not found";

    QString inputFilename( "BRUGGE_0000.EGRID" );
    QString inputFilePath = baseFolder.absoluteFilePath( inputFilename );
    ASSERT_TRUE( QFile::exists( inputFilePath ) ) << "Test model file not found: " << inputFilePath.toStdString();

    // Step 1: Load original grid using RifReaderEclipseOutput (properly initializes everything)
    std::unique_ptr<RimEclipseResultCase> testCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( testCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( inputFilePath, caseData.p() );
    ASSERT_TRUE( success ) << "Failed to load grid";

    testCase->setReservoirData( caseData.p() );

    const RigMainGrid* grid = caseData->mainGrid();
    ASSERT_NE( grid, nullptr ) << "Grid is null";

    // Step 2: Create custom visibility for a subset of cells (e.g., middle 50% of grid)
    size_t cellCount = grid->cellCount();

    cvf::ref<cvf::UByteArray> customVisibility = new cvf::UByteArray( cellCount );
    customVisibility->setAll( 0 ); // Start with all invisible

    std::vector<int> bcconValues( cellCount, 0 );

    // Make a box in the middle visible
    size_t startI = grid->cellCountI() / 4;
    size_t endI   = 3 * grid->cellCountI() / 4;
    size_t startJ = grid->cellCountJ() / 4;
    size_t endJ   = 3 * grid->cellCountJ() / 4;
    size_t startK = grid->cellCountK() / 4;
    size_t endK   = 3 * grid->cellCountK() / 4;

    for ( size_t i = startI; i < endI; ++i )
    {
        for ( size_t j = startJ; j < endJ; ++j )
        {
            for ( size_t k = startK; k < endK; ++k )
            {
                size_t cellIndex                 = grid->cellIndexFromIJK( i, j, k );
                ( *customVisibility )[cellIndex] = 1;

                // Set bccon==1 for half of the cells
                if ( i > grid->cellCountI() / 2 )
                {
                    bcconValues[cellIndex] = 1;
                }
            }
        }
    }

    // Step 3: Generate BORDNUM result
    RigEclipseResultTools::generateBorderResult( testCase.get(), customVisibility, "BORDNUM" );

    // Verify BORDNUM was created
    auto resultsData = testCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );

    RigEclipseResultAddress bordNumAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, "BORDNUM" );
    ASSERT_TRUE( resultsData->hasResultEntry( bordNumAddr ) ) << "BORDNUM result not created";

    RigEclipseResultTools::createResultVector( *testCase, "BCCON", bcconValues );

    // Step 4: Generate border cell faces
    auto borderCellFaces = RigEclipseResultTools::generateBorderCellFaces( testCase.get() );

    // Step 5: Verify results
    EXPECT_GT( borderCellFaces.size(), 0 ) << "No border cell faces generated";

    // Verify that all faces are within the visible region boundaries
    for ( const auto& face : borderCellFaces )
    {
        EXPECT_GE( face.ijk[0], startI );
        EXPECT_LT( face.ijk[0], endI );
        EXPECT_GE( face.ijk[1], startJ );
        EXPECT_LT( face.ijk[1], endJ );
        EXPECT_GE( face.ijk[2], startK );
        EXPECT_LT( face.ijk[2], endK );

        // Verify face type is valid
        EXPECT_GE( static_cast<int>( face.faceType ), static_cast<int>( cvf::StructGridInterface::POS_I ) );
        EXPECT_LT( static_cast<int>( face.faceType ), static_cast<int>( cvf::StructGridInterface::NO_FACE ) );

        // Verify boundary condition (should be 1 if no BCCON grid property exists)
        EXPECT_EQ( face.boundaryCondition, 1 );
    }

    // Step 6: Create OPM deck file and add BCCON keyword
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() ) << "Failed to create temporary directory";

    // Create a minimal deck file with GRID section
    QString deckFilePath = tempDir.filePath( "test_bccon.DATA" );
    {
        QFile deckFile( deckFilePath );
        ASSERT_TRUE( deckFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
        QTextStream out( &deckFile );
        out << "RUNSPEC\n\n";
        out << "DIMENS\n";
        out << grid->cellCountI() << " " << grid->cellCountJ() << " " << grid->cellCountK() << " /\n\n";
        out << "GRID\n\n";
        out << "-- BCCON will be added here\n\n";
        out << "PROPS\n\n";
        out << "SCHEDULE\n\n";
        deckFile.close();
    }

    // Load the deck file
    RifOpmFlowDeckFile deckFile;
    bool               deckLoaded = deckFile.loadDeck( deckFilePath.toStdString() );
    ASSERT_TRUE( deckLoaded ) << "Failed to load deck file";

    // Create BCCON keyword using factory and replace in deck
    Opm::DeckKeyword bcconKw    = RimKeywordFactory::bcconKeyword( borderCellFaces );
    bool             bcconAdded = deckFile.replaceKeyword( "GRID", bcconKw );
    ASSERT_TRUE( bcconAdded ) << "Failed to replace BCCON keyword";

    // Step 7: Save deck and verify format
    QString outputDeckPath = tempDir.filePath( "output_bccon.DATA" );
    bool    deckSaved      = deckFile.saveDeck( tempDir.path().toStdString(), "output_bccon.DATA" );
    ASSERT_TRUE( deckSaved ) << "Failed to save deck file";
    ASSERT_TRUE( QFile::exists( outputDeckPath ) ) << "Output deck file not created";

    // Read and verify BCCON content
    QFile outputFile( outputDeckPath );
    ASSERT_TRUE( outputFile.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString content = QTextStream( &outputFile ).readAll();
    outputFile.close();

    // Verify BCCON keyword is present
    EXPECT_TRUE( content.contains( "BCCON" ) ) << "BCCON keyword not found in output";

    // Verify at least one entry exists
    // BCCON format: counter i1 i2 j1 j2 k1 k2 face
    // Should have face directions like X, X-, Y, Y-, Z, Z-
    bool hasFaceDirection = content.contains( "X-" ) || content.contains( "Y-" ) || content.contains( "Z-" ) || content.contains( " X " ) ||
                            content.contains( " Y " ) || content.contains( " Z " );
    EXPECT_TRUE( hasFaceDirection ) << "BCCON entries don't contain face directions";
}

//--------------------------------------------------------------------------------------------------
/// Test BCCON result generation with face numbering 1-6
///
/// This test verifies that:
/// 1. We can generate BORDNUM result identifying border and interior cells
/// 2. generateBcconResult() assigns values 1-6 to border cells based on which face of the box they're on
/// 3. Values are assigned correctly: 1=I-, 2=I+, 3=J-, 4=J+, 5=K-, 6=K+
///
/// Test process:
/// 1. Load test model
/// 2. Create custom visibility for a box region
/// 3. Generate BORDNUM result
/// 4. Generate BCCON result with box bounds
/// 5. Verify border cells have correct BCCON values (1-6) based on their position
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseResultToolsTest, BcconResultWithFaceNumbering )
{
    // Setup test data directory
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "Case_with_10_timesteps/Real0" );
    ASSERT_TRUE( subFolderExists ) << "Test model directory not found";

    QString inputFilename( "BRUGGE_0000.EGRID" );
    QString inputFilePath = baseFolder.absoluteFilePath( inputFilename );
    ASSERT_TRUE( QFile::exists( inputFilePath ) ) << "Test model file not found: " << inputFilePath.toStdString();

    // Step 1: Load grid
    std::unique_ptr<RimEclipseResultCase> testCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( testCase.get() );

    cvf::ref<RifReaderEclipseOutput> readerInterfaceEcl = new RifReaderEclipseOutput;
    bool                             success            = readerInterfaceEcl->open( inputFilePath, caseData.p() );
    ASSERT_TRUE( success ) << "Failed to load grid";

    testCase->setReservoirData( caseData.p() );

    const RigMainGrid* grid = caseData->mainGrid();
    ASSERT_NE( grid, nullptr ) << "Grid is null";

    // Step 2: Create custom visibility for a box region
    size_t cellCount = grid->cellCount();

    cvf::ref<cvf::UByteArray> customVisibility = new cvf::UByteArray( cellCount );
    customVisibility->setAll( 0 ); // Start with all invisible

    // Define a box in the middle of the grid
    size_t startI = grid->cellCountI() / 4;
    size_t endI   = 3 * grid->cellCountI() / 4;
    size_t startJ = grid->cellCountJ() / 4;
    size_t endJ   = 3 * grid->cellCountJ() / 4;
    size_t startK = grid->cellCountK() / 4;
    size_t endK   = 3 * grid->cellCountK() / 4;

    cvf::Vec3st min( startI, startJ, startK );
    cvf::Vec3st max( endI - 1, endJ - 1, endK - 1 );

    // Make the box visible
    for ( size_t i = startI; i < endI; ++i )
    {
        for ( size_t j = startJ; j < endJ; ++j )
        {
            for ( size_t k = startK; k < endK; ++k )
            {
                size_t cellIndex                 = grid->cellIndexFromIJK( i, j, k );
                ( *customVisibility )[cellIndex] = 1;
            }
        }
    }

    // Step 3: Generate BORDNUM result
    RigEclipseResultTools::generateBorderResult( testCase.get(), customVisibility, "BORDNUM" );

    // Verify BORDNUM was created
    auto resultsData = testCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_NE( resultsData, nullptr );

    RigEclipseResultAddress bordNumAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, "BORDNUM" );
    ASSERT_TRUE( resultsData->hasResultEntry( bordNumAddr ) ) << "BORDNUM result not created";

    // Step 4: Generate BCCON result
    RigEclipseResultTools::generateBcconResult( testCase.get(), min, max );

    // Verify BCCON was created
    RigEclipseResultAddress bcconAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, "BCCON" );
    ASSERT_TRUE( resultsData->hasResultEntry( bcconAddr ) ) << "BCCON result not created";

    // Get BCCON and BORDNUM values
    resultsData->ensureKnownResultLoaded( bcconAddr );
    resultsData->ensureKnownResultLoaded( bordNumAddr );

    auto bcconValues   = resultsData->cellScalarResults( bcconAddr, 0 );
    auto bordNumValues = resultsData->cellScalarResults( bordNumAddr, 0 );

    ASSERT_FALSE( bcconValues.empty() ) << "BCCON values are empty";
    ASSERT_FALSE( bordNumValues.empty() ) << "BORDNUM values are empty";

    // Step 5: Verify BCCON values are correct
    int countI_minus = 0, countI_plus = 0;
    int countJ_minus = 0, countJ_plus = 0;
    int countK_minus = 0, countK_plus = 0;

    for ( size_t i = startI; i < endI; ++i )
    {
        for ( size_t j = startJ; j < endJ; ++j )
        {
            for ( size_t k = startK; k < endK; ++k )
            {
                size_t cellIndex = grid->cellIndexFromIJK( i, j, k );

                int borderValue = static_cast<int>( bordNumValues[cellIndex] );
                int bcconValue  = static_cast<int>( bcconValues[cellIndex] );

                // Only check border cells
                if ( borderValue == RigEclipseResultTools::BorderType::BORDER_CELL )
                {
                    // Verify BCCON value is in valid range 1-6
                    EXPECT_GE( bcconValue, 1 ) << "BCCON value out of range at (" << i << "," << j << "," << k << ")";
                    EXPECT_LE( bcconValue, 6 ) << "BCCON value out of range at (" << i << "," << j << "," << k << ")";

                    // Check specific face values
                    if ( i == min.x() )
                    {
                        EXPECT_EQ( bcconValue, 1 ) << "I- face should have BCCON=1 at (" << i << "," << j << "," << k << ")";
                        countI_minus++;
                    }
                    else if ( i == max.x() )
                    {
                        EXPECT_EQ( bcconValue, 2 ) << "I+ face should have BCCON=2 at (" << i << "," << j << "," << k << ")";
                        countI_plus++;
                    }
                    else if ( j == min.y() )
                    {
                        EXPECT_EQ( bcconValue, 3 ) << "J- face should have BCCON=3 at (" << i << "," << j << "," << k << ")";
                        countJ_minus++;
                    }
                    else if ( j == max.y() )
                    {
                        EXPECT_EQ( bcconValue, 4 ) << "J+ face should have BCCON=4 at (" << i << "," << j << "," << k << ")";
                        countJ_plus++;
                    }
                    else if ( k == min.z() )
                    {
                        EXPECT_EQ( bcconValue, 5 ) << "K- face should have BCCON=5 at (" << i << "," << j << "," << k << ")";
                        countK_minus++;
                    }
                    else if ( k == max.z() )
                    {
                        EXPECT_EQ( bcconValue, 6 ) << "K+ face should have BCCON=6 at (" << i << "," << j << "," << k << ")";
                        countK_plus++;
                    }
                }
                else
                {
                    // Interior cells should have BCCON=0
                    EXPECT_EQ( bcconValue, 0 ) << "Interior cell should have BCCON=0 at (" << i << "," << j << "," << k << ")";
                }
            }
        }
    }

    // Verify we found border cells on all 6 faces
    EXPECT_GT( countI_minus, 0 ) << "No border cells found on I- face";
    EXPECT_GT( countI_plus, 0 ) << "No border cells found on I+ face";
    EXPECT_GT( countJ_minus, 0 ) << "No border cells found on J- face";
    EXPECT_GT( countJ_plus, 0 ) << "No border cells found on J+ face";
    EXPECT_GT( countK_minus, 0 ) << "No border cells found on K- face";
    EXPECT_GT( countK_plus, 0 ) << "No border cells found on K+ face";
}
