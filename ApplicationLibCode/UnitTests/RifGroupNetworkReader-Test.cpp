/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifGroupNetworkReader.h"

#include <QDateTime>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimeZone>

namespace
{
//--------------------------------------------------------------------------------------------------
std::time_t utcDate( int year, int month, int day )
{
    return QDateTime( QDate( year, month, day ), QTime( 0, 0 ), QTimeZone::utc() ).toSecsSinceEpoch();
}

//--------------------------------------------------------------------------------------------------
std::map<std::string, std::string> parents( const RigGroupNetwork::TreeRevision& tree )
{
    std::map<std::string, std::string> result;
    for ( const auto& edge : tree.edges )
    {
        result[edge.child] = edge.parent;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
const RigGroupNetwork::Edge* findEdge( const RigGroupNetwork::TreeRevision& tree, const std::string& child )
{
    for ( const auto& edge : tree.edges )
    {
        if ( edge.child == child ) return &edge;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
RigGroupNetwork readDeckText( const QString& deckText )
{
    QTemporaryDir tempDir;
    EXPECT_TRUE( tempDir.isValid() );

    const QString fileName = tempDir.filePath( "NETWORK.DATA" );
    {
        QFile file( fileName );
        EXPECT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        QTextStream out( &file );
        out << deckText;
    }

    auto network = RifGroupNetworkReader::readFromFile( fileName );
    EXPECT_TRUE( network.has_value() ) << network.error().toStdString();
    return network.value_or( RigGroupNetwork() );
}

const QString networkDeck = R"(
RUNSPEC
DIMENS
 3 3 3 /
START
 1 'JAN' 2020 /
SCHEDULE
GRUPTREE
 'PLAT' 'FIELD' /
 'G1' 'PLAT' /
 'G2' 'PLAT' /
/
WELSPECS
 'P1' 'G1' 1 1 1* 'OIL' /
 'P2' 'G2' 2 2 1* 'OIL' /
 'I1' 'INJ' 3 3 1* 'WATER' /
/
GRUPNET
 'PLAT' 20.0 /
/
TSTEP
 31 /
GRUPTREE
 'G2' 'FIELD' /
/
DATES
 1 'MAR' 2020 /
/
BRANPROP
 'G1' 'PLAT' 3 1.5 /
 'PLAT' 'FIELD' 1 /
 'G2' 'PLAT' 2 /
/
NODEPROP
 'FIELD' 10.0 /
/
DATES
 1 'APR' 2020 /
/
BRANPROP
 'G2' 'PLAT' 9999 /
/
)";
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, standardNetworkPerDate )
{
    const auto network = readDeckText( networkDeck );

    const auto& trees = network.trees( RigGroupNetwork::TreeType::GRUPTREE );
    ASSERT_EQ( 2u, trees.size() );
    EXPECT_EQ( utcDate( 2020, 1, 1 ), trees[0].date );
    EXPECT_EQ( utcDate( 2020, 2, 1 ), trees[1].date );

    const std::map<std::string, std::string> expectedFirst = { { "FIELD", "" },
                                                               { "PLAT", "FIELD" },
                                                               { "INJ", "FIELD" },
                                                               { "G1", "PLAT" },
                                                               { "G2", "PLAT" },
                                                               { "P1", "G1" },
                                                               { "P2", "G2" },
                                                               { "I1", "INJ" } };
    EXPECT_EQ( expectedFirst, parents( trees[0] ) );

    // Parents are listed before their children
    EXPECT_EQ( "FIELD", trees[0].edges.front().child );

    // GRUPTREE re-parents G2 to FIELD
    EXPECT_EQ( "FIELD", parents( trees[1] ).at( "G2" ) );

    const auto* wellEdge = findEdge( trees[0], "P1" );
    ASSERT_NE( nullptr, wellEdge );
    EXPECT_TRUE( wellEdge->isWell() );

    const auto* platEdge = findEdge( trees[0], "PLAT" );
    ASSERT_NE( nullptr, platEdge );
    EXPECT_FALSE( platEdge->isWell() );
    EXPECT_DOUBLE_EQ( 20.0, platEdge->properties.at( "TERMINAL_PRESSURE" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, extendedNetwork )
{
    const auto network = readDeckText( networkDeck );

    const auto& trees = network.trees( RigGroupNetwork::TreeType::BRANPROP );
    ASSERT_EQ( 2u, trees.size() );
    EXPECT_EQ( utcDate( 2020, 3, 1 ), trees[0].date );
    EXPECT_EQ( utcDate( 2020, 4, 1 ), trees[1].date );

    // Only wells with a parent in the extended network are included
    const std::map<std::string, std::string> expectedFirst =
        { { "FIELD", "" }, { "PLAT", "FIELD" }, { "G1", "PLAT" }, { "G2", "PLAT" }, { "P1", "G1" }, { "P2", "G2" } };
    EXPECT_EQ( expectedFirst, parents( trees[0] ) );

    const auto* g1 = findEdge( trees[0], "G1" );
    ASSERT_NE( nullptr, g1 );
    EXPECT_EQ( RigGroupNetwork::EdgeKeyword::BRANPROP, g1->keyword );
    EXPECT_DOUBLE_EQ( 3.0, g1->properties.at( "VFP_TABLE" ) );
    EXPECT_DOUBLE_EQ( 1.5, g1->properties.at( "ALQ" ) );

    const auto* field = findEdge( trees[0], "FIELD" );
    ASSERT_NE( nullptr, field );
    EXPECT_DOUBLE_EQ( 10.0, field->properties.at( "TERMINAL_PRESSURE" ) );

    // VFP table 9999 removes the branch, and the wells below it
    const std::map<std::string, std::string> expectedSecond = { { "FIELD", "" }, { "PLAT", "FIELD" }, { "G1", "PLAT" }, { "P1", "G1" } };
    EXPECT_EQ( expectedSecond, parents( trees[1] ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, treeAtDate )
{
    const auto network = readDeckText( networkDeck );

    const auto* beforeFirst = network.treeAtDate( RigGroupNetwork::TreeType::GRUPTREE, utcDate( 2019, 1, 1 ) );
    ASSERT_NE( nullptr, beforeFirst );
    EXPECT_EQ( utcDate( 2020, 1, 1 ), beforeFirst->date );

    const auto* between = network.treeAtDate( RigGroupNetwork::TreeType::GRUPTREE, utcDate( 2020, 1, 15 ) );
    ASSERT_NE( nullptr, between );
    EXPECT_EQ( utcDate( 2020, 1, 1 ), between->date );

    const auto* after = network.treeAtDate( RigGroupNetwork::TreeType::GRUPTREE, utcDate( 2021, 1, 1 ) );
    ASSERT_NE( nullptr, after );
    EXPECT_EQ( utcDate( 2020, 2, 1 ), after->date );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, welspecsOnlyConnectsGroupsToField )
{
    const auto network = readDeckText( R"(
START
 1 'JAN' 2020 /
SCHEDULE
WELSPECS
 'P1' 'G1' 1 1 1* 'OIL' /
 'P2' 'FIELD' 2 2 1* 'OIL' /
/
)" );

    EXPECT_FALSE( network.hasTreeType( RigGroupNetwork::TreeType::BRANPROP ) );

    const auto& trees = network.trees( RigGroupNetwork::TreeType::GRUPTREE );
    ASSERT_EQ( 1u, trees.size() );

    const std::map<std::string, std::string> expected = { { "FIELD", "" }, { "G1", "FIELD" }, { "P1", "G1" }, { "P2", "FIELD" } };
    EXPECT_EQ( expected, parents( trees[0] ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, deckWithIncludes )
{
    const QString fileName = QString( "%1/RigSimulationInputTool/model5/0_BASE_MODEL5.DATA" ).arg( TEST_DATA_DIR );

    const auto network = RifGroupNetworkReader::readFromFile( fileName );
    ASSERT_TRUE( network.has_value() ) << network.error().toStdString();

    const auto& trees = network->trees( RigGroupNetwork::TreeType::GRUPTREE );
    ASSERT_FALSE( trees.empty() );
    EXPECT_EQ( utcDate( 2020, 1, 1 ), trees.front().date );

    const auto treeParents = parents( trees.front() );
    EXPECT_EQ( "", treeParents.at( "FIELD" ) );
    EXPECT_EQ( "FIELD", treeParents.at( "PROD" ) );
    EXPECT_EQ( "PLAT-A", treeParents.at( "M5S" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, datesAndTimeSteps )
{
    const auto network = readDeckText( R"(
GRUPTREE
 'G0' 'FIELD' /
/
START
 1 'JAN' 2020 /
SCHEDULE
GRUPTREE
 'G1' 'FIELD' /
/
TSTEP
 10 21 /
GRUPTREE
 'G2' 'FIELD' /
/
DATES
 1 'MAR' 2020 /
 1 'APR' 2020 /
/
GRUPTREE
 'G3' 'FIELD' /
/
)" );

    // A keyword with several dates moves to the last date. Keywords before the start date are ignored.
    const auto& trees = network.trees( RigGroupNetwork::TreeType::GRUPTREE );
    ASSERT_EQ( 3u, trees.size() );
    EXPECT_EQ( utcDate( 2020, 1, 1 ), trees[0].date );
    EXPECT_EQ( utcDate( 2020, 2, 1 ), trees[1].date );
    EXPECT_EQ( utcDate( 2020, 4, 1 ), trees[2].date );

    const std::map<std::string, std::string> expectedFirst = { { "FIELD", "" }, { "G1", "FIELD" } };
    EXPECT_EQ( expectedFirst, parents( trees[0] ) );
    EXPECT_TRUE( parents( trees[2] ).contains( "G3" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, actionBlocksAreIgnored )
{
    const auto network = readDeckText( R"(
START
 1 'JAN' 2020 /
SCHEDULE
GRUPTREE
 'G1' 'FIELD' /
/
ACTIONX
 'ACT1' 1 /
 FOPR > 1000 /
/
GRUPTREE
 'G1' 'PLAT' /
/
ENDACTIO
WELSPECS
 'P1' 'G1' 1 1 1* 'OIL' /
/
)" );

    const auto& trees = network.trees( RigGroupNetwork::TreeType::GRUPTREE );
    ASSERT_EQ( 1u, trees.size() );

    const std::map<std::string, std::string> expected = { { "FIELD", "" }, { "G1", "FIELD" }, { "P1", "G1" } };
    EXPECT_EQ( expected, parents( trees[0] ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, missingFile )
{
    EXPECT_FALSE( RifGroupNetworkReader::readFromFile( "does_not_exist.DATA" ).has_value() );
    EXPECT_FALSE( RifGroupNetworkReader::readFromFile( "" ).has_value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, sameTopology )
{
    const auto network = readDeckText( networkDeck );
    EXPECT_TRUE( network.hasSameTopology( readDeckText( networkDeck ) ) );

    // Other well group
    QString otherDeck = networkDeck;
    otherDeck.replace( "'P2' 'G2'", "'P2' 'G1'" );
    EXPECT_FALSE( network.hasSameTopology( readDeckText( otherDeck ) ) );

    // Other date of a change
    otherDeck = networkDeck;
    otherDeck.replace( "1 'APR' 2020", "2 'APR' 2020" );
    EXPECT_FALSE( network.hasSameTopology( readDeckText( otherDeck ) ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, sameTopologyIgnoresPropertyChanges )
{
    const QString deck = R"(
START
 1 'JAN' 2020 /
SCHEDULE
GRUPTREE
 'G1' 'FIELD' /
/
WELSPECS
 'P1' 'G1' 1 1 1* 'OIL' /
/
GRUPNET
 'G1' 20.0 /
/
)";

    // A pressure update at a later date does not change the connectivity
    const QString deckWithPressureUpdate = deck + R"(
TSTEP
 31 /
GRUPNET
 'G1' 25.0 /
/
)";

    EXPECT_TRUE( readDeckText( deck ).hasSameTopology( readDeckText( deckWithPressureUpdate ) ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RifGroupNetworkReader, sameTopologyDistinguishesWellsAndGroups )
{
    const QString wellDeck = R"(
START
 1 'JAN' 2020 /
SCHEDULE
GRUPTREE
 'G1' 'FIELD' /
/
WELSPECS
 'X' 'G1' 1 1 1* 'OIL' /
/
)";

    const QString groupDeck = R"(
START
 1 'JAN' 2020 /
SCHEDULE
GRUPTREE
 'G1' 'FIELD' /
 'X' 'G1' /
/
)";

    EXPECT_FALSE( readDeckText( wellDeck ).hasSameTopology( readDeckText( groupDeck ) ) );
}
