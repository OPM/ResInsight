#include "gtest/gtest.h"

#include "RifSurfaceImporter.h"

#include "RiaTestDataDirectory.h"
#include "RigGocadData.h"

#include "QDir"
#include <QString>
#include <QStringList>

TEST( RifSurfaceImporter, GocadReadValidFile )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceReader/tsurf_eks.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigGocadData gocadData;
    RifSurfaceImporter::readGocadFile( filePath, &gocadData );

    auto surface  = gocadData.gocadGeometry();
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

    QString filename( "RifSurfaceReader/tsurf_invalid.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigGocadData gocadData;
    RifSurfaceImporter::readGocadFile( filePath, &gocadData );

    auto surface  = gocadData.gocadGeometry();
    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)18, vertices.size() );
    EXPECT_EQ( (size_t)15, indices.size() );
}

TEST( RifSurfaceImporter, GocadReadProperties )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceReader/geom_with_properties.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigGocadData gocadData;
    RifSurfaceImporter::readGocadFile( filePath, &gocadData );

    auto surface  = gocadData.gocadGeometry();
    auto vertices = surface.first;
    auto indices  = surface.second;

    std::vector<QString> propNames = gocadData.propertyNames();

    EXPECT_TRUE( propNames.size() == 3 );
    EXPECT_TRUE( propNames[0] == "SX" );

    std::vector<float> propValues_SX = gocadData.propertyValues( propNames[0] );

    float SX_first  = propValues_SX[0];
    float SX_second = propValues_SX[1];
    float SX_last   = propValues_SX[propValues_SX.size() - 1];

    EXPECT_NEAR( 0.000907, SX_first, 1e-4 );
    EXPECT_NEAR( 0.000972, SX_second, 1e-4 );
    EXPECT_NEAR( 0.004293, SX_last, 1e-4 );

    std::vector<float> propValues_SY = gocadData.propertyValues( "SY" );

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

    QString filename( "RifSurfaceReader/tsurf_eks.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigGocadData gocadData;
    RifSurfaceImporter::readGocadFile( filePath, &gocadData );

    std::vector<QString> propNames  = gocadData.propertyNames();
    std::vector<float>   propValues = gocadData.propertyValues( "" );

    EXPECT_TRUE( propNames.size() == 0 );
    EXPECT_TRUE( propValues.size() == 0 );
}

TEST( RifSurfaceImporter, GocadReadNonExistingProperty )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceReader/geom_with_properties.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    RigGocadData gocadData;
    RifSurfaceImporter::readGocadFile( filePath, &gocadData );

    std::vector<float> propValues = gocadData.propertyValues( "NonExistingProperty" );

    EXPECT_TRUE( propValues.size() == 0 );
}

TEST( RifSurfaceImporter, ReadWrongFileType )
{
    QDir baseFolder( TEST_DATA_DIR );

    {
        QString filename( "RifSurfaceReader/test.ptl" );
        QString filePath = baseFolder.absoluteFilePath( filename );
        EXPECT_TRUE( QFile::exists( filePath ) );

        RigGocadData gocadData;
        RifSurfaceImporter::readGocadFile( filePath, &gocadData );

        auto surface  = gocadData.gocadGeometry();
        auto vertices = surface.first;
        auto indices  = surface.second;

        EXPECT_EQ( (size_t)0, vertices.size() );
        EXPECT_EQ( (size_t)0, indices.size() );
    }

    {
        QString filename( "RifSurfaceReader/tsurf_eks.ts" );
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

    QString filename( "RifSurfaceReader/test.ptl" );
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
