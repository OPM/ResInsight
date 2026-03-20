/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RigSimulationInputTool.h"

#include "RiaTestDataDirectory.h"
#include "RifOpmDeckTools.h"
#include "RifOpmFlowDeckFile.h"
#include "RifReaderEclipseOutput.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCaseDataTools.h"
#include "RigMainGrid.h"
#include "RigModelPaddingSettings.h"
#include "RigSimulationInputSettings.h"
#include "RimEclipseResultCase.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Helper to create an EQUALS DeckRecord
/// EQUALS format: FIELD VALUE I1 I2 J1 J2 K1 K2 (1-based Eclipse coordinates)
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createEqualsRecord( const std::string& fieldName, int value, int i1, int i2, int j1, int j2, int k1, int k2 )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", fieldName ) );
    items.push_back( RifOpmDeckTools::item( "VALUE", value ) );
    items.push_back( RifOpmDeckTools::item( "I1", i1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", i2 ) );
    items.push_back( RifOpmDeckTools::item( "J1", j1 ) );
    items.push_back( RifOpmDeckTools::item( "J2", j2 ) );
    items.push_back( RifOpmDeckTools::item( "K1", k1 ) );
    items.push_back( RifOpmDeckTools::item( "K2", k2 ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record with no overlap - should return error
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_NoOverlap )
{
    // Sector: sectorMin=(0, 15, 0), sectorMax=(19, 29, 9) (inclusive, cells [0-19, 15-29, 0-9])
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record: FIPNUM 1 1 20 1 14 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[0-13], K[0-9]
    // Sector J is [15-29], so no overlap in J dimension
    auto record = createEqualsRecord( "FIPNUM", 1, 1, 20, 1, 14, 1, 10 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record with partial overlap requiring clamping
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_PartialOverlapWithClamping )
{
    // Sector: sectorMin=(0, 15, 0), sectorMax=(19, 29, 9) (inclusive, cells [0-19, 15-29, 0-9])
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record: FIPNUM 2 1 20 15 30 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[14-29], K[0-9]
    // Sector: I[0-19], J[15-29], K[0-9]
    // Intersection (clamped): I[0-19], J[15-29], K[0-9]
    auto record = createEqualsRecord( "FIPNUM", 2, 1, 20, 15, 30, 1, 10 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Check transformed coordinates (sector-relative, 1-based)
    // After clamping: I[0-19], J[15-29], K[0-9] (0-based)
    // Transformed to sector-relative (min at 0,15,0):
    //   I: (0-0)*1+1 = 1 to (19-0)*1+1 = 20
    //   J: (15-15)*1+1 = 1 to (29-15)*1+1 = 15
    //   K: (0-0)*1+1 = 1 to (9-0)*1+1 = 10
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2

    // Check field name and value are preserved
    EXPECT_EQ( "FIPNUM", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_EQ( 2, result->getItem( 1 ).get<int>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record completely inside sector (no clamping)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_CompletelyInside )
{
    // Sector: min=(0, 0, 0), max=(19, 19, 9) (inclusive, cells [0-19, 0-19, 0-9])
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 19, 19, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record completely inside: FIPNUM 3 5 15 5 15 2 8 (1-based Eclipse)
    // Converts to 0-based: I[4-14], J[4-14], K[1-7]
    auto record = createEqualsRecord( "FIPNUM", 3, 5, 15, 5, 15, 2, 8 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // No clamping, direct transformation (sector min is 0,0,0)
    // I: (4-0)*1+1 = 5 to (14-0)*1+1 = 15
    // J: (4-0)*1+1 = 5 to (14-0)*1+1 = 15
    // K: (1-0)*1+1 = 2 to (7-0)*1+1 = 8
    EXPECT_EQ( 5, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 15, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 5, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 2, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 8, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record completely outside sector
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_CompletelyOutside )
{
    // Sector: min=(0, 0, 0), max=(9, 9, 9) (inclusive, cells [0-9, 0-9, 0-9])
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record outside: FIPNUM 4 20 30 20 30 1 10 (1-based Eclipse)
    // Converts to 0-based: I[19-29], J[19-29], K[0-9]
    // No overlap with sector
    auto record = createEqualsRecord( "FIPNUM", 4, 20, 30, 20, 30, 1, 10 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test invalid EQUALS record (insufficient items)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_InvalidRecord )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // Create a record with only 1 item (field name, missing value)
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", std::string( "FIPNUM" ) ) );
    // Missing VALUE and box coordinates

    Opm::DeckRecord record{ std::move( items ) };

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "insufficient items" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record without box definition (only field and value)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_NoBoxDefinition )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // Create a record with only field name and value (no box coordinates)
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", std::string( "FIPNUM" ) ) );
    items.push_back( RifOpmDeckTools::item( "VALUE", 1 ) );

    Opm::DeckRecord record{ std::move( items ) };

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_TRUE( result.has_value() );
    EXPECT_EQ( result->size(), 2U );
    EXPECT_EQ( result->getItem( 0 ).get<std::string>( 0 ), "FIPNUM" );
    EXPECT_EQ( result->getItem( 1 ).get<int>( 0 ), 1 );
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record at sector boundary
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_AtBoundary )
{
    // Sector: min=(5, 5, 5), max=(14, 14, 14) (inclusive, cells [5-14, 5-14, 5-14])
    caf::VecIjk0 sectorMin( 5, 5, 5 );
    caf::VecIjk0 sectorMax( 14, 14, 14 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record exactly matching sector: 6 15 6 15 6 15 (1-based Eclipse)
    // Converts to 0-based: I[5-14], J[5-14], K[5-14]
    auto record = createEqualsRecord( "FIPNUM", 5, 6, 15, 6, 15, 6, 15 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Should span entire sector in all dimensions
    // Transformed: (5-5)*1+1 = 1 to (14-5)*1+1 = 10
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 10, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 10, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Helper to create an AQUCON DeckRecord
/// AQUCON format: ID I1 I2 J1 J2 K1 K2 CONNECT_FACE ... (1-based Eclipse coordinates)
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createAquconRecord( int aquiferId, int i1, int i2, int j1, int j2, int k1, int k2, const std::string& connectFace )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "ID", aquiferId ) );
    items.push_back( RifOpmDeckTools::item( "I1", i1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", i2 ) );
    items.push_back( RifOpmDeckTools::item( "J1", j1 ) );
    items.push_back( RifOpmDeckTools::item( "J2", j2 ) );
    items.push_back( RifOpmDeckTools::item( "K1", k1 ) );
    items.push_back( RifOpmDeckTools::item( "K2", k2 ) );
    items.push_back( RifOpmDeckTools::item( "CONNECT_FACE", connectFace ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record with partial overlap (based on user-provided data)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAquconRecord_PartialOverlap )
{
    // Sector: min=(40, 27, 21), max=(100, 72, 58) (inclusive, 0-based)
    // This corresponds to a sector with cells [40-100, 27-72, 21-58]
    caf::VecIjk0 sectorMin( 40, 27, 21 );
    caf::VecIjk0 sectorMax( 100, 72, 58 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // AQUCON record from user data: 1 57 57 28 36 46 58 'I+' (1-based Eclipse)
    // Converts to 0-based: I[56-56], J[27-35], K[45-57]
    // Sector is I[40-100], J[27-72], K[21-58]
    // Overlap: I[56-56], J[27-35], K[45-57] - all inside, no clamping needed
    auto record = createAquconRecord( 1, 57, 57, 28, 36, 46, 58, "I+" );

    auto result = RigSimulationInputTool::processAquconRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Check transformed coordinates (sector-relative, 1-based)
    // I: (56-40)*1+1 = 17 to (56-40)*1+1 = 17
    // J: (27-27)*1+1 = 1 to (35-27)*1+1 = 9
    // K: (45-21)*1+1 = 25 to (57-21)*1+1 = 37
    EXPECT_EQ( 17, result->getItem( 1 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 17, result->getItem( 2 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 3 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 9, result->getItem( 4 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 25, result->getItem( 5 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 37, result->getItem( 6 ).get<int>( 0 ) ); // K2

    // Check aquifer ID and connect face are preserved
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) );
    EXPECT_EQ( "I+", result->getItem( 7 ).get<std::string>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record with partial K overlap requiring clamping
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAquconRecord_PartialOverlapWithClamping )
{
    // Sector: min=(40, 27, 21), max=(100, 72, 58) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 40, 27, 21 );
    caf::VecIjk0 sectorMax( 100, 72, 58 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // AQUCON record: 1 79 79 41 67 5 11 'I+' (1-based Eclipse)
    // Converts to 0-based: I[78-78], J[40-66], K[4-10]
    // Sector K is [21-58], so K[4-10] is outside and should be skipped
    auto record = createAquconRecord( 1, 79, 79, 41, 67, 5, 11, "I+" );

    auto result = RigSimulationInputTool::processAquconRecord( record, sectorMin, sectorMax, refinement );

    // This should fail because K range [4-10] doesn't overlap with sector K range [21-58]
    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record completely inside sector
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAquconRecord_CompletelyInside )
{
    // Sector: min=(40, 27, 21), max=(100, 72, 58) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 40, 27, 21 );
    caf::VecIjk0 sectorMax( 100, 72, 58 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // AQUCON record: 1 61 61 48 72 12 17 'I+' (1-based Eclipse)
    // Converts to 0-based: I[60-60], J[47-71], K[11-16]
    // However, K[11-16] is outside sector K[21-58], so this should fail
    auto record = createAquconRecord( 1, 61, 61, 48, 72, 12, 17, "I+" );

    auto result = RigSimulationInputTool::processAquconRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record without box definition (only aquifer ID)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAquconRecord_NoBoxDefinition )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // Create a record with only aquifer ID (no box coordinates)
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "ID", 1 ) );

    Opm::DeckRecord record{ std::move( items ) };

    auto result = RigSimulationInputTool::processAquconRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_TRUE( result.has_value() );
    EXPECT_EQ( result->size(), 1U );
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5 )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model.DATA";

    // Set up export settings for sector export
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 19, 14, 9 ) ); // Sector (0-based inclusive)
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) ); // No refinement
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    // Get all keywords from the deck
    std::vector<std::string> allKeywords = deckFile.keywords( false );

    // Verify key keywords exist in the file
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "DIMENS" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "SPECGRID" ) != allKeywords.end() )
        << "SPECGRID keyword missing from exported file";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "COORD" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ZCORN" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ACTNUM" ) != allKeywords.end() );

    // Verify OPERNUM and OPERATER keywords exist (for boundary conditions with default OPERNUM_OPERATER)
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "OPERNUM" ) != allKeywords.end() )
        << "OPERNUM keyword missing from exported file";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "OPERATER" ) != allKeywords.end() )
        << "OPERATER keyword missing from exported file";

    // Read the file content to verify dimensions
    QFile file( exportFilePath );
    ASSERT_TRUE( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString fileContent = file.readAll();
    file.close();

    // Verify dimensions are correct for the sector (20x15x10)
    // Max is (19,14,9) which means 20 cells in I, 15 cells in J, 10 cells in K
    EXPECT_TRUE( fileContent.contains( " 20 15 10 /" ) ) << "File does not contain expected sector dimensions 20 15 10";
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data using BCCON_BCPROP boundary condition
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5WithBcconBcprop )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_bccon.DATA";

    // Set up export settings for sector export with BCCON_BCPROP boundary condition
    // Use a smaller sector (not starting at 0,0,0) to ensure border cells are created
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 5, 5, 2 ) );
    settings.setMax( caf::VecIjk0( 14, 14, 7 ) ); // Sector (0-based inclusive) - 10x10x6
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) ); // No refinement
    settings.setBoundaryCondition( RiaModelExportDefines::BCCON_BCPROP );
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    // Get all keywords from the deck
    std::vector<std::string> allKeywords = deckFile.keywords( false );

    // Verify key keywords exist in the file
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "DIMENS" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "SPECGRID" ) != allKeywords.end() )
        << "SPECGRID keyword missing from exported file";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "COORD" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ZCORN" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ACTNUM" ) != allKeywords.end() );

    // Verify BCCON and BCPROP keywords exist (for boundary conditions with BCCON_BCPROP)
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "BCCON" ) != allKeywords.end() )
        << "BCCON keyword missing from exported file";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "BCPROP" ) != allKeywords.end() )
        << "BCPROP keyword missing from exported file";

    // Verify OPERNUM and OPERATER keywords do NOT exist (should only be present with OPERNUM_OPERATER)
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "OPERNUM" ) == allKeywords.end() )
        << "OPERNUM keyword should not be present when using BCCON_BCPROP";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "OPERATER" ) == allKeywords.end() )
        << "OPERATER keyword should not be present when using BCCON_BCPROP";

    // Read the file content to verify dimensions
    QFile file( exportFilePath );
    ASSERT_TRUE( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString fileContent = file.readAll();
    file.close();

    // Verify dimensions are correct for the sector (10x10x6)
    // Min (5,5,2) to Max (14,14,7) means 10 cells in I, 10 cells in J, 6 cells in K
    EXPECT_TRUE( fileContent.contains( " 10 10 6 /" ) ) << "File does not contain expected sector dimensions 10 10 6";
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data verifying data keyword cropping
/// Ensures PORO, EQLNUM, SATNUM are cropped to sector cell count
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5_DataKeywordCropping )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_datakw.DATA";

    // Set up export settings for a smaller sector to test cropping
    // Sector: min=(5,5,2), max=(14,14,7) => 10x10x6 = 600 cells
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 5, 5, 2 ) );
    settings.setMax( caf::VecIjk0( 14, 14, 7 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    const size_t expectedCellCount = 10 * 10 * 6; // 600 cells

    // Verify PORO keyword is cropped to sector size with correct values
    {
        auto poroKw = deckFile.findKeyword( "PORO" );
        ASSERT_TRUE( poroKw.has_value() ) << "PORO keyword missing from exported file";

        const auto& poroData = poroKw->getRawDoubleData();
        EXPECT_EQ( expectedCellCount, poroData.size() ) << "PORO should have " << expectedCellCount << " values";

        // Original model5 has PORO = 6000*0.28, so all cropped values should be 0.28
        for ( size_t i = 0; i < poroData.size(); ++i )
        {
            EXPECT_DOUBLE_EQ( 0.28, poroData[i] ) << "PORO value at index " << i << " should be 0.28";
        }
    }

    // Verify EQLNUM keyword is cropped to sector size
    {
        auto eqlnumKw = deckFile.findKeyword( "EQLNUM" );
        ASSERT_TRUE( eqlnumKw.has_value() ) << "EQLNUM keyword missing from exported file";

        const auto& eqlnumData = eqlnumKw->getIntData();
        EXPECT_EQ( expectedCellCount, eqlnumData.size() ) << "EQLNUM should have " << expectedCellCount << " values";

        // Original model5 has EQLNUM = 6000*1
        for ( size_t i = 0; i < eqlnumData.size(); ++i )
        {
            EXPECT_EQ( 1, eqlnumData[i] ) << "EQLNUM value at index " << i << " should be 1";
        }
    }

    // Verify SATNUM keyword is cropped to sector size
    {
        auto satnumKw = deckFile.findKeyword( "SATNUM" );
        ASSERT_TRUE( satnumKw.has_value() ) << "SATNUM keyword missing from exported file";

        const auto& satnumData = satnumKw->getIntData();
        EXPECT_EQ( expectedCellCount, satnumData.size() ) << "SATNUM should have " << expectedCellCount << " values";

        // Original model5 has SATNUM = 6000*1
        for ( size_t i = 0; i < satnumData.size(); ++i )
        {
            EXPECT_EQ( 1, satnumData[i] ) << "SATNUM value at index " << i << " should be 1";
        }
    }
}

TEST( RigSimulationInputTool, ExportModel5_DataKeywordCropping_4EqlnumRegions )
{
    // Load model5 test data with 4 EQLNUM regions
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "1_4EQLNUM_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_4eqlnum.DATA";

    // Set up export settings for a smaller sector to test cropping
    // Sector: min=(5,5,2), max=(14,14,7) => 10x10x6 = 600 cells
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 5, 5, 2 ) );
    settings.setMax( caf::VecIjk0( 14, 14, 7 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    const size_t expectedCellCount = 10 * 10 * 6; // 600 cells

    // Verify PORO keyword is cropped to sector size with correct values
    {
        auto poroKw = deckFile.findKeyword( "PORO" );
        ASSERT_TRUE( poroKw.has_value() ) << "PORO keyword missing from exported file";

        const auto& poroData = poroKw->getRawDoubleData();
        EXPECT_EQ( expectedCellCount, poroData.size() ) << "PORO should have " << expectedCellCount << " values";

        for ( size_t i = 0; i < poroData.size(); ++i )
        {
            EXPECT_DOUBLE_EQ( 0.28, poroData[i] ) << "PORO value at index " << i << " should be 0.28";
        }
    }

    // Verify EQLNUM keyword is cropped to sector size with correct region values
    // Full grid EQLNUM: K=1-3 region 1, K=4-5 region 2, K=6-7 region 3, K=8-10 region 4
    // Sector captures K=3-8 (0-based K=2-7), so:
    //   Sector K=0 (full K=3): region 1 -> 100 cells (indices 0-99)
    //   Sector K=1-2 (full K=4-5): region 2 -> 200 cells (indices 100-299)
    //   Sector K=3-4 (full K=6-7): region 3 -> 200 cells (indices 300-499)
    //   Sector K=5 (full K=8): region 4 -> 100 cells (indices 500-599)
    {
        auto eqlnumKw = deckFile.findKeyword( "EQLNUM" );
        ASSERT_TRUE( eqlnumKw.has_value() ) << "EQLNUM keyword missing from exported file";

        const auto& eqlnumData = eqlnumKw->getIntData();
        EXPECT_EQ( expectedCellCount, eqlnumData.size() ) << "EQLNUM should have " << expectedCellCount << " values";

        const size_t layerSize = 10 * 10; // 100 cells per layer

        for ( size_t i = 0; i < eqlnumData.size(); ++i )
        {
            size_t sectorK        = i / layerSize;
            int    expectedRegion = 0;
            if ( sectorK == 0 )
                expectedRegion = 1;
            else if ( sectorK <= 2 )
                expectedRegion = 2;
            else if ( sectorK <= 4 )
                expectedRegion = 3;
            else
                expectedRegion = 4;

            EXPECT_EQ( expectedRegion, eqlnumData[i] )
                << "EQLNUM value at index " << i << " (sector K=" << sectorK << ") should be " << expectedRegion;
        }
    }

    // Verify EQUIL has 4 records (one per EQLNUM region)
    {
        auto equilKw = deckFile.findKeyword( "EQUIL" );
        ASSERT_TRUE( equilKw.has_value() ) << "EQUIL keyword missing from exported file";

        EXPECT_EQ( 4u, equilKw->size() ) << "EQUIL should have 4 records (one per EQLNUM region)";
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper to create a BOX DeckRecord
/// BOX format: I1 I2 J1 J2 K1 K2 (1-based Eclipse coordinates)
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createBoxRecord( int i1, int i2, int j1, int j2, int k1, int k2 )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "I1", i1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", i2 ) );
    items.push_back( RifOpmDeckTools::item( "J1", j1 ) );
    items.push_back( RifOpmDeckTools::item( "J2", j2 ) );
    items.push_back( RifOpmDeckTools::item( "K1", k1 ) );
    items.push_back( RifOpmDeckTools::item( "K2", k2 ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Test BOX record with user-provided dimensions
/// BOX 1 46 1 112 1 22
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessBoxRecord_LargeBox )
{
    // Sector: min=(0, 0, 0), max=(45, 111, 21) (inclusive, 0-based)
    // This means the sector spans exactly the same range as the BOX keyword
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 45, 111, 21 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // BOX record: 1 46 1 112 1 22 (1-based Eclipse)
    // Converts to 0-based: I[0-45], J[0-111], K[0-21]
    auto record = createBoxRecord( 1, 46, 1, 112, 1, 22 );

    auto result = RigSimulationInputTool::processBoxRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Check transformed coordinates (should span entire sector, 1-based)
    // I: (0-0)*1+1 = 1 to (45-0)*1+1 = 46
    // J: (0-0)*1+1 = 1 to (111-0)*1+1 = 112
    // K: (0-0)*1+1 = 1 to (21-0)*1+1 = 22
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 46, result->getItem( 1 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 112, result->getItem( 3 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 22, result->getItem( 5 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test BOX record with refinement (3, 3, 1) and sector offset
/// BOX 1 46 1 112 1 22
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessBoxRecord_LargeBoxWithRefinement )
{
    // Sector: min=(19, 59, 0), max=(45, 111, 21) (inclusive, 0-based)
    // BOX will be clamped to this sector and then refined
    caf::VecIjk0 sectorMin( 19, 59, 0 );
    caf::VecIjk0 sectorMax( 45, 111, 21 );
    cvf::Vec3st  refinement( 3, 3, 1 ); // Refinement: I=3, J=3, K=1

    // BOX record: 1 46 1 112 1 22 (1-based Eclipse)
    // Converts to 0-based: I[0-45], J[0-111], K[0-21]
    // After clamping to sector: I[19-45], J[59-111], K[0-21]
    auto record = createBoxRecord( 1, 46, 1, 112, 1, 22 );

    auto result = RigSimulationInputTool::processBoxRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Check transformed coordinates with refinement (1-based)
    // After clamping: I[19-45], J[59-111], K[0-21] (0-based)
    // Relative to sector min (19, 59, 0): I[0-26], J[0-52], K[0-21]
    // This gives us: 27 cells in I, 53 cells in J, 22 cells in K
    //
    // With refinement (3, 3, 1):
    // - I: 27 cells * 3 = 81 refined cells, 0-based [0-80], 1-based [1-81]
    // - J: 53 cells * 3 = 159 refined cells, 0-based [0-158], 1-based [1-159]
    // - K: 22 cells * 1 = 22 refined cells, 0-based [0-21], 1-based [1-22]
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 81, result->getItem( 1 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 159, result->getItem( 3 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 22, result->getItem( 5 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test BOX record with partial overlap requiring clamping
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessBoxRecord_PartialOverlapWithClamping )
{
    // Sector: min=(10, 20, 5), max=(39, 99, 19) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 10, 20, 5 );
    caf::VecIjk0 sectorMax( 39, 99, 19 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // BOX record: 1 46 1 112 1 22 (1-based Eclipse)
    // Converts to 0-based: I[0-45], J[0-111], K[0-21]
    // Sector: I[10-39], J[20-99], K[5-19]
    // Intersection (clamped): I[10-39], J[20-99], K[5-19]
    auto record = createBoxRecord( 1, 46, 1, 112, 1, 22 );

    auto result = RigSimulationInputTool::processBoxRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Check transformed coordinates (sector-relative, 1-based)
    // After clamping: I[10-39], J[20-99], K[5-19] (0-based)
    // Transformed to sector-relative (min at 10,20,5):
    //   I: (10-10)*1+1 = 1 to (39-10)*1+1 = 30
    //   J: (20-20)*1+1 = 1 to (99-20)*1+1 = 80
    //   K: (5-5)*1+1 = 1 to (19-5)*1+1 = 15
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 30, result->getItem( 1 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 80, result->getItem( 3 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test BOX record completely inside sector (no clamping)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessBoxRecord_CompletelyInside )
{
    // Sector: min=(0, 0, 0), max=(49, 119, 29) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 49, 119, 29 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // BOX record: 5 20 10 50 5 15 (1-based Eclipse)
    // Converts to 0-based: I[4-19], J[9-49], K[4-14]
    // All inside sector, no clamping needed
    auto record = createBoxRecord( 5, 20, 10, 50, 5, 15 );

    auto result = RigSimulationInputTool::processBoxRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // No clamping, direct transformation (sector min is 0,0,0)
    // I: (4-0)*1+1 = 5 to (19-0)*1+1 = 20
    // J: (9-0)*1+1 = 10 to (49-0)*1+1 = 50
    // K: (4-0)*1+1 = 5 to (14-0)*1+1 = 15
    EXPECT_EQ( 5, result->getItem( 0 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 1 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 10, result->getItem( 2 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 50, result->getItem( 3 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 5, result->getItem( 4 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test BOX record with no overlap - should return error
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessBoxRecord_NoOverlap )
{
    // Sector: min=(0, 0, 0), max=(19, 29, 9) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // BOX record: 30 50 40 60 1 10 (1-based Eclipse)
    // Converts to 0-based: I[29-49], J[39-59], K[0-9]
    // No overlap in I and J dimensions with sector
    auto record = createBoxRecord( 30, 50, 40, 60, 1, 10 );

    auto result = RigSimulationInputTool::processBoxRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test invalid BOX record (insufficient items)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessBoxRecord_InvalidRecord )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // Create a record with only 3 items (missing J2, K1, K2)
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "I1", 1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", 5 ) );
    items.push_back( RifOpmDeckTools::item( "J1", 1 ) );

    Opm::DeckRecord record{ std::move( items ) };

    auto result = RigSimulationInputTool::processBoxRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "insufficient items" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test BOX record at sector boundary
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessBoxRecord_AtBoundary )
{
    // Sector: min=(5, 10, 3), max=(24, 39, 12) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 5, 10, 3 );
    caf::VecIjk0 sectorMax( 24, 39, 12 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // BOX record exactly matching sector: 6 25 11 40 4 13 (1-based Eclipse)
    // Converts to 0-based: I[5-24], J[10-39], K[3-12]
    auto record = createBoxRecord( 6, 25, 11, 40, 4, 13 );

    auto result = RigSimulationInputTool::processBoxRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Should span entire sector in all dimensions
    // Transformed: (5-5)*1+1 = 1 to (24-5)*1+1 = 20
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 1 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 30, result->getItem( 3 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 5 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data and refinement (1, 3, 5)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5WithRefinement )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_refined.DATA";

    // Set up export settings for sector export with refinement (1, 3, 5)
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 19, 14, 9 ) ); // Sector (0-based inclusive)
    settings.setRefinement( cvf::Vec3st( 1, 3, 5 ) ); // Refinement: I=1 (no refinement), J=3, K=5
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    // Get all keywords from the deck
    std::vector<std::string> allKeywords = deckFile.keywords( false );

    // Verify key keywords exist in the file
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "DIMENS" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "SPECGRID" ) != allKeywords.end() )
        << "SPECGRID keyword missing from exported file";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "COORD" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ZCORN" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ACTNUM" ) != allKeywords.end() );

    // Read the file content to verify dimensions
    QFile file( exportFilePath );
    ASSERT_TRUE( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString fileContent = file.readAll();
    file.close();

    // Verify dimensions are correct for the refined sector
    // Original sector: 20x15x10 cells
    // With refinement (1, 3, 5): 20*1 x 15*3 x 10*5 = 20 x 45 x 50
    EXPECT_TRUE( fileContent.contains( " 20 45 50 /" ) ) << "File does not contain expected refined dimensions 20 45 50";

    // Verify WELSPECS coordinates are centered in refined cells
    auto welspecsKeywords = deckFile.findAllKeywords( "WELSPECS" );
    ASSERT_FALSE( welspecsKeywords.empty() ) << "WELSPECS keyword not found in exported file";

    std::cout << "\n=== WELSPECS Analysis ===" << std::endl;
    for ( const auto& keyword : welspecsKeywords )
    {
        for ( size_t i = 0; i < keyword.size(); ++i )
        {
            const auto& record   = keyword.getRecord( i );
            std::string wellName = record.getItem( "WELL" ).get<std::string>( 0 );
            int         iHeel    = record.getItem( "HEAD_I" ).get<int>( 0 );
            int         jHeel    = record.getItem( "HEAD_J" ).get<int>( 0 );

            std::cout << "Well: " << wellName << " - WELSPECS I=" << iHeel << ", J=" << jHeel << std::endl;
        }
    }

    // Verify specific well coordinates are centered correctly
    // For refinement (1, 3, 5):
    // - I dimension: refinement=1, center offset = (1+1)/2 = 1
    // - J dimension: refinement=3, center offset = (3+1)/2 = 2
    // Formula: new_coord = (old_coord - sector_min) * refinement + center_offset
    auto checkWelspecsCoordinates = [&welspecsKeywords]( const std::string& wellName, int expectedI, int expectedJ ) -> bool
    {
        return std::any_of( welspecsKeywords.begin(),
                            welspecsKeywords.end(),
                            [&]( const auto& kw )
                            {
                                for ( size_t i = 0; i < kw.size(); ++i )
                                {
                                    const auto& rec = kw.getRecord( i );
                                    if ( rec.getItem( "WELL" ).template get<std::string>( 0 ) == wellName )
                                    {
                                        return rec.getItem( "HEAD_I" ).template get<int>( 0 ) == expectedI &&
                                               rec.getItem( "HEAD_J" ).template get<int>( 0 ) == expectedJ;
                                    }
                                }
                                return false;
                            } );
    };

    //
    // B-1H: original (11, 3), sector_min (0, 0)
    //   I: (11-0)*1 + 1 = 11, J: (3-0)*3 + 2 = 8
    EXPECT_TRUE( checkWelspecsCoordinates( "B-1H", 11, 8 ) );

    // B-2H: original (4, 7)
    //   I: (4-0)*1 + 1 = 4, J: (7-0)*3 + 2 = 20
    EXPECT_TRUE( checkWelspecsCoordinates( "B-2H", 4, 20 ) );

    // Verify COMPDAT coordinates are centered in refined cells
    auto compdatKeywords = deckFile.findAllKeywords( "COMPDAT" );
    ASSERT_FALSE( compdatKeywords.empty() ) << "COMPDAT keyword not found in exported file";

    std::cout << "\n=== COMPDAT Analysis ===" << std::endl;
    for ( const auto& keyword : compdatKeywords )
    {
        for ( size_t i = 0; i < keyword.size(); ++i )
        {
            const auto& record   = keyword.getRecord( i );
            std::string wellName = record.getItem( "WELL" ).get<std::string>( 0 );
            int         iComp    = record.getItem( "I" ).get<int>( 0 );
            int         jComp    = record.getItem( "J" ).get<int>( 0 );
            int         k1       = record.getItem( "K1" ).get<int>( 0 );
            int         k2       = record.getItem( "K2" ).get<int>( 0 );

            std::cout << "Well: " << wellName << " - COMPDAT I=" << iComp << ", J=" << jComp << ", K1=" << k1 << ", K2=" << k2 << std::endl;
        }
    }

    // Verify specific COMPDAT coordinates are centered correctly
    // For refinement (1, 3, 5):
    // - K dimension: refinement=5, center offset = (5+1)/2 = 3
    auto checkCompdatCoordinates = [&compdatKeywords]( const std::string& wellName, int expectedK1, int expectedK2 ) -> bool
    {
        return std::any_of( compdatKeywords.begin(),
                            compdatKeywords.end(),
                            [&]( const auto& kw )
                            {
                                for ( size_t i = 0; i < kw.size(); ++i )
                                {
                                    const auto& rec = kw.getRecord( i );
                                    if ( rec.getItem( "WELL" ).template get<std::string>( 0 ) == wellName )
                                    {
                                        return rec.getItem( "K1" ).template get<int>( 0 ) == expectedK1 &&
                                               rec.getItem( "K2" ).template get<int>( 0 ) == expectedK2;
                                    }
                                }
                                return false;
                            } );
    };

    // B-1H: original K1=1, K2=5, sector_min K=0
    //   K1: (1-0)*5 + 3 = 3, K2: (5-0)*5 + 3 = 23
    EXPECT_TRUE( checkCompdatCoordinates( "B-1H", 3, 23 ) ) << "B-1H COMPDAT K coordinates not centered correctly";

    // B-2H: original K1=1, K2=6
    //   K1: (1-0)*5 + 3 = 3, K2: (6-0)*5 + 3 = 28
    EXPECT_TRUE( checkCompdatCoordinates( "B-2H", 3, 28 ) ) << "B-2H COMPDAT K coordinates not centered correctly";
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data and refinement (3, 5, 1)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5WithRefinement_3_5_1 )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_refined_3_5_1.DATA";

    // Set up export settings for sector export with refinement (3, 5, 1)
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 19, 14, 9 ) ); // Sector (0-based inclusive)
    settings.setRefinement( cvf::Vec3st( 3, 5, 1 ) ); // Refinement: I=3, J=5, K=1 (no refinement in K)
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    // Get all keywords from the deck
    std::vector<std::string> allKeywords = deckFile.keywords( false );

    // Verify key keywords exist in the file
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "DIMENS" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "SPECGRID" ) != allKeywords.end() )
        << "SPECGRID keyword missing from exported file";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "COORD" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ZCORN" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ACTNUM" ) != allKeywords.end() );

    // Read the file content to verify dimensions
    QFile file( exportFilePath );
    ASSERT_TRUE( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString fileContent = file.readAll();
    file.close();

    // Verify dimensions are correct for the refined sector
    // Original sector: 20x15x10 cells
    // With refinement (3, 5, 1): 20*3 x 15*5 x 10*1 = 60 x 75 x 10
    EXPECT_TRUE( fileContent.contains( " 60 75 10 /" ) ) << "File does not contain expected refined dimensions 60 75 10";

    // Verify WELSPECS coordinates are centered in refined cells
    auto welspecsKeywords = deckFile.findAllKeywords( "WELSPECS" );
    ASSERT_FALSE( welspecsKeywords.empty() ) << "WELSPECS keyword not found in exported file";

    std::cout << "\n=== WELSPECS Analysis (3, 5, 1) ===" << std::endl;
    for ( const auto& keyword : welspecsKeywords )
    {
        for ( size_t i = 0; i < keyword.size(); ++i )
        {
            const auto& record   = keyword.getRecord( i );
            std::string wellName = record.getItem( "WELL" ).get<std::string>( 0 );
            int         iHeel    = record.getItem( "HEAD_I" ).get<int>( 0 );
            int         jHeel    = record.getItem( "HEAD_J" ).get<int>( 0 );

            std::cout << "Well: " << wellName << " - WELSPECS I=" << iHeel << ", J=" << jHeel << std::endl;
        }
    }

    // Verify specific well coordinates are centered correctly
    // For refinement (3, 5, 1):
    // - I dimension: refinement=3, center offset = (3+1)/2 = 2
    // - J dimension: refinement=5, center offset = (5+1)/2 = 3
    // Formula: new_coord = (old_coord - sector_min) * refinement + center_offset
    auto checkWelspecsCoordinates = [&welspecsKeywords]( const std::string& wellName, int expectedI, int expectedJ ) -> bool
    {
        return std::any_of( welspecsKeywords.begin(),
                            welspecsKeywords.end(),
                            [&]( const auto& kw )
                            {
                                for ( size_t i = 0; i < kw.size(); ++i )
                                {
                                    const auto& rec = kw.getRecord( i );
                                    if ( rec.getItem( "WELL" ).template get<std::string>( 0 ) == wellName )
                                    {
                                        return rec.getItem( "HEAD_I" ).template get<int>( 0 ) == expectedI &&
                                               rec.getItem( "HEAD_J" ).template get<int>( 0 ) == expectedJ;
                                    }
                                }
                                return false;
                            } );
    };

    // B-1H: original (11, 3) in 1-based Eclipse, converted to 0-based (10, 2), sector_min (0, 0)
    //   I: (10-0)*3 + 2 = 32, J: (2-0)*5 + 3 = 13
    EXPECT_TRUE( checkWelspecsCoordinates( "B-1H", 32, 13 ) );

    // B-2H: original (4, 7) in 1-based Eclipse, converted to 0-based (3, 6)
    //   I: (3-0)*3 + 2 = 11, J: (6-0)*5 + 3 = 33
    EXPECT_TRUE( checkWelspecsCoordinates( "B-2H", 11, 33 ) );

    // Verify COMPDAT coordinates are centered in refined cells
    auto compdatKeywords = deckFile.findAllKeywords( "COMPDAT" );
    ASSERT_FALSE( compdatKeywords.empty() ) << "COMPDAT keyword not found in exported file";

    std::cout << "\n=== COMPDAT Analysis (3, 5, 1) ===" << std::endl;
    for ( const auto& keyword : compdatKeywords )
    {
        for ( size_t i = 0; i < keyword.size(); ++i )
        {
            const auto& record   = keyword.getRecord( i );
            std::string wellName = record.getItem( "WELL" ).get<std::string>( 0 );
            int         iComp    = record.getItem( "I" ).get<int>( 0 );
            int         jComp    = record.getItem( "J" ).get<int>( 0 );
            int         k1       = record.getItem( "K1" ).get<int>( 0 );
            int         k2       = record.getItem( "K2" ).get<int>( 0 );

            std::cout << "Well: " << wellName << " - COMPDAT I=" << iComp << ", J=" << jComp << ", K1=" << k1 << ", K2=" << k2 << std::endl;
        }
    }

    // Verify specific COMPDAT coordinates are centered correctly
    // For refinement (3, 5, 1):
    // - K dimension: refinement=1, center offset = (1+1)/2 = 1
    auto checkCompdatCoordinates = [&compdatKeywords]( const std::string& wellName, int expectedK1, int expectedK2 ) -> bool
    {
        return std::any_of( compdatKeywords.begin(),
                            compdatKeywords.end(),
                            [&]( const auto& kw )
                            {
                                for ( size_t i = 0; i < kw.size(); ++i )
                                {
                                    const auto& rec = kw.getRecord( i );
                                    if ( rec.getItem( "WELL" ).template get<std::string>( 0 ) == wellName )
                                    {
                                        return rec.getItem( "K1" ).template get<int>( 0 ) == expectedK1 &&
                                               rec.getItem( "K2" ).template get<int>( 0 ) == expectedK2;
                                    }
                                }
                                return false;
                            } );
    };

    // B-1H: original K1=1, K2=5, sector_min K=0
    //   K1: (1-0)*1 + 1 = 1, K2: (5-0)*1 + 1 = 5
    EXPECT_TRUE( checkCompdatCoordinates( "B-1H", 1, 5 ) ) << "B-1H COMPDAT K coordinates not centered correctly";

    // B-2H: original K1=1, K2=6
    //   K1: (1-0)*1 + 1 = 1, K2: (6-0)*1 + 1 = 6
    EXPECT_TRUE( checkCompdatCoordinates( "B-2H", 1, 6 ) ) << "B-2H COMPDAT K coordinates not centered correctly";
}

//--------------------------------------------------------------------------------------------------
/// Helper to load model5 test case
//--------------------------------------------------------------------------------------------------
static cvf::ref<RigEclipseCaseData> loadModel5TestCase()
{
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    if ( !subFolderExists ) return nullptr;

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    if ( !QFile::exists( egridFilePath ) ) return nullptr;

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    if ( !loadResult ) return nullptr;

    return caseData;
}

//--------------------------------------------------------------------------------------------------
/// Test filterInternalSectorConnections - both cells inside
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, FilterNNCConnections_BothInside )
{
    // Load model5 test data (20x30x10 grid)
    auto caseData = loadModel5TestCase();
    ASSERT_TRUE( caseData.notNull() );
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    RigMainGrid* mainGrid = caseData->mainGrid();

    // Create test connections using the real grid
    std::vector<RigSimulationInputTool::NNCConnection> allConnections;

    // Connection 1: (2,3,4) to (3,3,4) - both inside sector [1,5] x [1,5] x [1,5]
    size_t c1Idx = mainGrid->cellIndexFromIJK( 2, 3, 4 );
    size_t c2Idx = mainGrid->cellIndexFromIJK( 3, 3, 4 );
    allConnections.push_back( { c1Idx, c2Idx, 1.5 } );

    // Connection 2: (10,20,8) to (11,20,8) - both outside sector
    c1Idx = mainGrid->cellIndexFromIJK( 10, 20, 8 );
    c2Idx = mainGrid->cellIndexFromIJK( 11, 20, 8 );
    allConnections.push_back( { c1Idx, c2Idx, 2.5 } );

    // Connection 3: (1,2,3) to (15,15,8) - one inside, one outside
    c1Idx = mainGrid->cellIndexFromIJK( 1, 2, 3 );
    c2Idx = mainGrid->cellIndexFromIJK( 15, 15, 8 );
    allConnections.push_back( { c1Idx, c2Idx, 3.5 } );

    // Filter for sector [1,5] x [1,5] x [1,5]
    caf::VecIjk0 sectorMin( 1, 1, 1 );
    caf::VecIjk0 sectorMax( 5, 5, 5 );

    auto filtered = RigSimulationInputTool::filterInternalSectorConnections( allConnections, *mainGrid, sectorMin, sectorMax );

    // Only connection 1 should remain
    ASSERT_EQ( 1u, filtered.size() );
    EXPECT_EQ( 1.5, filtered[0].transmissibility );
}

//--------------------------------------------------------------------------------------------------
/// Test transformNNCToSectorCoordinates - no refinement
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, TransformNNCToSectorCoordinates_NoRefinement )
{
    // Load model5 test data (20x30x10 grid)
    auto caseData = loadModel5TestCase();
    ASSERT_TRUE( caseData.notNull() );
    RigMainGrid* mainGrid = caseData->mainGrid();

    // Connection between (10,15,5) and (11,15,5) - I-face neighbors
    size_t c1Idx = mainGrid->cellIndexFromIJK( 10, 15, 5 );
    size_t c2Idx = mainGrid->cellIndexFromIJK( 11, 15, 5 );

    RigSimulationInputTool::NNCConnection connection{ c1Idx, c2Idx, 4.5 };

    // Sector starts at (5,10,2)
    caf::VecIjk0 sectorMin( 5, 10, 2 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    auto result = RigSimulationInputTool::transformNNCToSectorCoordinates( connection, *mainGrid, sectorMin, refinement );

    ASSERT_TRUE( result.has_value() );

    // Cell 1: (10,15,5) - sector min (5,10,2) = (5,5,3) in 0-based sector coords
    EXPECT_EQ( 5, result->cell1.i() );
    EXPECT_EQ( 5, result->cell1.j() );
    EXPECT_EQ( 3, result->cell1.k() );

    // Cell 2: (11,15,5) - sector min (5,10,2) = (6,5,3) in 0-based sector coords
    EXPECT_EQ( 6, result->cell2.i() );
    EXPECT_EQ( 5, result->cell2.j() );
    EXPECT_EQ( 3, result->cell2.k() );

    EXPECT_EQ( 4.5, result->transmissibility );
}

//--------------------------------------------------------------------------------------------------
/// Test refineEDITNNCConnection - corresponding subcells with refinement (2,2,1)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, RefineEDITNNCConnection_CorrespondingSubcells )
{
    // Load model5 test data (20x30x10 grid)
    auto caseData = loadModel5TestCase();
    ASSERT_TRUE( caseData.notNull() );
    RigMainGrid* mainGrid = caseData->mainGrid();

    // Connection between cells (3,4,5) and (6,8,7) - non-neighbors
    size_t c1Idx = mainGrid->cellIndexFromIJK( 3, 4, 5 );
    size_t c2Idx = mainGrid->cellIndexFromIJK( 6, 8, 7 );

    RigSimulationInputTool::NNCConnection connection{ c1Idx, c2Idx, 2.5 }; // TRAN_MULT = 2.5

    caf::VecIjk0 sectorMin( 0, 0, 0 );
    cvf::Vec3st  refinement( 2, 2, 1 ); // Refine by 2x2x1

    auto refined = RigSimulationInputTool::refineEditNncConnection( connection, *mainGrid, sectorMin, refinement );

    // For EDITNNC: creates connections between corresponding subcells
    // Should create 2*2*1 = 4 connections (one per subcell pair at same relative position)
    ASSERT_EQ( 4u, refined.size() );

    // All connections should have same TRAN_MULT (no distribution)
    for ( const auto& conn : refined )
    {
        EXPECT_DOUBLE_EQ( 2.5, conn.transmissibility );
    }

    // Verify corresponding subcell connections
    // Cell1 starts at sector coordinates (3*2, 4*2, 5*1) = (6, 8, 5)
    // Cell2 starts at sector coordinates (6*2, 8*2, 7*1) = (12, 16, 7)

    // Connection 0: (0,0,0) -> (0,0,0): (6,8,5) -> (12,16,7)
    EXPECT_EQ( caf::VecIjk0( 6, 8, 5 ), refined[0].cell1 );
    EXPECT_EQ( caf::VecIjk0( 12, 16, 7 ), refined[0].cell2 );

    // Connection 1: (0,1,0) -> (0,1,0): (6,9,5) -> (12,17,7)
    EXPECT_EQ( caf::VecIjk0( 6, 9, 5 ), refined[1].cell1 );
    EXPECT_EQ( caf::VecIjk0( 12, 17, 7 ), refined[1].cell2 );

    // Connection 2: (1,0,0) -> (1,0,0): (7,8,5) -> (13,16,7)
    EXPECT_EQ( caf::VecIjk0( 7, 8, 5 ), refined[2].cell1 );
    EXPECT_EQ( caf::VecIjk0( 13, 16, 7 ), refined[2].cell2 );

    // Connection 3: (1,1,0) -> (1,1,0): (7,9,5) -> (13,17,7)
    EXPECT_EQ( caf::VecIjk0( 7, 9, 5 ), refined[3].cell1 );
    EXPECT_EQ( caf::VecIjk0( 13, 17, 7 ), refined[3].cell2 );
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data and model padding
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5WithPadding )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_padded.DATA";

    // Set up export settings for sector export with padding
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 19, 14, 9 ) ); // Sector (0-based inclusive) -> 20x15x10
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) ); // No refinement
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Configure padding: 2 upper, 3 lower
    RigModelPaddingSettings paddingSettings;
    paddingSettings.setEnabled( true );
    paddingSettings.setNzUpper( 2 );
    paddingSettings.setNzLower( 3 );
    paddingSettings.setTopUpper( 1000.0 );
    paddingSettings.setBottomLower( 3600.0 );
    paddingSettings.setUpperPorosity( 0.1 );
    paddingSettings.setMinLayerThickness( 10.0 );
    paddingSettings.setVerticalPillars( true );
    paddingSettings.setMonotonicZcorn( true );
    paddingSettings.setFillGaps( true );
    settings.setPaddingSettings( paddingSettings );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    // Get all keywords from the deck
    std::vector<std::string> allKeywords = deckFile.keywords( false );

    // Verify key keywords exist in the file
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "DIMENS" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "SPECGRID" ) != allKeywords.end() )
        << "SPECGRID keyword missing from exported file";
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "COORD" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ZCORN" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "ACTNUM" ) != allKeywords.end() );
    EXPECT_TRUE( std::find( allKeywords.begin(), allKeywords.end(), "PORO" ) != allKeywords.end() );

    // Read the file content to verify padded dimensions
    QFile file( exportFilePath );
    ASSERT_TRUE( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString fileContent = file.readAll();
    file.close();

    // Verify dimensions are correct for the padded sector (20x15x15)
    // Original sector: 20x15x10, with nzUpper=2, nzLower=3: 20x15x(10+2+3) = 20x15x15
    EXPECT_TRUE( fileContent.contains( " 20 15 15 /" ) ) << "File does not contain expected padded dimensions 20 15 15";

    // Verify SPECGRID/DIMENS show correct padded dimensions
    auto specgridKw = deckFile.findKeyword( "SPECGRID" );
    ASSERT_TRUE( specgridKw.has_value() );
    EXPECT_EQ( 20, specgridKw->getRecord( 0 ).getItem( 0 ).get<int>( 0 ) );
    EXPECT_EQ( 15, specgridKw->getRecord( 0 ).getItem( 1 ).get<int>( 0 ) );
    EXPECT_EQ( 15, specgridKw->getRecord( 0 ).getItem( 2 ).get<int>( 0 ) );

    // Verify COORD array size: (NX+1)*(NY+1)*6 = 21*16*6 = 2016 (unchanged by padding)
    auto coordKw = deckFile.findKeyword( "COORD" );
    ASSERT_TRUE( coordKw.has_value() );
    EXPECT_EQ( 2016u, coordKw->getRecord( 0 ).getItem( 0 ).data_size() );

    // Verify ZCORN array size: NX*NY*NZ_new*8 = 20*15*15*8 = 36000
    auto zcornKw = deckFile.findKeyword( "ZCORN" );
    ASSERT_TRUE( zcornKw.has_value() );
    EXPECT_EQ( 36000u, zcornKw->getRecord( 0 ).getItem( 0 ).data_size() );

    // Verify ACTNUM array size: NX*NY*NZ_new = 20*15*15 = 4500
    auto actnumKw = deckFile.findKeyword( "ACTNUM" );
    ASSERT_TRUE( actnumKw.has_value() );
    EXPECT_EQ( 4500u, actnumKw->getRecord( 0 ).getItem( 0 ).data_size() );
}

//--------------------------------------------------------------------------------------------------
/// Helper to create an ADD DeckRecord with box indices
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createAddRecord( const std::string& fieldName, double shift, int i1, int i2, int j1, int j2, int k1, int k2 )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", fieldName ) );
    items.push_back( RifOpmDeckTools::item( "SHIFT", shift ) );
    items.push_back( RifOpmDeckTools::item( "I1", i1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", i2 ) );
    items.push_back( RifOpmDeckTools::item( "J1", j1 ) );
    items.push_back( RifOpmDeckTools::item( "J2", j2 ) );
    items.push_back( RifOpmDeckTools::item( "K1", k1 ) );
    items.push_back( RifOpmDeckTools::item( "K2", k2 ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Helper to create an ADD DeckRecord without box indices
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createAddRecordNoBox( const std::string& fieldName, double shift )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", fieldName ) );
    items.push_back( RifOpmDeckTools::item( "SHIFT", shift ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Helper to create a MULTIPLY DeckRecord with box indices
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createMultiplyRecord( const std::string& fieldName, double factor, int i1, int i2, int j1, int j2, int k1, int k2 )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", fieldName ) );
    items.push_back( RifOpmDeckTools::item( "FACTOR", factor ) );
    items.push_back( RifOpmDeckTools::item( "I1", i1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", i2 ) );
    items.push_back( RifOpmDeckTools::item( "J1", j1 ) );
    items.push_back( RifOpmDeckTools::item( "J2", j2 ) );
    items.push_back( RifOpmDeckTools::item( "K1", k1 ) );
    items.push_back( RifOpmDeckTools::item( "K2", k2 ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Helper to create a MULTIPLY DeckRecord without box indices
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createMultiplyRecordNoBox( const std::string& fieldName, double factor )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", fieldName ) );
    items.push_back( RifOpmDeckTools::item( "FACTOR", factor ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Test ADD record without explicit box indices - should pass through with just field+shift
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAddRecord_NoBoxIndices )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    auto record = createAddRecordNoBox( "PORO", 0.1 );

    auto result = RigSimulationInputTool::processAddRecord( record, sectorMin, sectorMax, refinement );
    ASSERT_TRUE( result.has_value() );

    // Should have only 2 items (field name and shift value, no box indices)
    EXPECT_EQ( 2u, result->size() );
    EXPECT_EQ( "PORO", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( 0.1, result->getItem( 1 ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test ADD record with explicit box indices - should transform coordinates
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAddRecord_WithBoxIndices )
{
    // Sector: sectorMin=(0, 15, 0), sectorMax=(19, 29, 9)
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // ADD record with box fully inside sector (1-based Eclipse coords)
    auto record = createAddRecord( "PORO", 0.05, 1, 20, 16, 30, 1, 10 );

    auto result = RigSimulationInputTool::processAddRecord( record, sectorMin, sectorMax, refinement );
    ASSERT_TRUE( result.has_value() );

    // Should have 8 items with transformed coordinates
    EXPECT_EQ( 8u, result->size() );
    EXPECT_EQ( "PORO", result->getItem( 0 ).get<std::string>( 0 ) );

    // Transformed: I stays same (1-20), J transforms from [16-30] to [1-15], K stays (1-10)
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test MULTIPLY record without explicit box indices - should pass through with just field+factor
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessMultiplyRecord_NoBoxIndices )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    auto record = createMultiplyRecordNoBox( "PERMX", 2.0 );

    auto result = RigSimulationInputTool::processMultiplyRecord( record, sectorMin, sectorMax, refinement );
    ASSERT_TRUE( result.has_value() );

    // Should have only 2 items (field name and factor, no box indices)
    EXPECT_EQ( 2u, result->size() );
    EXPECT_EQ( "PERMX", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( 2.0, result->getItem( 1 ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test MULTIPLY record with explicit box indices - should transform coordinates
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessMultiplyRecord_WithBoxIndices )
{
    // Sector: sectorMin=(0, 15, 0), sectorMax=(19, 29, 9)
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // MULTIPLY record with box fully inside sector
    auto record = createMultiplyRecord( "PERMX", 3.0, 1, 20, 16, 30, 1, 10 );

    auto result = RigSimulationInputTool::processMultiplyRecord( record, sectorMin, sectorMax, refinement );
    ASSERT_TRUE( result.has_value() );

    // Should have 8 items with transformed coordinates
    EXPECT_EQ( 8u, result->size() );
    EXPECT_EQ( "PERMX", result->getItem( 0 ).get<std::string>( 0 ) );

    // Transformed coordinates
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test transformKeywordsInDeckFile: EQUALS inside BOX/ENDBOX with no explicit indices
/// gets BOX indices injected and then transformed to sector-relative coordinates.
/// Using sector covering the whole grid so transformed coords = original coords.
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExpandBoxContext_EqualsInheritsBoxIndices )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_box.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "BOX\n 2 8 3 7 1 5 /\n\n" );
        file.write( "EQUALS\n 'PORO' 0.25 /\n/\n\n" );
        file.write( "ENDBOX\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    // Verify EQUALS has no explicit box indices before transformation
    auto equalsBeforeVec = deckFile.findAllKeywordsWithIndices( "EQUALS" );
    ASSERT_EQ( 1u, equalsBeforeVec.size() );
    ASSERT_EQ( 1u, equalsBeforeVec[0].second.size() );
    const auto& recBefore         = equalsBeforeVec[0].second.getRecord( 0 );
    bool        hadExplicitBefore = recBefore.size() >= 8 && recBefore.getItem( 2 ).hasValue( 0 ) && recBefore.getItem( 3 ).hasValue( 0 ) &&
                             recBefore.getItem( 4 ).hasValue( 0 ) && recBefore.getItem( 5 ).hasValue( 0 ) &&
                             recBefore.getItem( 6 ).hasValue( 0 ) && recBefore.getItem( 7 ).hasValue( 0 );
    EXPECT_FALSE( hadExplicitBefore ) << "Record should not have explicit box indices before transformation";

    // Sector covers entire grid: sectorMin=(0,0,0), sectorMax=(9,9,4), refinement=(1,1,1)
    // This makes coordinate transformation an identity operation
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 9, 9, 4 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    // Verify EQUALS now has explicit box indices matching the BOX keyword
    auto equalsAfterVec = deckFile.findAllKeywordsWithIndices( "EQUALS" );
    ASSERT_EQ( 1u, equalsAfterVec.size() );
    ASSERT_EQ( 1u, equalsAfterVec[0].second.size() );

    const auto& recAfter = equalsAfterVec[0].second.getRecord( 0 );
    ASSERT_GE( recAfter.size(), 8u );
    EXPECT_TRUE( recAfter.getItem( 2 ).hasValue( 0 ) );
    EXPECT_EQ( 2, recAfter.getItem( 2 ).get<int>( 0 ) ); // I1 from BOX
    EXPECT_EQ( 8, recAfter.getItem( 3 ).get<int>( 0 ) ); // I2 from BOX
    EXPECT_EQ( 3, recAfter.getItem( 4 ).get<int>( 0 ) ); // J1 from BOX
    EXPECT_EQ( 7, recAfter.getItem( 5 ).get<int>( 0 ) ); // J2 from BOX
    EXPECT_EQ( 1, recAfter.getItem( 6 ).get<int>( 0 ) ); // K1 from BOX
    EXPECT_EQ( 5, recAfter.getItem( 7 ).get<int>( 0 ) ); // K2 from BOX
}

//--------------------------------------------------------------------------------------------------
/// Test transformKeywordsInDeckFile: records with explicit indices keep their own indices
/// (not overridden by BOX indices) and are transformed to sector-relative coordinates.
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExpandBoxContext_ExplicitIndicesPreserved )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_box_explicit.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "BOX\n 2 8 3 7 1 5 /\n\n" );
        file.write( "EQUALS\n 'PORO' 0.25 1 5 1 5 1 3 /\n/\n\n" );
        file.write( "ENDBOX\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    // Sector covers entire grid
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 9, 9, 4 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    auto equalsVec = deckFile.findAllKeywordsWithIndices( "EQUALS" );
    ASSERT_EQ( 1u, equalsVec.size() );
    ASSERT_EQ( 1u, equalsVec[0].second.size() );

    const auto& rec = equalsVec[0].second.getRecord( 0 );
    ASSERT_GE( rec.size(), 8u );
    // Should keep original explicit indices, not BOX indices
    EXPECT_EQ( 1, rec.getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 5, rec.getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, rec.getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 5, rec.getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, rec.getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 3, rec.getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test transformKeywordsInDeckFile: EQUALS outside BOX context without explicit indices
/// passes through unchanged (no box indices injected).
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExpandBoxContext_OutsideBoxNotModified )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_no_box.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "EQUALS\n 'PORO' 0.25 /\n/\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    // Sector covers entire grid
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 9, 9, 4 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    auto equalsVec = deckFile.findAllKeywordsWithIndices( "EQUALS" );
    ASSERT_EQ( 1u, equalsVec.size() );
    ASSERT_EQ( 1u, equalsVec[0].second.size() );

    const auto& rec = equalsVec[0].second.getRecord( 0 );
    // Should still not have explicit box indices
    bool hasExplicit = rec.size() >= 8 && rec.getItem( 2 ).hasValue( 0 ) && rec.getItem( 3 ).hasValue( 0 ) && rec.getItem( 4 ).hasValue( 0 ) &&
                       rec.getItem( 5 ).hasValue( 0 ) && rec.getItem( 6 ).hasValue( 0 ) && rec.getItem( 7 ).hasValue( 0 );
    EXPECT_FALSE( hasExplicit );
}

//--------------------------------------------------------------------------------------------------
/// Test transformKeywordsInDeckFile: data keywords inside BOX/ENDBOX are cropped
/// to the intersection of the box with the sector
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, CropDataKeywordsInsideBoxContext )
{
    // Grid: 10x10x5. BOX covers K=5 (1-based), i.e. K-index 4 (0-based).
    // Sector: (2,2,1)-(7,7,4) => covers part of the box region.
    // BOX: I[0-9], J[0-9], K[4-4] => 100 values
    // Intersection: I[2-7], J[2-7], K[4-4] => 6x6x1 = 36 values
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_box_crop.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "REGIONS\n\n" );
        file.write( "EQLNUM\n 500*1 /\n\n" );
        file.write( "BOX\n 1 10 1 10 5 5 /\n\n" );
        file.write( "EQLNUM\n 100*2 /\n\n" );
        file.write( "ENDBOX\n\n" );
        file.write( "SATNUM\n 500*1 /\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    // Before cropping: should find two EQLNUM keywords
    auto eqlnumBefore = deckFile.findAllKeywordsWithIndices( "EQLNUM" );
    EXPECT_EQ( 2u, eqlnumBefore.size() );

    // Run transformation with sector (2,2,1)-(7,7,4), no refinement
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 2, 2, 1 ) );
    settings.setMax( caf::VecIjk0( 7, 7, 4 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    // After cropping: should still find two EQLNUM keywords
    auto eqlnumAfter = deckFile.findAllKeywordsWithIndices( "EQLNUM" );
    ASSERT_EQ( 2u, eqlnumAfter.size() );

    // First EQLNUM (full grid) should be unchanged (500 values)
    EXPECT_EQ( 500u, eqlnumAfter[0].second.getRecord( 0 ).getItem( 0 ).data_size() );

    // Second EQLNUM (inside BOX) should be cropped to 6x6x1 = 36 values
    const auto& croppedKw = eqlnumAfter[1].second;
    EXPECT_EQ( 36u, croppedKw.getRecord( 0 ).getItem( 0 ).data_size() );

    // All cropped values should still be 2
    const auto& croppedData = croppedKw.getIntData();
    for ( size_t i = 0; i < croppedData.size(); ++i )
    {
        EXPECT_EQ( 2, croppedData[i] ) << "Cropped EQLNUM value at index " << i << " should be 2";
    }

    // SATNUM outside BOX should still be present and unchanged
    auto satnumAfter = deckFile.findAllKeywordsWithIndices( "SATNUM" );
    EXPECT_EQ( 1u, satnumAfter.size() );
    EXPECT_EQ( 500u, satnumAfter[0].second.getRecord( 0 ).getItem( 0 ).data_size() );
}

//--------------------------------------------------------------------------------------------------
/// Test transformKeywordsInDeckFile: box that doesn't intersect sector removes data keyword
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, CropDataKeywordsInsideBoxContext_NoIntersection )
{
    // Grid: 10x10x5. BOX covers K=5 (1-based), i.e. K-index 4 (0-based).
    // Sector: (2,2,0)-(7,7,2) => does NOT include K=4, so no intersection.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_box_nointersect.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "REGIONS\n\n" );
        file.write( "EQLNUM\n 500*1 /\n\n" );
        file.write( "BOX\n 1 10 1 10 5 5 /\n\n" );
        file.write( "EQLNUM\n 100*2 /\n\n" );
        file.write( "ENDBOX\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    // Sector does not include K=4
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 2, 2, 0 ) );
    settings.setMax( caf::VecIjk0( 7, 7, 2 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    // Boxed EQLNUM should be removed since box doesn't intersect sector
    auto eqlnumAfter = deckFile.findAllKeywordsWithIndices( "EQLNUM" );
    ASSERT_EQ( 1u, eqlnumAfter.size() );
    EXPECT_EQ( 500u, eqlnumAfter[0].second.getRecord( 0 ).getItem( 0 ).data_size() );
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 BOX data: EQLNUM inside BOX/ENDBOX
/// Verifies that the boxed EQLNUM is cropped to the sector/box intersection and the
/// BOX/ENDBOX structure is preserved in the exported deck.
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5_DataKeywordInsideBox )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "2_BOX_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_box_model.DATA";

    // Set up export settings: sector includes K=10 to verify BOX override
    // Sector: min=(5,5,2), max=(14,14,9) => 10x10x8 = 800 cells (includes K=10 which is K-index 9)
    // BOX in input is: I[1-20], J[1-30], K[10-10] (1-based) = I[0-19], J[0-29], K[9-9] (0-based)
    // Intersection with sector: I[5-14], J[5-14], K[9-9] => 10x10x1 = 100 cells
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 5, 5, 2 ) );
    settings.setMax( caf::VecIjk0( 14, 14, 9 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    const size_t sectorNx          = 10;
    const size_t sectorNy          = 10;
    const size_t sectorNz          = 8;
    const size_t expectedCellCount = sectorNx * sectorNy * sectorNz; // 800 cells

    // Both EQLNUM keywords should exist: the full-grid one (cropped to sector) and the boxed one (cropped to intersection)
    auto eqlnumAll = deckFile.findAllKeywordsWithIndices( "EQLNUM" );
    EXPECT_EQ( 2u, eqlnumAll.size() ) << "Should have two EQLNUM keywords (full-grid + cropped boxed)";

    // First EQLNUM (full grid, cropped to sector): 10x10x8 = 800 values
    {
        const auto& firstKw   = eqlnumAll[0].second;
        const auto& firstData = firstKw.getIntData();
        EXPECT_EQ( expectedCellCount, firstData.size() ) << "First EQLNUM should have " << expectedCellCount << " values";
    }

    // Second EQLNUM (boxed, cropped to sector/box intersection): 10x10x1 = 100 values
    {
        const size_t expectedBoxedCount = sectorNx * sectorNy * 1; // 100 cells
        const auto&  secondKw           = eqlnumAll[1].second;
        const auto&  secondData         = secondKw.getIntData();
        EXPECT_EQ( expectedBoxedCount, secondData.size() ) << "Boxed EQLNUM should have " << expectedBoxedCount << " values";

        // All boxed values should be 2 (the BOX override value)
        for ( size_t i = 0; i < secondData.size(); ++i )
        {
            EXPECT_EQ( 2, secondData[i] ) << "Boxed EQLNUM value at index " << i << " should be 2";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test MULTIPLY record with no overlap - should return error
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessMultiplyRecord_NoOverlap )
{
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // MULTIPLY record: PERMX 2.0 1 20 1 14 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[0-13], K[0-9]
    // Sector J is [15-29], so no overlap in J dimension
    auto record = createMultiplyRecord( "PERMX", 2.0, 1, 20, 1, 14, 1, 10 );

    auto result = RigSimulationInputTool::processMultiplyRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test MULTIPLY record with partial overlap requiring clamping
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessMultiplyRecord_PartialOverlapWithClamping )
{
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // MULTIPLY record: PERMX 3.5 1 20 15 30 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[14-29], K[0-9]
    // Clamped to sector: I[0-19], J[15-29], K[0-9]
    auto record = createMultiplyRecord( "PERMX", 3.5, 1, 20, 15, 30, 1, 10 );

    auto result = RigSimulationInputTool::processMultiplyRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Transformed to sector-relative (1-based):
    //   I: 1 to 20, J: 1 to 15, K: 1 to 10
    EXPECT_EQ( "PERMX", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( 3.5, result->getItem( 1 ).get<double>( 0 ) );
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test MULTIPLY record completely inside sector (no clamping)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessMultiplyRecord_CompletelyInsideSector )
{
    caf::VecIjk0 sectorMin( 2, 2, 1 );
    caf::VecIjk0 sectorMax( 7, 7, 4 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // MULTIPLY record: PORV 0.5 4 6 4 6 2 4 (1-based Eclipse)
    // Converts to 0-based: I[3-5], J[3-5], K[1-3] — inside sector
    auto record = createMultiplyRecord( "PORV", 0.5, 4, 6, 4, 6, 2, 4 );

    auto result = RigSimulationInputTool::processMultiplyRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Transformed to sector-relative (1-based):
    //   I: (3-2)+1=2 to (5-2)+1=4, J: (3-2)+1=2 to (5-2)+1=4, K: (1-1)+1=1 to (3-1)+1=3
    EXPECT_EQ( "PORV", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( 0.5, result->getItem( 1 ).get<double>( 0 ) );
    EXPECT_EQ( 2, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 4, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 2, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 4, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 3, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test ADD record with no overlap - should return error
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAddRecord_NoOverlap )
{
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // ADD record: PRESSURE 100.0 1 20 1 14 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[0-13], K[0-9]
    // Sector J is [15-29], so no overlap
    auto record = createAddRecord( "PRESSURE", 100.0, 1, 20, 1, 14, 1, 10 );

    auto result = RigSimulationInputTool::processAddRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test ADD record with partial overlap requiring clamping
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAddRecord_PartialOverlapWithClamping )
{
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // ADD record: PRESSURE -50.0 1 20 15 30 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[14-29], K[0-9]
    // Clamped to sector: I[0-19], J[15-29], K[0-9]
    auto record = createAddRecord( "PRESSURE", -50.0, 1, 20, 15, 30, 1, 10 );

    auto result = RigSimulationInputTool::processAddRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Transformed to sector-relative (1-based):
    //   I: 1 to 20, J: 1 to 15, K: 1 to 10
    EXPECT_EQ( "PRESSURE", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( -50.0, result->getItem( 1 ).get<double>( 0 ) );
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test ADD record completely inside sector (no clamping)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessAddRecord_CompletelyInsideSector )
{
    caf::VecIjk0 sectorMin( 2, 2, 1 );
    caf::VecIjk0 sectorMax( 7, 7, 4 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // ADD record: PORO 0.01 4 6 4 6 2 4 (1-based Eclipse)
    // Converts to 0-based: I[3-5], J[3-5], K[1-3] — inside sector
    auto record = createAddRecord( "PORO", 0.01, 4, 6, 4, 6, 2, 4 );

    auto result = RigSimulationInputTool::processAddRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Transformed to sector-relative (1-based):
    //   I: (3-2)+1=2 to (5-2)+1=4, J: (3-2)+1=2 to (5-2)+1=4, K: (1-1)+1=1 to (3-1)+1=3
    EXPECT_EQ( "PORO", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( 0.01, result->getItem( 1 ).get<double>( 0 ) );
    EXPECT_EQ( 2, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 4, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 2, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 4, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 3, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test MULTIPLY inside BOX/ENDBOX context: coordinates are transformed correctly
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, MultiplyInsideBoxContext_NoIntersection )
{
    // Grid: 10x10x5. BOX covers I[1-10], J[1-10], K[5-5] (1-based = K=4 0-based).
    // Sector: (2,2,0)-(7,7,2) => K[0-2], does NOT include K=4.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_multiply_box.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "PROPS\n\n" );
        file.write( "BOX\n 1 10 1 10 5 5 /\n\n" );
        file.write( "MULTIPLY\n PERMX 2.0 /\n/\n\n" );
        file.write( "ENDBOX\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    // Sector (2,2,0)-(7,7,2) does NOT include K=4 (0-based)
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 2, 2, 0 ) );
    settings.setMax( caf::VecIjk0( 7, 7, 2 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    // MULTIPLY record with box K=5 (0-based K=4) outside sector K[0-2] should be removed
    auto multiplyAll = deckFile.findAllKeywordsWithIndices( "MULTIPLY" );
    if ( !multiplyAll.empty() )
    {
        EXPECT_EQ( 0u, multiplyAll[0].second.size() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Test MULTIPLY inside BOX/ENDBOX with intersection: coordinates transformed correctly
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, MultiplyInsideBoxContext_WithIntersection )
{
    // Grid: 10x10x5. BOX covers I[3-8], J[3-8], K[2-4] (1-based).
    // Sector: (2,2,1)-(7,7,4) (0-based) => I[2-7], J[2-7], K[1-4]
    // BOX 0-based: I[2-7], J[2-7], K[1-3]
    // Intersection: I[2-7], J[2-7], K[1-3] => fully inside sector
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_multiply_box_intersect.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "PROPS\n\n" );
        file.write( "BOX\n 3 8 3 8 2 4 /\n\n" );
        file.write( "MULTIPLY\n PERMX 2.5 /\n/\n\n" );
        file.write( "ENDBOX\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 2, 2, 1 ) );
    settings.setMax( caf::VecIjk0( 7, 7, 4 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    auto multiplyAll = deckFile.findAllKeywordsWithIndices( "MULTIPLY" );
    ASSERT_EQ( 1u, multiplyAll.size() );

    const auto& kw = multiplyAll[0].second;
    ASSERT_GE( kw.size(), 1u );

    const auto& rec = kw.getRecord( 0 );
    EXPECT_EQ( "PERMX", rec.getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( 2.5, rec.getItem( 1 ).get<double>( 0 ) );

    // BOX 0-based I[2-7], J[2-7], K[1-3], sector min (2,2,1)
    // Sector-relative (1-based): I: 1-6, J: 1-6, K: 1-3
    EXPECT_EQ( 1, rec.getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 6, rec.getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, rec.getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 6, rec.getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, rec.getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 3, rec.getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test ADD inside BOX/ENDBOX with intersection: coordinates transformed correctly
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, AddInsideBoxContext_WithIntersection )
{
    // Grid: 10x10x5. BOX covers I[3-8], J[3-8], K[2-4] (1-based).
    // Sector: (2,2,1)-(7,7,4) (0-based)
    // BOX 0-based: I[2-7], J[2-7], K[1-3] — fully inside sector
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_add_box_intersect.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "PROPS\n\n" );
        file.write( "BOX\n 3 8 3 8 2 4 /\n\n" );
        file.write( "ADD\n PRESSURE 100.0 /\n/\n\n" );
        file.write( "ENDBOX\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 2, 2, 1 ) );
    settings.setMax( caf::VecIjk0( 7, 7, 4 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    auto addAll = deckFile.findAllKeywordsWithIndices( "ADD" );
    ASSERT_EQ( 1u, addAll.size() );

    const auto& kw = addAll[0].second;
    ASSERT_GE( kw.size(), 1u );

    const auto& rec = kw.getRecord( 0 );
    EXPECT_EQ( "PRESSURE", rec.getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_DOUBLE_EQ( 100.0, rec.getItem( 1 ).get<double>( 0 ) );

    // Sector-relative (1-based): I: 1-6, J: 1-6, K: 1-3
    EXPECT_EQ( 1, rec.getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 6, rec.getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, rec.getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 6, rec.getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, rec.getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 3, rec.getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test ADD inside BOX/ENDBOX with no intersection: record should be removed
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, AddInsideBoxContext_NoIntersection )
{
    // Grid: 10x10x5. BOX covers K=5 (1-based), i.e. K-index 4 (0-based).
    // Sector: (2,2,1)-(7,7,4) does NOT include K=4 (0-based is K[1-4], and K=4 is included)
    // Actually sector max K=4 means K[1-4] inclusive (0-based), so K=4 IS included.
    // Use sector (2,2,0)-(7,7,2) to NOT include K=4.
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString dataFilePath = tempDir.path() + "/test_add_box_nointersect.DATA";
    {
        QFile file( dataFilePath );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        file.write( "RUNSPEC\n\n" );
        file.write( "DIMENS\n 10 10 5 /\n\n" );
        file.write( "GRID\n\n" );
        file.write( "PROPS\n\n" );
        file.write( "BOX\n 1 10 1 10 5 5 /\n\n" );
        file.write( "ADD\n PRESSURE 100.0 /\n/\n\n" );
        file.write( "ENDBOX\n\n" );
        file.close();
    }

    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( dataFilePath.toStdString() );
    ASSERT_TRUE( loadResult.has_value() ) << loadResult.error();

    // Sector (2,2,0)-(7,7,2) does NOT include K=4 (0-based)
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 2, 2, 0 ) );
    settings.setMax( caf::VecIjk0( 7, 7, 2 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );

    RigSimulationInputTool::transformKeywordsInDeckFile( nullptr, settings, deckFile );

    // ADD record with box K=5 (0-based K=4) outside sector K[0-2] should be removed
    auto addAll = deckFile.findAllKeywordsWithIndices( "ADD" );
    if ( !addAll.empty() )
    {
        EXPECT_EQ( 0u, addAll[0].second.size() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data verifying FIP keyword cropping
/// Ensures custom FIPABC keyword in REGIONS section is cropped to sector cell count
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5_FipKeywordCropping )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_fipkw.DATA";

    // Set up export settings for a smaller sector to test cropping
    // Sector: min=(5,5,2), max=(14,14,7) => 10x10x6 = 600 cells
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 5, 5, 2 ) );
    settings.setMax( caf::VecIjk0( 14, 14, 7 ) );
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) );
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    const size_t expectedCellCount = 10 * 10 * 6; // 600 cells

    // Verify FIPABC keyword is cropped to sector size with correct values
    // Original model5 has FIPABC = 2000*1 2000*2 2000*3 over a 20x30x10 grid (600 cells/layer)
    // Sector (5,5,2)-(14,14,7) extracts 10x10 cells from layers k=2..7
    {
        auto fipabcKw = deckFile.findKeyword( "FIPABC" );
        ASSERT_TRUE( fipabcKw.has_value() ) << "FIPABC keyword missing from exported file";

        const auto& fipabcData = fipabcKw->getIntData();
        EXPECT_EQ( expectedCellCount, fipabcData.size() ) << "FIPABC should have " << expectedCellCount << " values";

        if ( fipabcData.size() == expectedCellCount )
        {
            // Verify expected values for each layer in the sector:
            // k_s=0 (orig k=2): all value 1 (flat indices in [1200,1800), all in first 2000)
            // k_s=1 (orig k=3): j_s=0..4 (orig j=5..9) -> value 1, j_s=5..9 (orig j=10..14) -> value 2
            // k_s=2..4 (orig k=4..6): all value 2
            // k_s=5 (orig k=7): all value 3

            const size_t NI = 10;
            const size_t NJ = 10;

            // k_s=0: all value 1
            for ( size_t j = 0; j < NJ; ++j )
                for ( size_t i = 0; i < NI; ++i )
                    EXPECT_EQ( 1, fipabcData[i + j * NI + 0 * NI * NJ] ) << "FIPABC at k_s=0, j=" << j << ", i=" << i << " should be 1";

            // k_s=1: j_s=0..4 -> value 1, j_s=5..9 -> value 2
            for ( size_t j = 0; j < 5; ++j )
                for ( size_t i = 0; i < NI; ++i )
                    EXPECT_EQ( 1, fipabcData[i + j * NI + 1 * NI * NJ] ) << "FIPABC at k_s=1, j=" << j << ", i=" << i << " should be 1";
            for ( size_t j = 5; j < NJ; ++j )
                for ( size_t i = 0; i < NI; ++i )
                    EXPECT_EQ( 2, fipabcData[i + j * NI + 1 * NI * NJ] ) << "FIPABC at k_s=1, j=" << j << ", i=" << i << " should be 2";

            // k_s=2..4: all value 2
            for ( size_t k = 2; k <= 4; ++k )
                for ( size_t j = 0; j < NJ; ++j )
                    for ( size_t i = 0; i < NI; ++i )
                        EXPECT_EQ( 2, fipabcData[i + j * NI + k * NI * NJ] )
                            << "FIPABC at k_s=" << k << ", j=" << j << ", i=" << i << " should be 2";

            // k_s=5: all value 3
            for ( size_t j = 0; j < NJ; ++j )
                for ( size_t i = 0; i < NI; ++i )
                    EXPECT_EQ( 3, fipabcData[i + j * NI + 5 * NI * NJ] ) << "FIPABC at k_s=5, j=" << j << ", i=" << i << " should be 3";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test exportSimulationInput with model5 data and padding for FIP keyword
/// Ensures custom FIPABC keyword is padded with minimum value in pad layers
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ExportModel5WithPadding_FipKeyword )
{
    // Load model5 test data
    QDir baseFolder( TEST_DATA_DIR );
    bool subFolderExists = baseFolder.cd( "RigSimulationInputTool/model5" );
    ASSERT_TRUE( subFolderExists );

    QString egridFilename( "0_BASE_MODEL5.EGRID" );
    QString egridFilePath = baseFolder.absoluteFilePath( egridFilename );
    ASSERT_TRUE( QFile::exists( egridFilePath ) );

    QString dataFilename( "0_BASE_MODEL5.DATA" );
    QString dataFilePath = baseFolder.absoluteFilePath( dataFilename );
    ASSERT_TRUE( QFile::exists( dataFilePath ) );

    // Create Eclipse case and load grid
    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    cvf::ref<RifReaderEclipseOutput> reader     = new RifReaderEclipseOutput;
    bool                             loadResult = reader->open( egridFilePath, caseData.p() );
    ASSERT_TRUE( loadResult );

    // Verify grid dimensions (20x30x10)
    ASSERT_TRUE( caseData->mainGrid() != nullptr );
    EXPECT_EQ( 20u, caseData->mainGrid()->cellCountI() );
    EXPECT_EQ( 30u, caseData->mainGrid()->cellCountJ() );
    EXPECT_EQ( 10u, caseData->mainGrid()->cellCountK() );

    // Create temporary directory for export
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString exportFilePath = tempDir.path() + "/exported_model_padded_fipkw.DATA";

    // Set up export settings for sector export with padding
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 19, 14, 9 ) ); // Sector (0-based inclusive) -> 20x15x10
    settings.setRefinement( cvf::Vec3st( 1, 1, 1 ) ); // No refinement
    settings.setInputDeckFileName( dataFilePath );
    settings.setOutputDeckFileName( exportFilePath );

    // Configure padding: 2 upper, 3 lower
    RigModelPaddingSettings paddingSettings;
    paddingSettings.setEnabled( true );
    paddingSettings.setNzUpper( 2 );
    paddingSettings.setNzLower( 3 );
    paddingSettings.setTopUpper( 1000.0 );
    paddingSettings.setBottomLower( 3600.0 );
    paddingSettings.setUpperPorosity( 0.1 );
    paddingSettings.setMinLayerThickness( 10.0 );
    paddingSettings.setVerticalPillars( true );
    paddingSettings.setMonotonicZcorn( true );
    paddingSettings.setFillGaps( true );
    settings.setPaddingSettings( paddingSettings );

    // Create visibility from IJK bounds
    cvf::ref<cvf::UByteArray> visibility =
        RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData.p(), settings.min(), settings.max() );

    // Export simulation input
    resultCase->setReservoirData( caseData.p() );
    auto exportResult = RigSimulationInputTool::exportSimulationInput( *resultCase, settings, visibility.p() );
    ASSERT_TRUE( exportResult.has_value() ) << "Export failed: " << exportResult.error().toStdString();

    // Verify exported file exists
    ASSERT_TRUE( QFile::exists( exportFilePath ) );

    // Load the exported deck file using RifOpmFlowDeckFile
    RifOpmFlowDeckFile deckFile;
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() ).has_value();
    ASSERT_TRUE( deckLoadResult ) << "Failed to load exported deck file";

    // Padded dimensions: 20x15x15 (original 20x15x10 + 2 upper + 3 lower)
    const size_t expectedCellCount = 20 * 15 * 15; // 4500 cells

    // Verify FIPABC keyword exists and has correct padded size
    {
        auto fipabcKw = deckFile.findKeyword( "FIPABC" );
        ASSERT_TRUE( fipabcKw.has_value() ) << "FIPABC keyword missing from padded exported file";

        const auto& fipabcData = fipabcKw->getIntData();
        EXPECT_EQ( expectedCellCount, fipabcData.size() ) << "FIPABC should have " << expectedCellCount << " values in padded model";

        if ( fipabcData.size() == expectedCellCount )
        {
            const size_t NI        = 20;
            const size_t NJ        = 15;
            const size_t nzUpper   = 2;
            const size_t nzLower   = 3;
            const size_t layerSize = NI * NJ; // 300

            // Upper pad layers (k=0,1) should use minimum value from FIPABC data (value 1)
            for ( size_t k = 0; k < nzUpper; ++k )
                for ( size_t idx = 0; idx < layerSize; ++idx )
                    EXPECT_EQ( 1, fipabcData[k * layerSize + idx] )
                        << "FIPABC upper pad at k=" << k << ", idx=" << idx << " should be 1 (minimum)";

            // Lower pad layers (k=12,13,14) should use minimum value from FIPABC data (value 1)
            for ( size_t k = nzUpper + 10; k < nzUpper + 10 + nzLower; ++k )
                for ( size_t idx = 0; idx < layerSize; ++idx )
                    EXPECT_EQ( 1, fipabcData[k * layerSize + idx] )
                        << "FIPABC lower pad at k=" << k << ", idx=" << idx << " should be 1 (minimum)";
        }
    }
}
