#include "gtest/gtest.h"

#include "RifSurfaceImporter.h"

#include "RiaTestDataDirectory.h"
#include "Surface/RigTriangleMeshData.h"

#include "QDir"
#include <QString>
#include <QStringList>

TEST( RifSurfaceImporter, GocadReadValidFile )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/tsurf_eks.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigTriangleMeshData triangleMeshData;
    RifSurfaceImporter::readGocadFile( filePath, &triangleMeshData );

    auto surface  = triangleMeshData.geometry();
    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)159, vertices.size() );
    EXPECT_EQ( (size_t)759, indices.size() );

    EXPECT_EQ( (size_t)4, indices.front() );
    EXPECT_EQ( (size_t)64, indices.back() );
}

TEST( RifSurfaceImporter, GocadReadWrongIndices )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/tsurf_invalid.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigTriangleMeshData triangleMeshData;
    RifSurfaceImporter::readGocadFile( filePath, &triangleMeshData );

    auto surface  = triangleMeshData.geometry();
    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)18, vertices.size() );
    EXPECT_EQ( (size_t)15, indices.size() );
}

TEST( RifSurfaceImporter, GocadReadProperties )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/geom_with_properties.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigTriangleMeshData triangleMeshData;
    RifSurfaceImporter::readGocadFile( filePath, &triangleMeshData );

    auto surface  = triangleMeshData.geometry();
    auto vertices = surface.first;
    auto indices  = surface.second;

    std::vector<QString> propNames = triangleMeshData.propertyNames();

    EXPECT_TRUE( propNames.size() == 3 );
    EXPECT_TRUE( propNames[0] == "SX" );

    std::vector<float> propValues_SX = triangleMeshData.propertyValues( propNames[0] );

    float SX_first  = propValues_SX[0];
    float SX_second = propValues_SX[1];
    float SX_last   = propValues_SX[propValues_SX.size() - 1];

    EXPECT_NEAR( 0.000907, SX_first, 1e-4 );
    EXPECT_NEAR( 0.000972, SX_second, 1e-4 );
    EXPECT_NEAR( 0.004293, SX_last, 1e-4 );

    std::vector<float> propValues_SY = triangleMeshData.propertyValues( "SY" );

    float SY_first  = propValues_SY[0];
    float SY_second = propValues_SY[1];
    float SY_last   = propValues_SY[propValues_SX.size() - 1];

    EXPECT_NEAR( 0.001550, SY_first, 1e-4 );
    EXPECT_NEAR( 0.001620, SY_second, 1e-4 );
    EXPECT_NEAR( 0.010476, SY_last, 1e-4 );
}

TEST( RifSurfaceImporter, GocadReadNoProperty )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/tsurf_eks.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigTriangleMeshData triangleMeshData;
    RifSurfaceImporter::readGocadFile( filePath, &triangleMeshData );

    std::vector<QString> propNames  = triangleMeshData.propertyNames();
    std::vector<float>   propValues = triangleMeshData.propertyValues( "" );

    EXPECT_TRUE( propNames.empty() );
    EXPECT_TRUE( propValues.empty() );
}

TEST( RifSurfaceImporter, GocadReadNonExistingProperty )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/geom_with_properties.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigTriangleMeshData triangleMeshData;
    RifSurfaceImporter::readGocadFile( filePath, &triangleMeshData );

    std::vector<float> propValues = triangleMeshData.propertyValues( "NonExistingProperty" );

    EXPECT_TRUE( propValues.empty() );
}

TEST( RifSurfaceImporter, ReadWrongFileType )
{
    QDir baseFolder( TEST_DATA_DIR );

    {
        QString filename( "RifSurfaceImporter/test.ptl" );
        QString filePath = baseFolder.absoluteFilePath( filename );
        EXPECT_TRUE( QFile::exists( filePath ) );

        RigTriangleMeshData triangleMeshData;
        RifSurfaceImporter::readGocadFile( filePath, &triangleMeshData );

        auto surface  = triangleMeshData.geometry();
        auto vertices = surface.first;
        auto indices  = surface.second;

        EXPECT_EQ( (size_t)0, vertices.size() );
        EXPECT_EQ( (size_t)0, indices.size() );
    }

    {
        QString filename( "RifSurfaceImporter/tsurf_eks.ts" );
        QString filePath = baseFolder.absoluteFilePath( filename );
        EXPECT_TRUE( QFile::exists( filePath ) );

        auto surface = RifSurfaceImporter::readPetrelFile( filePath );

        auto vertices = surface.first;
        auto indices  = surface.second;

        EXPECT_EQ( (size_t)0, vertices.size() );
        EXPECT_EQ( (size_t)0, indices.size() );
    }
}

TEST( RifSurfaceImporter, ReadPetrelData )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/test.ptl" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceImporter::readPetrelFile( filePath );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)3441, vertices.size() );
    EXPECT_EQ( (size_t)19872, indices.size() );

    EXPECT_EQ( (size_t)0, indices.front() );
    EXPECT_EQ( (size_t)3439, indices.back() );
}

TEST( RifSurfaceImporter, ReadClippedPetrelData )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/test_small_flipped_clipped.ptl" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceImporter::readPetrelFile( filePath );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)8, vertices.size() );
    EXPECT_EQ( (size_t)18, indices.size() );

    EXPECT_EQ( (size_t)0, indices.front() );
    EXPECT_EQ( (size_t)2, indices.back() );

    for ( size_t i = 0; i < indices.size(); i++ )
    {
        EXPECT_TRUE( indices[i] != ( (unsigned)-1 ) );
    }
}

TEST( RifSurfaceImporter, ReadTinyOpenWorksXyzFile )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/tiny-test.dat" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceImporter::readOpenWorksXyzFile( filePath, 0.1 );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)15, vertices.size() );
    EXPECT_EQ( (size_t)24, indices.size() );

    if ( !indices.empty() )
    {
        EXPECT_EQ( (size_t)0, indices.front() );
        EXPECT_EQ( (size_t)11, indices.back() );

        for ( size_t i = 0; i < indices.size(); i++ )
        {
            EXPECT_TRUE( indices[i] != ( (unsigned)-1 ) );
        }
    }
}

TEST( RifSurfaceImporter, ReadTinyOpenWorksXyzFileTabs )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/tiny-test-tabs.dat" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceImporter::readOpenWorksXyzFile( filePath, 0.1 );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)15, vertices.size() );
    EXPECT_EQ( (size_t)24, indices.size() );

    if ( !indices.empty() )
    {
        EXPECT_EQ( (size_t)0, indices.front() );
        EXPECT_EQ( (size_t)11, indices.back() );

        for ( size_t i = 0; i < indices.size(); i++ )
        {
            EXPECT_TRUE( indices[i] != ( (unsigned)-1 ) );
        }
    }
}

TEST( RifSurfaceImporter, ReadLargeOpenWorksXyzFile )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceImporter/norne_xyz.dat" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceImporter::readOpenWorksXyzFile( filePath, 0.1 );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)60805, vertices.size() );
    EXPECT_EQ( (size_t)360792, indices.size() );

    if ( !indices.empty() )
    {
        EXPECT_EQ( (size_t)0, indices.front() );
        EXPECT_EQ( (size_t)60802, indices.back() );

        for ( size_t i = 0; i < indices.size(); i++ )
        {
            EXPECT_TRUE( indices[i] != ( (unsigned)-1 ) );
        }
    }
}

// Test fixture for RifSurfaceImporter
class RifSurfaceImporterTest : public ::testing::Test
{
protected:
    RifSurfaceImporter                 rifSurfaceImporter;
    std::vector<std::vector<unsigned>> indexToPointData;
    std::vector<unsigned>              triangleIndices;
};

TEST_F( RifSurfaceImporterTest, HandlesEmptyInput )
{
    // Empty indexToPointData
    indexToPointData = {};
    EXPECT_FALSE( rifSurfaceImporter.generateTriangleIndices( indexToPointData, 0, 0, triangleIndices ) );
    EXPECT_TRUE( triangleIndices.empty() );
}

TEST_F( RifSurfaceImporterTest, HandlesOutOfBoundsInput )
{
    // Non-empty indexToPointData but invalid indices
    indexToPointData = { { 0, 1 }, { 2, 3 } };
    EXPECT_FALSE( rifSurfaceImporter.generateTriangleIndices( indexToPointData, 2, 0, triangleIndices ) );
    EXPECT_FALSE( rifSurfaceImporter.generateTriangleIndices( indexToPointData, 0, 2, triangleIndices ) );
    EXPECT_TRUE( triangleIndices.empty() );
}

TEST_F( RifSurfaceImporterTest, HandlesSingleResolution )
{
    // Valid input with resolution 1
    indexToPointData = { { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 } };
    EXPECT_TRUE( rifSurfaceImporter.generateTriangleIndices( indexToPointData, 0, 0, triangleIndices, 1 ) );

    // Check expected triangle indices
    std::vector<unsigned> expected = { 0, 1, 4, 0, 4, 3 };
    EXPECT_EQ( triangleIndices, expected );
}

TEST_F( RifSurfaceImporterTest, HandlesLargerResolution )
{
    // Valid input with larger resolution
    indexToPointData = { { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 } };
    EXPECT_TRUE( rifSurfaceImporter.generateTriangleIndices( indexToPointData, 0, 0, triangleIndices, 2 ) );

    // Check expected triangle indices
    std::vector<unsigned> expected = { 0, 2, 8, 0, 8, 6 };
    EXPECT_EQ( triangleIndices, expected );
}

TEST_F( RifSurfaceImporterTest, HandlesInvalidPoints )
{
    // Input with invalid points (-1 as unsigned max value)
    indexToPointData = { { 0, 1, 2 }, { 3, (unsigned)-1, 5 }, { 6, 7, 8 } };
    EXPECT_FALSE( rifSurfaceImporter.generateTriangleIndices( indexToPointData, 0, 1, triangleIndices ) );
    EXPECT_TRUE( triangleIndices.empty() );
}

TEST_F( RifSurfaceImporterTest, PreventsDegenerateTriangles )
{
    // Degenerate case where resolution leads to topI == i or topJ == j
    indexToPointData = { { 0, 1, 2 }, { 3, 4, 5 }, { 6, 7, 8 } };
    EXPECT_FALSE( rifSurfaceImporter.generateTriangleIndices( indexToPointData, 2, 2, triangleIndices, 1 ) );
    EXPECT_TRUE( triangleIndices.empty() );
}
