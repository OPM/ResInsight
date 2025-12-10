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
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() );
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
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() );
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
    bool               deckLoadResult = deckFile.loadDeck( exportFilePath.toStdString() );
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
