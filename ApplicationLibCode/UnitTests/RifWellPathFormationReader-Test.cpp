#include "gtest/gtest.h"

#include "RifWellPathFormationReader.h"

#include "RiaWellLogTrackDefines.h"

#include <QString>

#include <vector>

namespace
{
std::pair<std::vector<QString>, std::vector<double>> allFormations( const RigWellPathFormations& formations, RiaDefines::DepthType depthType )
{
    return formations.depthAndFormationNamesUpToLevel( RiaDefines::WellLogTrackFormationLevel::ALL, false, depthType );
}
} // namespace

TEST( RifWellPathFormationReader, ParseMdAndTvd )
{
    const QString content = "WELLNAME;UNITNAME;TOPMD;BASEMD;TOPTVDSS;BASETVDSS\n"
                            "A-1;Upper zone;100.0;110.0;90.0;95.0\n"
                            "A-1;Lower zone;120.0;130.0;98.0;104.0\n"
                            "B-2;Upper zone;200.0;210.0;180.0;185.0\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "picks.txt" );
    ASSERT_TRUE( result.has_value() );
    ASSERT_EQ( 2u, result->size() );

    const auto& wellA = result->at( "A-1" );
    EXPECT_EQ( 2u, wellA.formationNamesCount() );
    EXPECT_EQ( "picks.txt", wellA.filePath() );
    EXPECT_EQ( "A-1", wellA.keyInFile() );

    auto [mdNames, mdDepths] = allFormations( wellA, RiaDefines::DepthType::MEASURED_DEPTH );
    EXPECT_EQ( ( std::vector<QString>{ "Upper zone Top", "Lower zone Top", "Upper zone Base", "Lower zone Base" } ), mdNames );
    EXPECT_EQ( ( std::vector<double>{ 100.0, 120.0, 110.0, 130.0 } ), mdDepths );

    // TVDSS is stored with negated sign
    auto [tvdNames, tvdDepths] = allFormations( wellA, RiaDefines::DepthType::TRUE_VERTICAL_DEPTH );
    EXPECT_EQ( ( std::vector<double>{ -90.0, -98.0, -95.0, -104.0 } ), tvdDepths );

    EXPECT_EQ( 1u, result->at( "B-2" ).formationNamesCount() );
}

TEST( RifWellPathFormationReader, ParseMdOnly )
{
    const QString content = "wellname;unitname;topmd;basemd\n"
                            "A-1;Zone;100.0;110.0\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "picks.txt" );
    ASSERT_TRUE( result.has_value() );

    auto [names, depths] = allFormations( result->at( "A-1" ), RiaDefines::DepthType::MEASURED_DEPTH );
    EXPECT_EQ( ( std::vector<double>{ 100.0, 110.0 } ), depths );
}

TEST( RifWellPathFormationReader, ParseTvdOnly )
{
    const QString content = "wellname;unitname;toptvdss;basetvdss\n"
                            "A-1;Zone;90.0;95.0\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "picks.txt" );
    ASSERT_TRUE( result.has_value() );

    auto [names, depths] = allFormations( result->at( "A-1" ), RiaDefines::DepthType::TRUE_VERTICAL_DEPTH );
    EXPECT_EQ( ( std::vector<double>{ -90.0, -95.0 } ), depths );
}

TEST( RifWellPathFormationReader, HeaderAfterShortLinesWithWhitespaceAndCrLf )
{
    const QString content = "Exported picks\r\n"
                            "\r\n"
                            " Well Name ; Unit Name ; Top MD ; Base MD \r\n"
                            "A-1; Zone ;100.0;110.0\r\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "picks.txt" );
    ASSERT_TRUE( result.has_value() );

    auto [names, depths] = allFormations( result->at( "A-1" ), RiaDefines::DepthType::MEASURED_DEPTH );
    EXPECT_EQ( ( std::vector<QString>{ "Zone Top", "Zone Base" } ), names );
    EXPECT_EQ( ( std::vector<double>{ 100.0, 110.0 } ), depths );
}

TEST( RifWellPathFormationReader, SkipsMalformedAndEmptyRows )
{
    const QString content = "wellname;unitname;topmd;basemd\n"
                            "A-1;Zone;100.0\n"
                            ";;;\n"
                            "A-1;Zone;100.0;110.0\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "picks.txt" );
    ASSERT_TRUE( result.has_value() );
    EXPECT_EQ( 1u, result->size() );
    EXPECT_EQ( 1u, result->at( "A-1" ).formationNamesCount() );
}

TEST( RifWellPathFormationReader, ErrorWhenNoDepthColumns )
{
    const QString content = "wellname;unitname;topmd;something\n"
                            "A-1;Zone;100.0;110.0\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "picks.txt" );
    ASSERT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "Neither MD or TVD" ) );
}

TEST( RifWellPathFormationReader, ErrorWhenRequiredColumnsMissing )
{
    const QString content = "wellname;formation;topmd;basemd\n"
                            "A-1;Zone;100.0;110.0\n";

    EXPECT_FALSE( RifWellPathFormationReader::parseWellFormations( content, "picks.txt" ).has_value() );
}

TEST( RifWellPathFormationReader, ErrorWhenNoRows )
{
    EXPECT_FALSE( RifWellPathFormationReader::parseWellFormations( "", "picks.txt" ).has_value() );
    EXPECT_FALSE( RifWellPathFormationReader::parseWellFormations( "wellname;unitname;topmd;basemd\n", "picks.txt" ).has_value() );
}

TEST( RifWellPathFormationReader, ErrorWhenFileMissing )
{
    EXPECT_FALSE( RifWellPathFormationReader::readWellFormations( "this/file/does/not/exist.txt" ).has_value() );
}

TEST( RifWellPathFormationReader, ParseFmuFormationsCsv )
{
    const QString content = "X_UTME,Y_UTMN,TOP_TVD,TOP_MD,ZONE_CODE,WELL,BASE_TVD,BASE_MD,ZONE\n"
                            "460994.9,5933813.29,1644.0,1693.0,1,R_A2,1662.0,1711.0,Valysar\n"
                            "460994.9,5933813.29,1662.0,1711.0,2,R_A2,1678.0,1727.0,Therys\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "formations.csv" );
    ASSERT_TRUE( result.has_value() );
    ASSERT_EQ( 1u, result->size() );

    const auto& well = result->at( "R_A2" );
    EXPECT_EQ( 2u, well.formationNamesCount() );

    auto [mdNames, mdDepths] = allFormations( well, RiaDefines::DepthType::MEASURED_DEPTH );
    // The base of Valysar coincides with the top of Therys, so it is deduplicated like adjacent "unitname" zones
    EXPECT_EQ( ( std::vector<QString>{ "Valysar Top", "Therys Top", "Therys Base" } ), mdNames );
    EXPECT_EQ( ( std::vector<double>{ 1693.0, 1711.0, 1727.0 } ), mdDepths );

    // TOP_TVD/BASE_TVD (without the "SS" suffix) are already positive-down, and are used as-is
    auto [tvdNames, tvdDepths] = allFormations( well, RiaDefines::DepthType::TRUE_VERTICAL_DEPTH );
    EXPECT_EQ( ( std::vector<double>{ 1644.0, 1662.0, 1678.0 } ), tvdDepths );
}

TEST( RifWellPathFormationReader, ParseFmuFormationsCsvIncludesTopXY )
{
    const QString content = "X_UTME,Y_UTMN,TOP_TVD,TOP_MD,ZONE_CODE,WELL,BASE_TVD,BASE_MD,ZONE\n"
                            "460994.9,5933813.29,1644.0,1693.0,1,R_A2,1662.0,1711.0,Valysar\n"
                            "461200.1,5933900.5,1662.0,1711.0,2,R_A2,1678.0,1727.0,Therys\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "formations.csv" );
    ASSERT_TRUE( result.has_value() );

    const auto& well = result->at( "R_A2" );
    ASSERT_TRUE( well.formationAt( 0 ).topXY.has_value() );
    EXPECT_DOUBLE_EQ( 460994.9, well.formationAt( 0 ).topXY->x() );
    EXPECT_DOUBLE_EQ( 5933813.29, well.formationAt( 0 ).topXY->y() );

    ASSERT_TRUE( well.formationAt( 1 ).topXY.has_value() );
    EXPECT_DOUBLE_EQ( 461200.1, well.formationAt( 1 ).topXY->x() );
    EXPECT_DOUBLE_EQ( 5933900.5, well.formationAt( 1 ).topXY->y() );
}

TEST( RifWellPathFormationReader, ParseFmuFormationsCsvWithoutXY )
{
    const QString content = "TOP_MD,WELL,BASE_MD,ZONE\n"
                            "1693.0,R_A2,1711.0,Valysar\n";

    auto result = RifWellPathFormationReader::parseWellFormations( content, "formations.csv" );
    ASSERT_TRUE( result.has_value() );

    const auto& well = result->at( "R_A2" );
    EXPECT_FALSE( well.formationAt( 0 ).topXY.has_value() );
}
