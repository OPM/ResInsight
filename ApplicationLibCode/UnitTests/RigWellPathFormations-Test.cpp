#include "gtest/gtest.h"

#include "Well/RigWellPathFormations.h"

#include "RiaWellLogTrackDefines.h"

#include <QString>

#include <vector>

using FormationLevel = RiaDefines::WellLogTrackFormationLevel;
using DepthType      = RiaDefines::DepthType;

namespace
{
RigWellPathFormation formation( const QString& name, double mdTop, double mdBase, double tvdTop = 0.0, double tvdBase = 0.0 )
{
    return RigWellPathFormation{ mdTop, mdBase, tvdTop, tvdBase, name };
}

RigWellPathFormations createHierarchy()
{
    return RigWellPathFormations( { formation( "GARN", 100.0, 200.0 ),
                                    formation( "Garn 1", 100.0, 150.0 ),
                                    formation( "Garn 2", 150.0, 200.0 ),
                                    formation( "Garn 2.1", 150.0, 175.0 ) },
                                  "file",
                                  "well" );
}
} // namespace

TEST( RigWellPathFormations, DetectsLevels )
{
    RigWellPathFormations formations( { formation( "GARN", 0, 1 ),
                                        formation( "Garn", 0, 1 ),
                                        formation( "Ile 2", 0, 1 ),
                                        formation( "Ile 2.1", 0, 1 ),
                                        formation( "Ile 2.1.1", 0, 1 ),
                                        formation( "Tofte 1+2", 0, 1 ),
                                        formation( "Ror 1.2 A1", 0, 1 ),
                                        formation( "Not 1 2", 0, 1 ) },
                                      "file",
                                      "well" );

    std::vector<FormationLevel> expected = { FormationLevel::GROUP,
                                             FormationLevel::LEVEL0,
                                             FormationLevel::LEVEL1,
                                             FormationLevel::LEVEL2,
                                             FormationLevel::LEVEL3,
                                             FormationLevel::UNKNOWN };
    EXPECT_EQ( expected, formations.formationsLevelsPresent() );
}

TEST( RigWellPathFormations, FluidsAreSeparatedFromFormations )
{
    RigWellPathFormations formations( { formation( "Garn", 100.0, 200.0 ), formation( "OIL", 120.0, 140.0 ), formation( "WATER", 140.0, 160.0 ) },
                                      "file",
                                      "well" );

    EXPECT_EQ( 3u, formations.formationNamesCount() );
    EXPECT_EQ( std::vector<FormationLevel>{ FormationLevel::LEVEL0 }, formations.formationsLevelsPresent() );

    // A fluid top replaces a fluid base at the same depth
    auto [names, depths] = formations.depthAndFormationNamesUpToLevel( FormationLevel::NONE, true, DepthType::MEASURED_DEPTH );
    EXPECT_EQ( ( std::vector<QString>{ "OIL Top", "WATER Top", "WATER Base" } ), names );
    EXPECT_EQ( ( std::vector<double>{ 120.0, 140.0, 160.0 } ), depths );

    auto [namesWithFormations, depthsWithFormations] =
        formations.depthAndFormationNamesUpToLevel( FormationLevel::ALL, true, DepthType::MEASURED_DEPTH );
    EXPECT_EQ( ( std::vector<QString>{ "OIL Top", "WATER Top", "WATER Base", "Garn Top", "Garn Base" } ), namesWithFormations );
    EXPECT_EQ( ( std::vector<double>{ 120.0, 140.0, 160.0, 100.0, 200.0 } ), depthsWithFormations );
}

TEST( RigWellPathFormations, AllLevelsSkipsDuplicateDepths )
{
    auto [names, depths] = createHierarchy().depthAndFormationNamesUpToLevel( FormationLevel::ALL, false, DepthType::MEASURED_DEPTH );

    EXPECT_EQ( ( std::vector<QString>{ "GARN Top", "Garn 2 Top", "GARN Base", "Garn 2.1 Base" } ), names );
    EXPECT_EQ( ( std::vector<double>{ 100.0, 150.0, 200.0, 175.0 } ), depths );
}

TEST( RigWellPathFormations, AllLevelsTreatsDepthsWithinToleranceAsEqual )
{
    RigWellPathFormations formations( { formation( "A", 100.0, 110.0 ), formation( "B", 110.05, 120.0 ) }, "file", "well" );

    auto [names, depths] = formations.depthAndFormationNamesUpToLevel( FormationLevel::ALL, false, DepthType::MEASURED_DEPTH );
    EXPECT_EQ( ( std::vector<QString>{ "A Top", "B Top", "B Base" } ), names );
}

TEST( RigWellPathFormations, UpToLevelPrefersDeepestLevelAtSameDepth )
{
    auto formations = createHierarchy();

    {
        auto [names, depths] = formations.depthAndFormationNamesUpToLevel( FormationLevel::GROUP, false, DepthType::MEASURED_DEPTH );
        EXPECT_EQ( ( std::vector<QString>{ "GARN Top", "GARN Base" } ), names );
        EXPECT_EQ( ( std::vector<double>{ 100.0, 200.0 } ), depths );
    }
    {
        auto [names, depths] = formations.depthAndFormationNamesUpToLevel( FormationLevel::LEVEL1, false, DepthType::MEASURED_DEPTH );
        EXPECT_EQ( ( std::vector<QString>{ "Garn 1 Top", "Garn 2 Top", "Garn 2 Base" } ), names );
        EXPECT_EQ( ( std::vector<double>{ 100.0, 150.0, 200.0 } ), depths );
    }
    {
        auto [names, depths] = formations.depthAndFormationNamesUpToLevel( FormationLevel::LEVEL2, false, DepthType::MEASURED_DEPTH );
        EXPECT_EQ( ( std::vector<QString>{ "Garn 1 Top", "Garn 2.1 Top", "Garn 2.1 Base", "Garn 2 Base" } ), names );
        EXPECT_EQ( ( std::vector<double>{ 100.0, 150.0, 175.0, 200.0 } ), depths );
    }
    {
        auto [names, depths] = formations.depthAndFormationNamesUpToLevel( FormationLevel::NONE, false, DepthType::MEASURED_DEPTH );
        EXPECT_TRUE( names.empty() );
        EXPECT_TRUE( depths.empty() );
    }
}

TEST( RigWellPathFormations, UsesTvdWhenRequested )
{
    RigWellPathFormations formations( { formation( "Garn", 100.0, 200.0, -90.0, -180.0 ) }, "file", "well" );

    auto [names, depths] = formations.depthAndFormationNamesUpToLevel( FormationLevel::LEVEL0, false, DepthType::TRUE_VERTICAL_DEPTH );
    EXPECT_EQ( ( std::vector<QString>{ "Garn Base", "Garn Top" } ), names );
    EXPECT_EQ( ( std::vector<double>{ -180.0, -90.0 } ), depths );
}

TEST( RigWellPathFormations, UnsupportedDepthTypeGivesNoPicks )
{
    RigWellPathFormations formations( { formation( "Garn", 100.0, 200.0 ), formation( "OIL", 120.0, 140.0 ) }, "file", "well" );

    for ( auto level : { FormationLevel::ALL, FormationLevel::LEVEL0 } )
    {
        auto [names, depths] = formations.depthAndFormationNamesUpToLevel( level, true, DepthType::TRUE_VERTICAL_DEPTH_RKB );
        EXPECT_TRUE( names.empty() );
        EXPECT_TRUE( depths.empty() );
    }
}

TEST( RigWellPathFormations, DepthRangesUpToLevelReturnsZoneIntervals )
{
    auto formations = createHierarchy();

    {
        auto ranges = formations.depthRangesUpToLevel( FormationLevel::GROUP, DepthType::MEASURED_DEPTH );
        ASSERT_EQ( 1u, ranges.size() );
        EXPECT_EQ( QString( "GARN" ), std::get<0>( ranges[0] ) );
        EXPECT_DOUBLE_EQ( 100.0, std::get<1>( ranges[0] ) );
        EXPECT_DOUBLE_EQ( 200.0, std::get<2>( ranges[0] ) );
    }
    {
        auto ranges = formations.depthRangesUpToLevel( FormationLevel::LEVEL1, DepthType::MEASURED_DEPTH );
        ASSERT_EQ( 3u, ranges.size() );
        EXPECT_EQ( QString( "GARN" ), std::get<0>( ranges[0] ) );
        EXPECT_EQ( QString( "Garn 1" ), std::get<0>( ranges[1] ) );
        EXPECT_EQ( QString( "Garn 2" ), std::get<0>( ranges[2] ) );
    }
    {
        auto ranges = formations.depthRangesUpToLevel( FormationLevel::NONE, DepthType::MEASURED_DEPTH );
        EXPECT_TRUE( ranges.empty() );
    }
}

TEST( RigWellPathFormations, DepthRangesUpToLevelExcludesFluids )
{
    RigWellPathFormations formations( { formation( "Garn", 100.0, 200.0 ), formation( "OIL", 120.0, 140.0 ) }, "file", "well" );

    auto ranges = formations.depthRangesUpToLevel( FormationLevel::ALL, DepthType::MEASURED_DEPTH );
    ASSERT_EQ( 1u, ranges.size() );
    EXPECT_EQ( QString( "Garn" ), std::get<0>( ranges[0] ) );
}

TEST( RigWellPathFormations, DepthRangesUpToLevelUnsupportedDepthTypeGivesNoRanges )
{
    RigWellPathFormations formations( { formation( "Garn", 100.0, 200.0 ) }, "file", "well" );

    auto ranges = formations.depthRangesUpToLevel( FormationLevel::ALL, DepthType::TRUE_VERTICAL_DEPTH_RKB );
    EXPECT_TRUE( ranges.empty() );
}
