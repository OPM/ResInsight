/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RiaTestDataDirectory.h"
#include "RifEclipseInputFileTools.h"
#include "RigResdataGridConverter.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RimEclipseResultCase.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>

//--------------------------------------------------------------------------------------------------
/// Test that verifies the native grid export functionality works correctly
///
/// This test addresses issue #12859: Remove resdata dependency from sector model export
///
/// Test process:
/// 1. Load reek test model (reek_box_grid_w_props.grdecl)
/// 2. Export grid using new native implementation
/// 3. Read back the exported grid
/// 4. Compare key properties between original and exported grids
//--------------------------------------------------------------------------------------------------
TEST( RigResdataGridConverterTest, NativeGridExportRoundTrip )
{
    // Setup test data directory
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "reek" );
    ASSERT_TRUE( subFolderExists ) << "Test model directory not found";

    QString inputFilename( "reek_box_grid_w_props.grdecl" );
    QString inputFilePath = baseFolder.absoluteFilePath( inputFilename );
    ASSERT_TRUE( QFile::exists( inputFilePath ) ) << "Test model file not found: " << inputFilePath.toStdString();

    // Step 1: Load original grid
    std::unique_ptr<RimEclipseResultCase> originalCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          originalCaseData = new RigEclipseCaseData( originalCase.get() );

    QString errorMessages;
    bool    loadResult = RifEclipseInputFileTools::openGridFile( inputFilePath, originalCaseData.p(), false, &errorMessages );
    ASSERT_TRUE( loadResult ) << "Failed to load original grid: " << errorMessages.toStdString();

    const RigMainGrid* originalGrid = originalCaseData->mainGrid();
    ASSERT_NE( originalGrid, nullptr ) << "Original grid is null";

    // Record original grid properties
    size_t originalNI          = originalGrid->cellCountI();
    size_t originalNJ          = originalGrid->cellCountJ();
    size_t originalNK          = originalGrid->cellCountK();
    size_t originalCellCount   = originalGrid->cellCount();
    bool   originalUsesMapAxes = originalGrid->useMapAxes();
    auto   originalMapAxes     = originalGrid->mapAxesF();
    auto   originalBoundingBox = originalGrid->boundingBox();

    EXPECT_GT( originalNI, 0 ) << "Original grid has no I cells";
    EXPECT_GT( originalNJ, 0 ) << "Original grid has no J cells";
    EXPECT_GT( originalNK, 0 ) << "Original grid has no K cells";
    EXPECT_GT( originalCellCount, 0 ) << "Original grid has no cells";

    // Step 2: Export grid using new native implementation
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() ) << "Failed to create temporary directory";

    QString exportFilePath = tempDir.filePath( "exported_grid.grdecl" );

    bool exportResult = RigResdataGridConverter::exportGrid( exportFilePath,
                                                             originalCaseData.p(),
                                                             false, // exportInLocalCoordinates
                                                             nullptr, // cellVisibilityOverrideForActnum
                                                             cvf::Vec3st::ZERO, // min
                                                             cvf::Vec3st::UNDEFINED, // max (use full grid)
                                                             cvf::Vec3st( 1, 1, 1 ) // refinement (no refinement)
    );

    ASSERT_TRUE( exportResult ) << "Grid export failed";
    ASSERT_TRUE( QFile::exists( exportFilePath ) ) << "Exported grid file not created";

    // Verify exported file has required keywords
    QFile exportedFile( exportFilePath );
    ASSERT_TRUE( exportedFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) << "Cannot read exported file";

    QString exportedContent = QTextStream( &exportedFile ).readAll();
    exportedFile.close();

    EXPECT_TRUE( exportedContent.contains( "SPECGRID" ) ) << "Exported file missing SPECGRID keyword";
    EXPECT_TRUE( exportedContent.contains( "COORD" ) ) << "Exported file missing COORD keyword";
    EXPECT_TRUE( exportedContent.contains( "ZCORN" ) ) << "Exported file missing ZCORN keyword";
    EXPECT_TRUE( exportedContent.contains( "ACTNUM" ) ) << "Exported file missing ACTNUM keyword";

    if ( originalUsesMapAxes )
    {
        EXPECT_TRUE( exportedContent.contains( "MAPAXES" ) ) << "Exported file missing MAPAXES keyword";
    }

    // Step 3: Read back exported grid
    std::unique_ptr<RimEclipseResultCase> exportedCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          exportedCaseData = new RigEclipseCaseData( exportedCase.get() );

    QString readBackErrorMessages;
    bool    readBackResult = RifEclipseInputFileTools::openGridFile( exportFilePath, exportedCaseData.p(), false, &readBackErrorMessages );
    ASSERT_TRUE( readBackResult ) << "Failed to read back exported grid: " << readBackErrorMessages.toStdString();

    const RigMainGrid* exportedGrid = exportedCaseData->mainGrid();
    ASSERT_NE( exportedGrid, nullptr ) << "Exported grid is null";

    // Step 4: Compare original and exported grids
    EXPECT_EQ( originalNI, exportedGrid->cellCountI() ) << "Grid I dimension mismatch";
    EXPECT_EQ( originalNJ, exportedGrid->cellCountJ() ) << "Grid J dimension mismatch";
    EXPECT_EQ( originalNK, exportedGrid->cellCountK() ) << "Grid K dimension mismatch";
    EXPECT_EQ( originalCellCount, exportedGrid->cellCount() ) << "Grid cell count mismatch";
    EXPECT_EQ( originalUsesMapAxes, exportedGrid->useMapAxes() ) << "MAPAXES usage mismatch";

    if ( originalUsesMapAxes && exportedGrid->useMapAxes() )
    {
        auto exportedMapAxes = exportedGrid->mapAxesF();
        for ( size_t i = 0; i < 6; ++i )
        {
            EXPECT_NEAR( originalMapAxes[i], exportedMapAxes[i], 0.01 ) << "MAPAXES value " << i << " mismatch";
        }
    }

    auto exportedBoundingBox = exportedGrid->boundingBox();
    EXPECT_NEAR( originalBoundingBox.min().x(), exportedBoundingBox.min().x(), 1.0 ) << "Bounding box min X mismatch";
    EXPECT_NEAR( originalBoundingBox.min().y(), exportedBoundingBox.min().y(), 1.0 ) << "Bounding box min Y mismatch";
    EXPECT_NEAR( originalBoundingBox.min().z(), exportedBoundingBox.min().z(), 1.0 ) << "Bounding box min Z mismatch";
    EXPECT_NEAR( originalBoundingBox.max().x(), exportedBoundingBox.max().x(), 1.0 ) << "Bounding box max X mismatch";
    EXPECT_NEAR( originalBoundingBox.max().y(), exportedBoundingBox.max().y(), 1.0 ) << "Bounding box max Y mismatch";
    EXPECT_NEAR( originalBoundingBox.max().z(), exportedBoundingBox.max().z(), 1.0 ) << "Bounding box max Z mismatch";
}

//--------------------------------------------------------------------------------------------------
/// Full round-trip test
//--------------------------------------------------------------------------------------------------
TEST( RigResdataGridConverterTest, FullRoundTrip )
{
    // Setup test data directory
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "reek" );
    ASSERT_TRUE( subFolderExists );

    QString inputFilePath = baseFolder.absoluteFilePath( "reek_box_grid_w_props.grdecl" );
    ASSERT_TRUE( QFile::exists( inputFilePath ) );

    // Load original grid
    std::unique_ptr<RimEclipseResultCase> originalCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          originalCaseData = new RigEclipseCaseData( originalCase.get() );

    QString errorMessages;
    bool    loadResult = RifEclipseInputFileTools::openGridFile( inputFilePath, originalCaseData.p(), false, &errorMessages );
    ASSERT_TRUE( loadResult );

    const RigMainGrid* originalGrid        = originalCaseData->mainGrid();
    auto               originalMapAxes     = originalGrid->mapAxesF();
    auto               originalBoundingBox = originalGrid->boundingBox();

    // Export grid
    QTemporaryDir tempDir;
    QString       exportFilePath = tempDir.filePath( "exported_grid.grdecl" );

    bool exportResult = RigResdataGridConverter::exportGrid( exportFilePath,
                                                             originalCaseData.p(),
                                                             false,
                                                             nullptr,
                                                             cvf::Vec3st::ZERO,
                                                             cvf::Vec3st::UNDEFINED,
                                                             cvf::Vec3st( 1, 1, 1 ) );
    ASSERT_TRUE( exportResult );

    // Read back exported grid
    std::unique_ptr<RimEclipseResultCase> exportedCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          exportedCaseData = new RigEclipseCaseData( exportedCase.get() );

    QString readBackErrorMessages;
    bool    readBackResult = RifEclipseInputFileTools::openGridFile( exportFilePath, exportedCaseData.p(), false, &readBackErrorMessages );
    ASSERT_TRUE( readBackResult ) << "Read back failed: " << readBackErrorMessages.toStdString();

    const RigMainGrid* exportedGrid = exportedCaseData->mainGrid();

    // Compare grids
    EXPECT_EQ( originalGrid->cellCountI(), exportedGrid->cellCountI() );
    EXPECT_EQ( originalGrid->cellCountJ(), exportedGrid->cellCountJ() );
    EXPECT_EQ( originalGrid->cellCountK(), exportedGrid->cellCountK() );
    EXPECT_EQ( originalGrid->useMapAxes(), exportedGrid->useMapAxes() );

    if ( originalGrid->useMapAxes() && exportedGrid->useMapAxes() )
    {
        auto exportedMapAxes = exportedGrid->mapAxesF();
        for ( size_t i = 0; i < 6; ++i )
        {
            EXPECT_NEAR( originalMapAxes[i], exportedMapAxes[i], 0.01 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test grid export with refinement
//--------------------------------------------------------------------------------------------------
TEST( RigResdataGridConverterTest, GridExportWithRefinement )
{
    // Setup test data
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "reek" );
    ASSERT_TRUE( subFolderExists );

    QString inputFilePath = baseFolder.absoluteFilePath( "reek_box_grid_w_props.grdecl" );
    ASSERT_TRUE( QFile::exists( inputFilePath ) );

    // Load original grid
    std::unique_ptr<RimEclipseResultCase> originalCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          originalCaseData = new RigEclipseCaseData( originalCase.get() );

    QString errorMessages;
    bool    loadResult = RifEclipseInputFileTools::openGridFile( inputFilePath, originalCaseData.p(), false, &errorMessages );
    ASSERT_TRUE( loadResult ) << "Failed to load grid: " << errorMessages.toStdString();

    const RigMainGrid* originalGrid = originalCaseData->mainGrid();

    // Test with 2x2x1 refinement (should quadruple the number of cells in I and J directions)
    cvf::Vec3st refinement( 2, 2, 1 );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    QString exportFilePath = tempDir.filePath( "refined_grid.grdecl" );

    bool exportResult = RigResdataGridConverter::exportGrid( exportFilePath,
                                                             originalCaseData.p(),
                                                             false, // exportInLocalCoordinates
                                                             nullptr, // cellVisibilityOverrideForActnum
                                                             cvf::Vec3st::ZERO, // min
                                                             cvf::Vec3st::UNDEFINED, // max
                                                             refinement );

    ASSERT_TRUE( exportResult ) << "Refined grid export failed";
    ASSERT_TRUE( QFile::exists( exportFilePath ) ) << "Refined grid file not created";

    // Read back refined grid
    std::unique_ptr<RimEclipseResultCase> refinedCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          refinedCaseData = new RigEclipseCaseData( refinedCase.get() );

    QString readBackErrorMessages;
    bool    readBackResult = RifEclipseInputFileTools::openGridFile( exportFilePath, refinedCaseData.p(), false, &readBackErrorMessages );
    ASSERT_TRUE( readBackResult ) << "Failed to read back refined grid: " << readBackErrorMessages.toStdString();

    const RigMainGrid* refinedGrid = refinedCaseData->mainGrid();
    ASSERT_NE( refinedGrid, nullptr );

    // Verify refinement worked correctly
    EXPECT_EQ( originalGrid->cellCountI() * refinement.x(), refinedGrid->cellCountI() ) << "Refined I dimension incorrect";
    EXPECT_EQ( originalGrid->cellCountJ() * refinement.y(), refinedGrid->cellCountJ() ) << "Refined J dimension incorrect";
    EXPECT_EQ( originalGrid->cellCountK() * refinement.z(), refinedGrid->cellCountK() ) << "Refined K dimension incorrect";

    size_t expectedRefinedCellCount = originalGrid->cellCount() * refinement.x() * refinement.y() * refinement.z();
    EXPECT_EQ( expectedRefinedCellCount, refinedGrid->cellCount() ) << "Refined cell count incorrect";
}

//--------------------------------------------------------------------------------------------------
/// Test grid export with local coordinates
//--------------------------------------------------------------------------------------------------
TEST( RigResdataGridConverterTest, GridExportWithLocalCoordinates )
{
    // Setup test data
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "reek" );
    ASSERT_TRUE( subFolderExists );

    QString inputFilePath = baseFolder.absoluteFilePath( "reek_box_grid_w_props.grdecl" );
    ASSERT_TRUE( QFile::exists( inputFilePath ) );

    // Load original grid
    std::unique_ptr<RimEclipseResultCase> originalCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          originalCaseData = new RigEclipseCaseData( originalCase.get() );

    QString errorMessages;
    bool    loadResult = RifEclipseInputFileTools::openGridFile( inputFilePath, originalCaseData.p(), false, &errorMessages );
    ASSERT_TRUE( loadResult ) << "Failed to load grid: " << errorMessages.toStdString();

    const RigMainGrid* originalGrid = originalCaseData->mainGrid();

    // Only test local coordinates if original grid uses MAPAXES
    if ( !originalGrid->useMapAxes() )
    {
        return;
    }

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    QString globalExportPath = tempDir.filePath( "global_coords.grdecl" );
    QString localExportPath  = tempDir.filePath( "local_coords.grdecl" );

    // Export with global coordinates
    bool globalExportResult = RigResdataGridConverter::exportGrid( globalExportPath,
                                                                   originalCaseData.p(),
                                                                   false, // exportInLocalCoordinates = false
                                                                   nullptr,
                                                                   cvf::Vec3st::ZERO,
                                                                   cvf::Vec3st::UNDEFINED,
                                                                   cvf::Vec3st( 1, 1, 1 ) );

    // Export with local coordinates
    bool localExportResult = RigResdataGridConverter::exportGrid( localExportPath,
                                                                  originalCaseData.p(),
                                                                  true, // exportInLocalCoordinates = true
                                                                  nullptr,
                                                                  cvf::Vec3st::ZERO,
                                                                  cvf::Vec3st::UNDEFINED,
                                                                  cvf::Vec3st( 1, 1, 1 ) );

    ASSERT_TRUE( globalExportResult ) << "Global coordinates export failed";
    ASSERT_TRUE( localExportResult ) << "Local coordinates export failed";

    // Both files should exist and have different MAPAXES values
    ASSERT_TRUE( QFile::exists( globalExportPath ) );
    ASSERT_TRUE( QFile::exists( localExportPath ) );

    // Read both files and compare MAPAXES
    QFile globalFile( globalExportPath );
    QFile localFile( localExportPath );

    ASSERT_TRUE( globalFile.open( QIODevice::ReadOnly | QIODevice::Text ) );
    ASSERT_TRUE( localFile.open( QIODevice::ReadOnly | QIODevice::Text ) );

    QString globalContent = QTextStream( &globalFile ).readAll();
    QString localContent  = QTextStream( &localFile ).readAll();

    globalFile.close();
    localFile.close();

    // Both should have MAPAXES, but values should be different (local should be shifted)
    EXPECT_TRUE( globalContent.contains( "MAPAXES" ) ) << "Global export missing MAPAXES";
    EXPECT_TRUE( localContent.contains( "MAPAXES" ) ) << "Local export missing MAPAXES";
    EXPECT_NE( globalContent, localContent ) << "Global and local exports should be different";
}

//--------------------------------------------------------------------------------------------------
/// Test grid export with sector selection (partial grid export)
//--------------------------------------------------------------------------------------------------
TEST( RigResdataGridConverterTest, GridExportSectorSelection )
{
    // Setup test data
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "reek" );
    ASSERT_TRUE( subFolderExists );

    QString inputFilePath = baseFolder.absoluteFilePath( "reek_box_grid_w_props.grdecl" );
    ASSERT_TRUE( QFile::exists( inputFilePath ) );

    // Load original grid
    std::unique_ptr<RimEclipseResultCase> originalCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          originalCaseData = new RigEclipseCaseData( originalCase.get() );

    QString errorMessages;
    bool    loadResult = RifEclipseInputFileTools::openGridFile( inputFilePath, originalCaseData.p(), false, &errorMessages );
    ASSERT_TRUE( loadResult ) << "Failed to load grid: " << errorMessages.toStdString();

    const RigMainGrid* originalGrid = originalCaseData->mainGrid();

    // Export a sector (middle half of the grid)
    cvf::Vec3st min( originalGrid->cellCountI() / 4, originalGrid->cellCountJ() / 4, 0 );
    cvf::Vec3st max( 3 * originalGrid->cellCountI() / 4, 3 * originalGrid->cellCountJ() / 4, originalGrid->cellCountK() - 1 );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    QString sectorExportPath = tempDir.filePath( "sector_grid.grdecl" );

    bool sectorExportResult = RigResdataGridConverter::exportGrid( sectorExportPath,
                                                                   originalCaseData.p(),
                                                                   false, // exportInLocalCoordinates
                                                                   nullptr, // cellVisibilityOverrideForActnum
                                                                   min,
                                                                   max,
                                                                   cvf::Vec3st( 1, 1, 1 ) // no refinement
    );

    ASSERT_TRUE( sectorExportResult ) << "Sector export failed";
    ASSERT_TRUE( QFile::exists( sectorExportPath ) ) << "Sector grid file not created";

    // Read back sector grid
    std::unique_ptr<RimEclipseResultCase> sectorCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          sectorCaseData = new RigEclipseCaseData( sectorCase.get() );

    QString sectorErrorMessages;
    bool    sectorReadResult = RifEclipseInputFileTools::openGridFile( sectorExportPath, sectorCaseData.p(), false, &sectorErrorMessages );
    ASSERT_TRUE( sectorReadResult ) << "Failed to read back sector grid: " << sectorErrorMessages.toStdString();

    const RigMainGrid* sectorGrid = sectorCaseData->mainGrid();
    ASSERT_NE( sectorGrid, nullptr );

    // Verify sector dimensions
    size_t expectedNI = max.x() - min.x() + 1;
    size_t expectedNJ = max.y() - min.y() + 1;
    size_t expectedNK = max.z() - min.z() + 1;

    EXPECT_EQ( expectedNI, sectorGrid->cellCountI() ) << "Sector I dimension incorrect";
    EXPECT_EQ( expectedNJ, sectorGrid->cellCountJ() ) << "Sector J dimension incorrect";
    EXPECT_EQ( expectedNK, sectorGrid->cellCountK() ) << "Sector K dimension incorrect";

    size_t expectedSectorCellCount = expectedNI * expectedNJ * expectedNK;
    EXPECT_EQ( expectedSectorCellCount, sectorGrid->cellCount() ) << "Sector cell count incorrect";
}

//--------------------------------------------------------------------------------------------------
/// Test conversion of corner point arrays directly
//--------------------------------------------------------------------------------------------------
TEST( RigResdataGridConverterTest, CornerPointArrayConversion )
{
    // Create a simple 2x2x2 test grid
    size_t ni = 2, nj = 2, nk = 2;

    // Mock a simple eclipse case with basic grid
    std::unique_ptr<RimEclipseResultCase> testCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          testCaseData = new RigEclipseCaseData( testCase.get() );

    // This is a unit test for the array conversion function specifically
    // Since createHexCornerCoords is complex, we'll test the interface and basic functionality
    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    // For this test, we need a real grid. Let's load the reek model and test a small subset
    QDir baseFolder( TEST_MODEL_DIR );
    bool subFolderExists = baseFolder.cd( "reek" );
    ASSERT_TRUE( subFolderExists );

    QString inputFilePath = baseFolder.absoluteFilePath( "reek_box_grid_w_props.grdecl" );
    QString errorMessages;
    bool    loadResult = RifEclipseInputFileTools::openGridFile( inputFilePath, testCaseData.p(), false, &errorMessages );
    ASSERT_TRUE( loadResult );

    const RigMainGrid*       grid           = testCaseData->mainGrid();
    const RigActiveCellInfo* activeCellInfo = testCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    // Test conversion for a small subset (first 2x2x2 cells)
    cvf::Vec3st min( 0, 0, 0 );
    cvf::Vec3st max( 1, 1, 1 );
    cvf::Vec3st refinement( 1, 1, 1 );

    cvf::Mat4d mapAxisTransform = grid->mapAxisTransform();

    RigResdataGridConverter::convertGridToCornerPointArrays( testCaseData.p(),
                                                             activeCellInfo,
                                                             nullptr, // cellVisibilityOverrideForActnum
                                                             min,
                                                             max,
                                                             refinement,
                                                             mapAxisTransform,
                                                             grid->useMapAxes(),
                                                             ni,
                                                             nj,
                                                             nk,
                                                             coordArray,
                                                             zcornArray,
                                                             actnumArray );

    // Verify array sizes
    size_t expectedCoordSize  = ( ni + 1 ) * ( nj + 1 ) * 6; // (3*3*6 = 54)
    size_t expectedZcornSize  = ni * nj * nk * 8; // (2*2*2*8 = 64)
    size_t expectedActnumSize = ni * nj * nk; // (2*2*2 = 8)

    EXPECT_EQ( expectedCoordSize, coordArray.size() ) << "COORD array size incorrect";
    EXPECT_EQ( expectedZcornSize, zcornArray.size() ) << "ZCORN array size incorrect";
    EXPECT_EQ( expectedActnumSize, actnumArray.size() ) << "ACTNUM array size incorrect";

    // Verify that arrays have reasonable values (no NaN, not all zeros)
    bool hasNonZeroCoord = false;
    bool hasNonZeroZcorn = false;
    bool hasActiveCell   = false;

    for ( float coord : coordArray )
    {
        EXPECT_FALSE( std::isnan( coord ) ) << "COORD array contains NaN values";
        if ( coord != 0.0f ) hasNonZeroCoord = true;
    }

    for ( float zcorn : zcornArray )
    {
        EXPECT_FALSE( std::isnan( zcorn ) ) << "ZCORN array contains NaN values";
        if ( zcorn != 0.0f ) hasNonZeroZcorn = true;
    }

    for ( int actnum : actnumArray )
    {
        EXPECT_TRUE( actnum == 0 || actnum == 1 ) << "ACTNUM array contains invalid values";
        if ( actnum == 1 ) hasActiveCell = true;
    }

    EXPECT_TRUE( hasNonZeroCoord ) << "COORD array is all zeros";
    EXPECT_TRUE( hasNonZeroZcorn ) << "ZCORN array is all zeros";
    EXPECT_TRUE( hasActiveCell ) << "No active cells found";
}
