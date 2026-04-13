#include "RifVtkImportUtil.h"

#include "RiaTestDataDirectory.h"

#include <gtest/gtest.h>

#include "pugixml.hpp"

#include <cmath>
#include <filesystem>
#include <string>

// Helper: parse an XML string and return the <Piece> child of <UnstructuredGrid>
static pugi::xml_node parsePiece( pugi::xml_document& doc, const std::string& xml )
{
    doc.load_string( xml.c_str() );
    return doc.child( "UnstructuredGrid" ).child( "Piece" );
}

//--------------------------------------------------------------------------------------------------
// parsePvdDatasets
//--------------------------------------------------------------------------------------------------

TEST( RifVtkImportUtilTest, ParsePvdDatasets_ReturnsCorrectCount )
{
    std::filesystem::path pvdPath = std::filesystem::path( TEST_DATA_DIR ) / "RifVtkReader" / "model.pvd";

    auto datasets = RifVtkImportUtil::parsePvdDatasets( pvdPath );

    ASSERT_EQ( datasets.size(), 2u );
    EXPECT_DOUBLE_EQ( datasets[0].timestep, 1.0 );
    EXPECT_DOUBLE_EQ( datasets[1].timestep, 2.0 );
}

TEST( RifVtkImportUtilTest, ParsePvdDatasets_PathsAreAbsolute )
{
    std::filesystem::path pvdPath = std::filesystem::path( TEST_DATA_DIR ) / "RifVtkReader" / "model.pvd";

    auto datasets = RifVtkImportUtil::parsePvdDatasets( pvdPath );

    for ( const auto& ds : datasets )
    {
        EXPECT_TRUE( ds.filepath.is_absolute() );
        EXPECT_TRUE( std::filesystem::exists( ds.filepath ) );
    }
}

TEST( RifVtkImportUtilTest, ParsePvdDatasets_MissingFile )
{
    auto datasets = RifVtkImportUtil::parsePvdDatasets( "nonexistent.pvd" );
    EXPECT_TRUE( datasets.empty() );
}

//--------------------------------------------------------------------------------------------------
// readPoints
//--------------------------------------------------------------------------------------------------

TEST( RifVtkImportUtilTest, ReadPoints_ReturnsCoordinatesWithNegatedZ )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
            <Points>
              <DataArray type="Float32" Name="Coordinates" NumberOfComponents="3" format="ascii">
                1 2 3  4 5 6
              </DataArray>
            </Points>
          </Piece>
        </UnstructuredGrid>)" );

    auto points = RifVtkImportUtil::readPoints( piece );

    ASSERT_EQ( points.size(), 2u );
    EXPECT_DOUBLE_EQ( points[0].x(), 1.0 );
    EXPECT_DOUBLE_EQ( points[0].y(), 2.0 );
    EXPECT_DOUBLE_EQ( points[0].z(), -3.0 ); // z is negated
    EXPECT_DOUBLE_EQ( points[1].x(), 4.0 );
    EXPECT_DOUBLE_EQ( points[1].y(), 5.0 );
    EXPECT_DOUBLE_EQ( points[1].z(), -6.0 ); // z is negated
}

TEST( RifVtkImportUtilTest, ReadPoints_MissingPointsSection )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
          </Piece>
        </UnstructuredGrid>)" );

    auto points = RifVtkImportUtil::readPoints( piece );
    EXPECT_TRUE( points.empty() );
}

TEST( RifVtkImportUtilTest, ReadPoints_WrongDataArrayName )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
            <Points>
              <DataArray type="Float32" Name="Points" NumberOfComponents="3" format="ascii">
                1 2 3
              </DataArray>
            </Points>
          </Piece>
        </UnstructuredGrid>)" );

    auto points = RifVtkImportUtil::readPoints( piece );
    EXPECT_TRUE( points.empty() );
}

//--------------------------------------------------------------------------------------------------
// readDisplacements
//--------------------------------------------------------------------------------------------------

TEST( RifVtkImportUtilTest, ReadDisplacements_ReturnsVectorsWithNegatedZ )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
            <PointData>
              <DataArray type="Float32" Name="disp" NumberOfComponents="3" format="ascii">
                0.1 0.2 0.3  -0.4 -0.5 -0.6
              </DataArray>
            </PointData>
          </Piece>
        </UnstructuredGrid>)" );

    auto displacements = RifVtkImportUtil::readDisplacements( piece );

    ASSERT_EQ( displacements.size(), 2u );
    EXPECT_NEAR( displacements[0].x(), 0.1f, 1e-6f );
    EXPECT_NEAR( displacements[0].y(), 0.2f, 1e-6f );
    EXPECT_NEAR( displacements[0].z(), -0.3f, 1e-6f ); // z is negated
    EXPECT_NEAR( displacements[1].x(), -0.4f, 1e-6f );
    EXPECT_NEAR( displacements[1].y(), -0.5f, 1e-6f );
    EXPECT_NEAR( displacements[1].z(), 0.6f, 1e-6f ); // z is negated
}

TEST( RifVtkImportUtilTest, ReadDisplacements_MissingPointDataSection )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
          </Piece>
        </UnstructuredGrid>)" );

    auto displacements = RifVtkImportUtil::readDisplacements( piece );
    EXPECT_TRUE( displacements.empty() );
}

TEST( RifVtkImportUtilTest, ReadDisplacements_WrongFieldName )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
            <PointData>
              <DataArray type="Float32" Name="velocity" NumberOfComponents="3" format="ascii">
                1 2 3
              </DataArray>
            </PointData>
          </Piece>
        </UnstructuredGrid>)" );

    auto displacements = RifVtkImportUtil::readDisplacements( piece );
    EXPECT_TRUE( displacements.empty() );
}

//--------------------------------------------------------------------------------------------------
// readConnectivity
//--------------------------------------------------------------------------------------------------

TEST( RifVtkImportUtilTest, ReadConnectivity_ReturnsIndices )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
            <Cells>
              <DataArray type="Int32" Name="connectivity" format="ascii">
                0 1 4 3 6 7 10 9
                1 2 5 4 7 8 11 10
              </DataArray>
              <DataArray type="Int32" Name="offsets" format="ascii">8 16</DataArray>
              <DataArray type="UInt8" Name="types" format="ascii">12 12</DataArray>
            </Cells>
          </Piece>
        </UnstructuredGrid>)" );

    auto connectivity = RifVtkImportUtil::readConnectivity( piece );

    ASSERT_EQ( connectivity.size(), 16u );
    EXPECT_EQ( connectivity[0], 0u );
    EXPECT_EQ( connectivity[1], 1u );
    EXPECT_EQ( connectivity[7], 9u );
    EXPECT_EQ( connectivity[8], 1u );
    EXPECT_EQ( connectivity[15], 10u );
}

TEST( RifVtkImportUtilTest, ReadConnectivity_MissingCellsSection )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
          </Piece>
        </UnstructuredGrid>)" );

    auto connectivity = RifVtkImportUtil::readConnectivity( piece );
    EXPECT_TRUE( connectivity.empty() );
}

//--------------------------------------------------------------------------------------------------
// readProperties
//--------------------------------------------------------------------------------------------------

TEST( RifVtkImportUtilTest, ReadProperties_ReturnsNamedScalars )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
            <CellData>
              <DataArray type="Float32" Name="Stress" format="ascii">100.0 200.0</DataArray>
              <DataArray type="Float32" Name="Pressure" format="ascii">1.5 2.5</DataArray>
            </CellData>
          </Piece>
        </UnstructuredGrid>)" );

    auto properties = RifVtkImportUtil::readProperties( piece );

    ASSERT_EQ( properties.size(), 2u );
    ASSERT_TRUE( properties.count( "Stress" ) > 0 );
    ASSERT_TRUE( properties.count( "Pressure" ) > 0 );

    ASSERT_EQ( properties["Stress"].size(), 2u );
    EXPECT_FLOAT_EQ( properties["Stress"][0], 100.0f );
    EXPECT_FLOAT_EQ( properties["Stress"][1], 200.0f );

    ASSERT_EQ( properties["Pressure"].size(), 2u );
    EXPECT_FLOAT_EQ( properties["Pressure"][0], 1.5f );
    EXPECT_FLOAT_EQ( properties["Pressure"][1], 2.5f );
}

TEST( RifVtkImportUtilTest, ReadProperties_HandlesNaN )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
            <CellData>
              <DataArray type="Float32" Name="Perm" format="ascii">1.0 nan 3.0</DataArray>
            </CellData>
          </Piece>
        </UnstructuredGrid>)" );

    auto properties = RifVtkImportUtil::readProperties( piece );

    ASSERT_TRUE( properties.count( "Perm" ) > 0 );
    ASSERT_EQ( properties["Perm"].size(), 3u );
    EXPECT_FLOAT_EQ( properties["Perm"][0], 1.0f );
    EXPECT_TRUE( std::isnan( properties["Perm"][1] ) );
    EXPECT_FLOAT_EQ( properties["Perm"][2], 3.0f );
}

TEST( RifVtkImportUtilTest, ReadProperties_MissingCellDataSection )
{
    pugi::xml_document doc;
    auto               piece = parsePiece( doc, R"(
        <UnstructuredGrid>
          <Piece>
          </Piece>
        </UnstructuredGrid>)" );

    auto properties = RifVtkImportUtil::readProperties( piece );
    EXPECT_TRUE( properties.empty() );
}
