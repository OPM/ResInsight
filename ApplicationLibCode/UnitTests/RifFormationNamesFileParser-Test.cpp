#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifFormationNamesFileParser.h"
#include "RigFormationNames.h"

#include "cvfColor3.h"

#include <QDir>
#include <QString>

namespace
{
QString testFile( const QString& relativePath )
{
    return QDir( TEST_DATA_DIR ).absoluteFilePath( "RifFormationNamesReader/" + relativePath );
}
} // namespace

TEST( RifFormationNamesFileParser, ConvertStringToColor )
{
    auto named = RifFormationNamesFileParser::convertStringToColor( "red" );
    ASSERT_TRUE( named.has_value() );
    EXPECT_EQ( 1.0f, named->r() );
    EXPECT_EQ( 0.0f, named->g() );

    auto hex = RifFormationNamesFileParser::convertStringToColor( "#0000FF" );
    ASSERT_TRUE( hex.has_value() );
    EXPECT_EQ( 1.0f, hex->b() );

    EXPECT_FALSE( RifFormationNamesFileParser::convertStringToColor( "" ).has_value() );
    EXPECT_FALSE( RifFormationNamesFileParser::convertStringToColor( "not-a-color" ).has_value() );
}

TEST( RifFormationNamesFileParser, ReadLyrFile )
{
    auto fm = RifFormationNamesFileParser::readLyrFormationNameFile( testFile( "Norne_ATW2013ColorName.lyr" ) );
    ASSERT_TRUE( fm.has_value() );
    EXPECT_EQ( "Garn 3", fm->formationNameFromKLayerIdx( 0 ) );

    cvf::Color3f color;
    ASSERT_TRUE( fm->formationColorFromKLayerIdx( 1, &color ) );
    EXPECT_EQ( 1.0f, color.r() );

    EXPECT_FALSE( RifFormationNamesFileParser::readLyrFormationNameFile( testFile( "missing.lyr" ) ).has_value() );
}

TEST( RifFormationNamesFileParser, ReadFmuFileWithExplicitColors )
{
    std::map<QString, cvf::Color3f> colors = { { "Therys", cvf::Color3f( 0.0f, 1.0f, 0.0f ) } };

    auto fm = RifFormationNamesFileParser::readFmuFormationNameFile( testFile( "fmu_nocolors/layer_zone_table.txt" ), colors );
    ASSERT_TRUE( fm.has_value() );

    cvf::Color3f color;
    EXPECT_FALSE( fm->formationColorFromKLayerIdx( 0, &color ) );
    ASSERT_TRUE( fm->formationColorFromKLayerIdx( 1, &color ) );
    EXPECT_EQ( 1.0f, color.g() );

    EXPECT_FALSE( RifFormationNamesFileParser::readFmuFormationNameFile( testFile( "missing.txt" ) ).has_value() );
}

TEST( RifFormationNamesFileParser, ReadYamlFile )
{
    auto fm = RifFormationNamesFileParser::readYamlFormationNameFile( testFile( "fmu/zonation_ecl_map.yml" ) );
    ASSERT_TRUE( fm.has_value() );

    // Invalid and unrelated lines are skipped
    EXPECT_EQ( 3u, fm->formationNames().size() );
    EXPECT_EQ( "Valysar", fm->formationNameFromKLayerIdx( 0 ) );
    EXPECT_EQ( "Therys", fm->formationNameFromKLayerIdx( 20 ) );
    EXPECT_EQ( "Volon", fm->formationNameFromKLayerIdx( 54 ) );

    cvf::Color3f color;
    EXPECT_FALSE( fm->formationColorFromKLayerIdx( 0, &color ) );
}

TEST( RifFormationNamesFileParser, ReadYamlFileWithColorsAndErrors )
{
    std::map<QString, cvf::Color3f> colors = { { "Valysar", cvf::Color3f( 1.0f, 0.0f, 0.0f ) } };

    auto fm = RifFormationNamesFileParser::readYamlFormationNameFile( testFile( "fmu/zonation_ecl_map.yml" ), colors );
    ASSERT_TRUE( fm.has_value() );

    cvf::Color3f color;
    ASSERT_TRUE( fm->formationColorFromKLayerIdx( 5, &color ) );
    EXPECT_EQ( 1.0f, color.r() );

    EXPECT_FALSE( RifFormationNamesFileParser::readYamlFormationNameFile( testFile( "fmu_malformed/zonation_ecl_map.yaml" ) ).has_value() );
    EXPECT_FALSE( RifFormationNamesFileParser::readYamlFormationNameFile( testFile( "missing.yml" ) ).has_value() );
}

TEST( RifFormationNamesFileParser, ReadStratigraphyColorsSubzonePrecedence )
{
    auto colors = RifFormationNamesFileParser::readStratigraphyColors( testFile( "fmu/stratigraphy.json" ) );
    ASSERT_TRUE( colors.has_value() );

    EXPECT_EQ( 1.0f, colors->at( "Valysar" ).r() );
    EXPECT_EQ( 1.0f, colors->at( "Therys" ).b() );
    EXPECT_EQ( 1u, colors->count( "Other" ) );
    EXPECT_EQ( 0u, colors->count( "Volon" ) );

    EXPECT_FALSE( RifFormationNamesFileParser::readStratigraphyColors( testFile( "missing.json" ) ).has_value() );
}
