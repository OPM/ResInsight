#include "gtest/gtest.h"

#include "RifGriMetadataFooter.h"
#include "RifSurfio.h"

#include <QFile>
#include <QTemporaryDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static RigRegularSurfaceData createSurfaceData( int nx, int ny )
{
    RigRegularSurfaceData surfaceData;
    surfaceData.nx         = nx;
    surfaceData.ny         = ny;
    surfaceData.originX    = 1000.0;
    surfaceData.originY    = 2000.0;
    surfaceData.incrementX = 50.0;
    surfaceData.incrementY = 50.0;
    surfaceData.rotation   = 0.0;
    return surfaceData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifGriMetadataFooter, RoundTripFooter )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    const QString fileName = tempDir.filePath( "footer.gri" );

    const std::vector<float> values = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
    ASSERT_TRUE( RifSurfio::exportToGri( fileName.toStdString(), createSurfaceData( 3, 2 ), values ) );

    const std::vector<std::pair<QString, QString>> keyValues = { { "validityKey", "e60c3dfc979d4b5bfc14d1b3a29b8e2a" },
                                                                 { "statisticsType", "MEAN" },
                                                                 { "timeStep", "36" },
                                                                 { "sampleSpacing", "50.100000000000001" } };

    ASSERT_TRUE( RifGriMetadataFooter::appendFooter( fileName, keyValues ) );

    auto footer = RifGriMetadataFooter::readFooter( fileName );
    ASSERT_TRUE( footer.has_value() );
    ASSERT_EQ( keyValues.size(), footer->size() );

    for ( const auto& [key, value] : keyValues )
    {
        ASSERT_TRUE( footer->contains( key ) );
        EXPECT_EQ( value, footer->at( key ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// The footer must not affect the surface import: a GRI file with a footer must import with
/// identical header and values as the same file without a footer
//--------------------------------------------------------------------------------------------------
TEST( RifGriMetadataFooter, SurfaceImportUnaffectedByFooter )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    const QString cleanFileName  = tempDir.filePath( "clean.gri" );
    const QString footerFileName = tempDir.filePath( "with_footer.gri" );

    const std::vector<float> values = { 0.5f, -1.25f, 3.75f, 100.0f, -200.0f, 0.0f };

    ASSERT_TRUE( RifSurfio::exportToGri( cleanFileName.toStdString(), createSurfaceData( 3, 2 ), values ) );
    ASSERT_TRUE( QFile::copy( cleanFileName, footerFileName ) );
    ASSERT_TRUE( RifGriMetadataFooter::appendFooter( footerFileName, { { "validityKey", "abc123" }, { "timeStep", "36" } } ) );

    auto cleanImport  = RifSurfio::importSurfaceData( cleanFileName.toStdString() );
    auto footerImport = RifSurfio::importSurfaceData( footerFileName.toStdString() );

    ASSERT_TRUE( cleanImport.has_value() );
    ASSERT_TRUE( footerImport.has_value() );

    EXPECT_EQ( cleanImport->first.nx, footerImport->first.nx );
    EXPECT_EQ( cleanImport->first.ny, footerImport->first.ny );
    EXPECT_EQ( cleanImport->second, footerImport->second );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifGriMetadataFooter, MissingOrInvalidFooter )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // File without a footer
    const QString cleanFileName = tempDir.filePath( "clean.gri" );

    const std::vector<float> values = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
    ASSERT_TRUE( RifSurfio::exportToGri( cleanFileName.toStdString(), createSurfaceData( 3, 2 ), values ) );
    EXPECT_FALSE( RifGriMetadataFooter::readFooter( cleanFileName ).has_value() );

    // Non-existing file
    EXPECT_FALSE( RifGriMetadataFooter::readFooter( tempDir.filePath( "does_not_exist.gri" ) ).has_value() );

    // Footer with malformed content
    const QString malformedFileName = tempDir.filePath( "malformed.gri" );
    {
        QFile file( malformedFileName );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
        file.write( ( "some binary content\n" + RifGriMetadataFooter::markerLine() + "\nno separator here\n" ).toUtf8() );
    }
    EXPECT_FALSE( RifGriMetadataFooter::readFooter( malformedFileName ).has_value() );
}
