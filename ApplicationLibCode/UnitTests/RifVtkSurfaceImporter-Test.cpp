#include "RifVtkImportUtil.h"
#include "RifVtkSurfaceImporter.h"

#include "RiaTestDataDirectory.h"

#include "Surface/RigTriangleMeshData.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <vector>

// Test importing a VTU file
TEST( RifVtkSurfaceImporterTest, ImportFromFile )
{
    // Setup
    std::filesystem::path testDataDir = std::filesystem::path( TEST_DATA_DIR ) / "RifVtkSurfaceImporter";
    std::filesystem::path vtuFilePath = testDataDir / "fracture-00000.vtu";

    // Exercise
    auto triangleMeshData = RifVtkSurfaceImporter::importFromFile( vtuFilePath );

    // Verify
    ASSERT_NE( triangleMeshData, nullptr ) << "Failed to import VTU file";

    // Check vertex count (based on the sample data)
    // Each triangle has 3 vertices and the sample file has 24 triangles (cells)
    const size_t expectedVertexCount         = 24 * 3;
    auto [triangleVertices, triangleIndices] = triangleMeshData->geometry();
    EXPECT_EQ( triangleVertices.size(), expectedVertexCount );

    // Check connectivity size
    EXPECT_EQ( triangleIndices.size(), expectedVertexCount );

    // Check properties
    auto                 propertyNames      = triangleMeshData->propertyNames();
    std::vector<QString> expectedProperties = { "ReservoirCell", "ReservoirPerm", "ReservoirDist" };
    for ( const QString& expectedProperty : expectedProperties )
    {
        EXPECT_TRUE( std::any_of( propertyNames.begin(),
                                  propertyNames.end(),
                                  [expectedProperty]( const QString& name ) { return name == expectedProperty; } ) );
        EXPECT_EQ( triangleMeshData->propertyValues( expectedProperty ).size(), expectedVertexCount );
    }

    // Check that we handled NaN values
    auto values = triangleMeshData->propertyValues( "ReservoirPerm" );
    for ( size_t i = 24; i < 30; i++ )
    {
        ASSERT_TRUE( std::isnan( triangleMeshData->propertyValues( "ReservoirPerm" )[i] ) );
    }
}

// Test parsing a PVD file
TEST( RifVtkSurfaceImporterTest, ParsePvdDatasets )
{
    // Setup
    std::filesystem::path testDataDir = std::filesystem::path( TEST_DATA_DIR ) / "RifVtkSurfaceImporter";
    std::filesystem::path pvdFilePath = testDataDir / "dataset.pvd";

    // Exercise
    auto datasets = RifVtkImportUtil::parsePvdDatasets( pvdFilePath );

    // Verify
    ASSERT_EQ( datasets.size(), 2 ) << "Expected 2 datasets in the PVD file";

    // Check the first dataset
    EXPECT_DOUBLE_EQ( datasets[0].timestep, 1234567.123 );
    EXPECT_EQ( datasets[0].filepath.filename(), "fracture-00000.vtu" );

    // Check the second dataset
    EXPECT_DOUBLE_EQ( datasets[1].timestep, 7654321.765 );
    EXPECT_EQ( datasets[1].filepath.filename(), "fracture-00001.vtu" );

    // Check that file paths are absolute
    EXPECT_TRUE( datasets[0].filepath.is_absolute() );
    EXPECT_TRUE( datasets[1].filepath.is_absolute() );
}

// Test importing multiple VTU files from PVD
TEST( RifVtkSurfaceImporterTest, ImportMultipleTimesteps )
{
    // Setup
    std::filesystem::path testDataDir = std::filesystem::path( TEST_DATA_DIR ) / "RifVtkSurfaceImporter";
    std::filesystem::path pvdFilePath = testDataDir / "dataset.pvd";

    // Exercise
    auto datasets = RifVtkImportUtil::parsePvdDatasets( pvdFilePath );
    ASSERT_FALSE( datasets.empty() ) << "Failed to parse PVD datasets";

    // Import each VTU file
    std::map<QString, std::vector<double>> expectedValues;
    expectedValues["ReservoirPerm"] = { 1.2e-13, 2.2e-13 };
    expectedValues["ReservoirDist"] = { 10.0, 110.0 };
    expectedValues["ReservoirCell"] = { 83769, 73769 };

    int index = 0;
    for ( const auto& dataset : datasets )
    {
        auto triangleMeshData = RifVtkSurfaceImporter::importFromFile( dataset.filepath );

        // Verify
        ASSERT_NE( triangleMeshData, nullptr ) << "Failed to import VTU file: " << dataset.filepath;

        for ( const auto& [name, values] : expectedValues )
        {
            // Check if ReservoirPerm values differ between timesteps
            auto propValues = triangleMeshData->propertyValues( name );
            EXPECT_NEAR( propValues[0], values[index], 1e-15 );
        }

        index++;
    }
}

// Test handling of invalid files
TEST( RifVtkSurfaceImporterTest, HandleInvalidFiles )
{
    std::filesystem::path testDataDir = std::filesystem::path( TEST_DATA_DIR ) / "RifVtkSurfaceImporter";

    // Test with non-existent file
    std::filesystem::path nonExistentFile = testDataDir / "non_existent_file.vtu";
    auto                  result          = RifVtkSurfaceImporter::importFromFile( nonExistentFile );
    EXPECT_EQ( result, nullptr ) << "Expected nullptr for non-existent file";

    // Test with invalid PVD file
    std::filesystem::path invalidPvdFile = testDataDir / "invalid.pvd";
    auto                  datasets       = RifVtkImportUtil::parsePvdDatasets( invalidPvdFile );
    EXPECT_TRUE( datasets.empty() ) << "Expected empty dataset list for invalid PVD file";
}

// Test file path resolution in PVD datasets
TEST( RifVtkSurfaceImporterTest, PvdPathResolution )
{
    // Setup
    std::filesystem::path testDataDir = std::filesystem::path( TEST_DATA_DIR ) / "RifVtkSurfaceImporter";
    std::filesystem::path pvdFilePath = testDataDir / "dataset.pvd";

    // Exercise
    auto datasets = RifVtkImportUtil::parsePvdDatasets( pvdFilePath );
    ASSERT_FALSE( datasets.empty() ) << "Failed to parse PVD datasets";

    // Verify
    for ( const auto& dataset : datasets )
    {
        // Check that file paths are absolute
        EXPECT_TRUE( dataset.filepath.is_absolute() );

        // Check that the resolved path is in the same directory as the PVD file
        EXPECT_EQ( dataset.filepath.parent_path(), pvdFilePath.parent_path() );

        // Check file existence
        EXPECT_TRUE( std::filesystem::exists( dataset.filepath ) ) << "File does not exist: " << dataset.filepath;
    }
}
