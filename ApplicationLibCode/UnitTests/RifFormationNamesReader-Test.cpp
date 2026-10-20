#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifFormationNamesFileParser.h"
#include "RifFormationNamesReader.h"
#include "RigFormationNames.h"

#include "cvfColor3.h"

#include <QDir>
#include <QString>
#include <QStringList>

TEST( RifFormationNamesReader, ReadLYRFileWithoutColor )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifFormationNamesReader/Norne_ATW2013.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto fm = RifFormationNamesReader::readFormationNamesFile( filePath );
    ASSERT_TRUE( fm.has_value() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 0 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    EXPECT_TRUE( formationName_K1 == "Garn 3" );
    EXPECT_EQ( 1, formationIndex );
}

TEST( RifFormationNamesReader, ReadLYRFileWithColorName )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifFormationNamesReader/Norne_ATW2013ColorName.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto fm = RifFormationNamesReader::readFormationNamesFile( filePath );
    ASSERT_TRUE( fm.has_value() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 1 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    cvf::Color3f formationColor;
    bool         colorPresent = fm->formationColorFromKLayerIdx( 1, &formationColor );
    EXPECT_TRUE( colorPresent );

    EXPECT_TRUE( formationName_K1 == "Garn 2" );
    EXPECT_EQ( 1, formationIndex );
    EXPECT_EQ( 1.0f, formationColor.r() );
    EXPECT_EQ( 0.0f, formationColor.g() );
    EXPECT_EQ( 0.0f, formationColor.b() );
}

TEST( RifFormationNamesReader, ReadLYRFileWithColorHTML )
{
    QDir baseFolder( TEST_DATA_DIR );

    const QString filename( "RifFormationNamesReader/Norne_ATW2013ColorHTML.lyr" );
    const QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto fm = RifFormationNamesReader::readFormationNamesFile( filePath );
    ASSERT_TRUE( fm.has_value() );

    QString formationName_K1 = fm->formationNameFromKLayerIdx( 1 );
    int     formationIndex   = fm->formationIndexFromKLayerIdx( 1 );

    cvf::Color3f formationColor;
    bool         colorPresent = fm->formationColorFromKLayerIdx( 1, &formationColor );
    EXPECT_TRUE( colorPresent );

    EXPECT_TRUE( formationName_K1 == "Garn 2" );
    EXPECT_EQ( 1, formationIndex );
    EXPECT_EQ( 1.0f, formationColor.r() );
    EXPECT_EQ( 0.0f, formationColor.g() );
    EXPECT_EQ( 0.0f, formationColor.b() );
}

TEST( RifFormationNamesReader, ReadFmuFormationNamesWithStratigraphyColors )
{
    QDir baseFolder( TEST_DATA_DIR );

    auto fm = RifFormationNamesReader::readFormationNamesFile(
        baseFolder.absoluteFilePath( "RifFormationNamesReader/fmu/layer_zone_table.txt" ) );
    ASSERT_TRUE( fm.has_value() );

    cvf::Color3f color;
    ASSERT_TRUE( fm->formationColorFromKLayerIdx( 0, &color ) );
    EXPECT_EQ( 1.0f, color.r() );
    EXPECT_EQ( 0.0f, color.g() );

    ASSERT_TRUE( fm->formationColorFromKLayerIdx( 2, &color ) );
    EXPECT_EQ( 1.0f, color.b() );

    // Invalid color in json: formation keeps default coloring
    EXPECT_FALSE( fm->formationColorFromKLayerIdx( 4, &color ) );

    ASSERT_TRUE( fm->formationColorFromKLayerIdx( 5, &color ) );
    EXPECT_NEAR( 128.0f / 255.0f, color.r(), 1e-3 );
}

TEST( RifFormationNamesReader, ReadStratigraphyColorsParentFallback )
{
    QDir baseFolder( TEST_DATA_DIR );

    auto colors = RifFormationNamesFileParser::readStratigraphyColors(
        baseFolder.absoluteFilePath( "RifFormationNamesReader/fmu/stratigraphy.json" ) );
    ASSERT_TRUE( colors.has_value() );

    ASSERT_EQ( 1u, colors->count( "TopVolantis_BaseVolantis" ) );
    EXPECT_EQ( 1.0f, colors->at( "TopVolantis_BaseVolantis" ).g() );
    EXPECT_EQ( 0u, colors->count( "Volon" ) );
}

TEST( RifFormationNamesReader, ReadFmuFormationNamesIgnoresBadOrMissingStratigraphy )
{
    QDir baseFolder( TEST_DATA_DIR );

    for ( const QString folder : { "fmu_malformed", "fmu_nocolors" } )
    {
        auto fm = RifFormationNamesReader::readFormationNamesFile(
            baseFolder.absoluteFilePath( "RifFormationNamesReader/" + folder + "/layer_zone_table.txt" ) );
        ASSERT_TRUE( fm.has_value() );
        EXPECT_EQ( "Therys", fm->formationNameFromKLayerIdx( 1 ) );

        cvf::Color3f color;
        EXPECT_FALSE( fm->formationColorFromKLayerIdx( 0, &color ) );
    }

    auto bad = RifFormationNamesFileParser::readStratigraphyColors(
        baseFolder.absoluteFilePath( "RifFormationNamesReader/fmu_malformed/stratigraphy.json" ) );
    EXPECT_FALSE( bad.has_value() );
}

TEST( RifFormationNamesReader, ReadYamlZoneRangesWithStratigraphyColors )
{
    QDir baseFolder( TEST_DATA_DIR );

    auto fm = RifFormationNamesReader::readFormationNamesFile(
        baseFolder.absoluteFilePath( "RifFormationNamesReader/fmu/zonation_ecl_map.yml" ) );
    ASSERT_TRUE( fm.has_value() );

    EXPECT_EQ( 3u, fm->formationNames().size() );
    EXPECT_EQ( "Valysar", fm->formationNameFromKLayerIdx( 0 ) );
    EXPECT_EQ( "Valysar", fm->formationNameFromKLayerIdx( 19 ) );
    EXPECT_EQ( "Therys", fm->formationNameFromKLayerIdx( 20 ) );
    EXPECT_EQ( "Volon", fm->formationNameFromKLayerIdx( 68 ) );

    cvf::Color3f color;
    ASSERT_TRUE( fm->formationColorFromKLayerIdx( 0, &color ) );
    EXPECT_EQ( 1.0f, color.r() );
    EXPECT_FALSE( fm->formationColorFromKLayerIdx( 60, &color ) );
}

TEST( RifFormationNamesReader, ReadYamlWithoutZrangesFails )
{
    QDir baseFolder( TEST_DATA_DIR );

    auto fm = RifFormationNamesReader::readFormationNamesFile(
        baseFolder.absoluteFilePath( "RifFormationNamesReader/fmu_malformed/zonation_ecl_map.yaml" ) );
    EXPECT_FALSE( fm.has_value() );
}

TEST( RifFormationNamesReader, OtherYamlFilesAreNotRead )
{
    QDir baseFolder( TEST_DATA_DIR );

    auto fm = RifFormationNamesReader::readFormationNamesFile( baseFolder.absoluteFilePath( "RifFormationNamesReader/fmu/other.yml" ) );
    ASSERT_TRUE( fm.has_value() );
    EXPECT_TRUE( fm->formationNames().empty() );
}
