#include "gtest/gtest.h"

#include "RifSurfaceReader.h"

#include "QDir"
#include "RiaTestDataDirectory.h"
#include <QString>
#include <QStringList>

TEST( RifSurfaceReader, GocadReadValidFile )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceReader/tsurf_eks.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceReader::readGocadFile( filePath );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)159, vertices.size() );
    EXPECT_EQ( (size_t)759, indices.size() );

    EXPECT_EQ( (size_t)4, indices.front() );
    EXPECT_EQ( (size_t)64, indices.back() );
}

TEST( RifSurfaceReader, GocadReadWrongIndices )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceReader/tsurf_invalid.ts" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceReader::readGocadFile( filePath );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)18, vertices.size() );
    EXPECT_EQ( (size_t)15, indices.size() );
}

TEST( RifSurfaceReader, ReadWrongFileType )
{
    QDir baseFolder( TEST_DATA_DIR );

    {
        QString filename( "RifSurfaceReader/test.ptl" );
        QString filePath = baseFolder.absoluteFilePath( filename );
        EXPECT_TRUE( QFile::exists( filePath ) );

        auto surface = RifSurfaceReader::readGocadFile( filePath );

        auto vertices = surface.first;
        auto indices  = surface.second;

        EXPECT_EQ( (size_t)0, vertices.size() );
        EXPECT_EQ( (size_t)0, indices.size() );
    }

    {
        QString filename( "RifSurfaceReader/tsurf_eks.ts" );
        QString filePath = baseFolder.absoluteFilePath( filename );
        EXPECT_TRUE( QFile::exists( filePath ) );

        auto surface = RifSurfaceReader::readPetrelFile( filePath );

        auto vertices = surface.first;
        auto indices  = surface.second;

        EXPECT_EQ( (size_t)0, vertices.size() );
        EXPECT_EQ( (size_t)0, indices.size() );
    }
}

TEST( RifSurfaceReader, ReadPetrelData )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifSurfaceReader/test.ptl" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto surface = RifSurfaceReader::readPetrelFile( filePath );

    auto vertices = surface.first;
    auto indices  = surface.second;

    EXPECT_EQ( (size_t)3441, vertices.size() );
    EXPECT_EQ( (size_t)19872, indices.size() );

    EXPECT_EQ( (size_t)0, indices.front() );
    EXPECT_EQ( (size_t)3439, indices.back() );
}
