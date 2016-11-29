/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>
#include <iostream>
#include <boost/filesystem.hpp>

#define BOOST_TEST_MODULE ScheduleTests

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/GroupTree.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/CompletionSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/OilVaporizationProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

using namespace Opm;

static DeckPtr createDeck() {
    Opm::Parser parser;
    std::string input =
        "START\n"
        "8 MAR 1998 /\n"
        "\n"
        "SCHEDULE\n"
        "\n";

    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckWithWells() {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     \'W_1\'        \'OP\'   30   37  3.33       \'OIL\'  7* /   \n"
            "/ \n"
            "DATES             -- 1\n"
            " 10  \'JUN\'  2007 / \n"
            "/\n"
            "DATES             -- 2,3\n"
            "  10  JLY 2007 / \n"
            "   10  AUG 2007 / \n"
            "/\n"
            "WELSPECS\n"
            "     \'WX2\'        \'OP\'   30   37  3.33       \'OIL\'  7* /   \n"
            "     \'W_3\'        \'OP\'   20   51  3.92       \'OIL\'  7* /  \n"
            "/\n";

    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckForTestingCrossFlow() {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     \'DEFAULT\'    \'OP\'   30   37  3.33       \'OIL\'  7*/   \n"
            "     \'ALLOW\'      \'OP\'   30   37  3.33       \'OIL\'  3*  YES / \n"
            "     \'BAN\'        \'OP\'   20   51  3.92       \'OIL\'  3*  NO /  \n"
            "/\n"

            "COMPDAT\n"
            " \'BAN\'  1  1   1   1 \'OPEN\' 1*    1.168   0.311   107.872 1*  1*  \'Z\'  21.925 / \n"
            "/\n"

            "WCONHIST\n"
            "     'BAN'      'OPEN'      'RESV'      0.000      0.000      0.000  5* / \n"
            "/\n"

            "DATES             -- 1\n"
            " 10  JUN 2007 / \n"
            "/\n"

            "WCONHIST\n"
            "     'BAN'      'OPEN'      'RESV'      1.000      0.000      0.000  5* / \n"
            "/\n"

            "DATES             -- 2\n"
            " 10  JUL 2007 / \n"
            "/\n"

            "WCONPROD\n"
            "     'BAN'      'OPEN'      'ORAT'      0.000      0.000      0.000  5* / \n"
            "/\n"


            "DATES             -- 3\n"
            " 10  AUG 2007 / \n"
            "/\n"

            "WCONINJH\n"
            "     'BAN'      'WATER'      1*      0 / \n"
            "/\n"

            "DATES             -- 4\n"
            " 10  SEP 2007 / \n"
            "/\n"

            "WELOPEN\n"
            " 'BAN' OPEN / \n"
            "/\n"

            "DATES             -- 4\n"
            " 10  NOV 2007 / \n"
            "/\n"

            "WCONINJH\n"
            "     'BAN'      'WATER'      1*      1.0 / \n"
            "/\n";


    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckWithWellsOrdered() {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     \'CW_1\'        \'OP\'   30   37  3.33       \'OIL\'  7* /   \n"
            "     \'BW_2\'        \'OP\'   30   37  3.33       \'OIL\'  7* /   \n"
            "     \'AW_3\'        \'OP\'   20   51  3.92       \'OIL\'  7* /  \n"
            "/\n";

    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckWithWellsAndCompletionData() {
    Opm::Parser parser;
    std::string input =
      "START             -- 0 \n"
      "1 NOV 1979 / \n"
      "SCHEDULE\n"
      "DATES             -- 1\n"
      " 1 DES 1979/ \n"
      "/\n"
      "WELSPECS\n"
      "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
      "    'OP_2'       'OP'   8   8 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
      "    'OP_3'       'OP'   7   7 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
      "/\n"
      "COMPDAT\n"
      " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
      " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
      " 'OP_2'  8  8   1   3 'OPEN' 1*    1.168   0.311   107.872 1*  1*  'Y'  21.925 / \n"
      " 'OP_2'  8  7   3   3 'OPEN' 1*   15.071   0.311  1391.859 1*  1*  'Y'  21.920 / \n"
      " 'OP_2'  8  7   3   6 'OPEN' 1*    6.242   0.311   576.458 1*  1*  'Y'  21.915 / \n"
      " 'OP_3'  7  7   1   1 'OPEN' 1*   27.412   0.311  2445.337 1*  1*  'Y'  18.521 / \n"
      " 'OP_3'  7  7   2   2 'OPEN' 1*   55.195   0.311  4923.842 1*  1*  'Y'  18.524 / \n"
      "/\n"
      "DATES             -- 2,3\n"
      " 10  JUL 2007 / \n"
      " 10  AUG 2007 / \n"
      "/\n"
      "COMPDAT\n" // with defaulted I and J
      " 'OP_1'  0  *   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
      "/\n";

    return parser.parseString(input, ParseContext());
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckMissingReturnsDefaults) {
    DeckPtr deck(new Deck());
    deck->addKeyword( DeckKeyword( "SCHEDULE" ) );
    EclipseGrid grid(10,10,10);
    Schedule schedule(ParseContext() , grid , deck );
    BOOST_CHECK_EQUAL( schedule.getStartTime() , boost::posix_time::ptime(boost::gregorian::date( 1983  , boost::gregorian::Jan , 1)));
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWellsOrdered) {
    DeckPtr deck = createDeckWithWellsOrdered();
    EclipseGrid grid(100,100,100);
    Schedule schedule(ParseContext() , grid , deck );
    auto wells = schedule.getWells();

    BOOST_CHECK_EQUAL( "CW_1" , wells[0]->name());
    BOOST_CHECK_EQUAL( "BW_2" , wells[1]->name());
    BOOST_CHECK_EQUAL( "AW_3" , wells[2]->name());
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithStart) {
    DeckPtr deck = createDeck();
    EclipseGrid grid(10,10,10);
    Schedule schedule(ParseContext() , grid , deck );
    BOOST_CHECK_EQUAL( schedule.getStartTime() , boost::posix_time::ptime(boost::gregorian::date( 1998  , boost::gregorian::Mar , 8)));
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithSCHEDULENoThrow) {
    DeckPtr deck(new Deck());
    EclipseGrid grid(10,10,10);
    deck->addKeyword( DeckKeyword( "SCHEDULE" ) );

    BOOST_CHECK_NO_THROW(Schedule schedule(ParseContext() , grid , deck ));
}

BOOST_AUTO_TEST_CASE(EmptyScheduleHasNoWells) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeck();
    Schedule schedule(ParseContext() , grid , deck );
    BOOST_CHECK_EQUAL( 0U , schedule.numWells() );
    BOOST_CHECK_EQUAL( false , schedule.hasWell("WELL1") );
    BOOST_CHECK_THROW( schedule.getWell("WELL2") , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(CreateSchedule_DeckWithoutGRUPTREE_HasRootGroupTreeNodeForTimeStepZero) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeck();
    Schedule schedule(ParseContext() , grid , deck );
    BOOST_CHECK_EQUAL("FIELD", schedule.getGroupTree(0).getNode("FIELD")->name());
}

static std::shared_ptr< Deck > deckWithGRUPTREE() {
    DeckPtr deck = createDeck();
    DeckKeyword gruptreeKeyword("GRUPTREE");

    DeckRecord recordChildOfField;
    auto itemChild1 = DeckItem::make< std::string >( "CHILD_GROUP" );
    itemChild1.push_back(std::string("BARNET"));
    auto itemParent1 = DeckItem::make< std::string >( "PARENT_GROUP" );
    itemParent1.push_back(std::string("FAREN"));

    recordChildOfField.addItem( std::move( itemChild1 ) );
    recordChildOfField.addItem( std::move( itemParent1 ) );
    gruptreeKeyword.addRecord( std::move( recordChildOfField ) );
    deck->addKeyword( std::move( gruptreeKeyword ) );

    return deck;
}

BOOST_AUTO_TEST_CASE(CreateSchedule_DeckWithGRUPTREE_HasRootGroupTreeNodeForTimeStepZero) {
    EclipseGrid grid(10,10,10);
    auto deck = deckWithGRUPTREE();
    Schedule schedule(ParseContext() , grid , deck );
    GroupTreeNodePtr fieldNode = schedule.getGroupTree(0).getNode("FIELD");
    BOOST_CHECK_EQUAL("FIELD", fieldNode->name());
    GroupTreeNodePtr FAREN = fieldNode->getChildGroup("FAREN");
    BOOST_CHECK(FAREN->hasChildGroup("BARNET"));
}

BOOST_AUTO_TEST_CASE(GetGroups) {
    auto deck = deckWithGRUPTREE();
    EclipseGrid grid(10,10,10);
    Schedule schedule(ParseContext() , grid , deck );

    auto groups = schedule.getGroups();

    BOOST_CHECK_EQUAL( 3, groups.size() );

    std::vector< std::string > names;
    for( const auto group : groups ) names.push_back( group->name() );
    std::sort( names.begin(), names.end() );

    BOOST_CHECK_EQUAL( "BARNET", names[ 0 ] );
    BOOST_CHECK_EQUAL( "FAREN",  names[ 1 ] );
    BOOST_CHECK_EQUAL( "FIELD",  names[ 2 ] );
}

BOOST_AUTO_TEST_CASE(EmptyScheduleHasFIELDGroup) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeck();
    Schedule schedule(ParseContext() , grid , deck );
    BOOST_CHECK_EQUAL( 1U , schedule.numGroups() );
    BOOST_CHECK_EQUAL( true , schedule.hasGroup("FIELD") );
    BOOST_CHECK_EQUAL( false , schedule.hasGroup("GROUP") );
    BOOST_CHECK_THROW( schedule.getGroup("GROUP") , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(WellsIterator_Empty_EmptyVectorReturned) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeck();
    Schedule schedule(ParseContext() , grid , deck );
    size_t timeStep = 0;
    const auto wells_alltimesteps = schedule.getWells();
    BOOST_CHECK_EQUAL(0U, wells_alltimesteps.size());
    const auto wells_t0 = schedule.getWells(timeStep);
    BOOST_CHECK_EQUAL(0U, wells_t0.size());

    BOOST_CHECK_THROW(schedule.getWells(1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(WellsIterator_HasWells_WellsReturned) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeckWithWells();
    Schedule schedule(ParseContext() , grid , deck );
    size_t timeStep = 0;

    const auto wells_alltimesteps = schedule.getWells();
    BOOST_CHECK_EQUAL(3U, wells_alltimesteps.size());
    const auto wells_t0 = schedule.getWells(timeStep);
    BOOST_CHECK_EQUAL(1U, wells_t0.size());
    const auto wells_t3 = schedule.getWells(3);
    BOOST_CHECK_EQUAL(3U, wells_t3.size());
}

BOOST_AUTO_TEST_CASE(WellsIteratorWithRegex_HasWells_WellsReturned) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeckWithWells();
    Schedule schedule(ParseContext() , grid , deck );
    std::string wellNamePattern;

    wellNamePattern = "*";
    auto wells = schedule.getWellsMatching(wellNamePattern);
    BOOST_CHECK_EQUAL(3U, wells.size());

    wellNamePattern = "W_*";
    wells = schedule.getWellsMatching(wellNamePattern);
    BOOST_CHECK_EQUAL(2U, wells.size());

    wellNamePattern = "W_3";
    wells = schedule.getWellsMatching(wellNamePattern);
    BOOST_CHECK_EQUAL(1U, wells.size());
}

BOOST_AUTO_TEST_CASE(ReturnNumWellsTimestep) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeckWithWells();
    Schedule schedule(ParseContext() , grid , deck );

    BOOST_CHECK_EQUAL(schedule.numWells(0), 1);
    BOOST_CHECK_EQUAL(schedule.numWells(1), 1);
    BOOST_CHECK_EQUAL(schedule.numWells(2), 1);
    BOOST_CHECK_EQUAL(schedule.numWells(3), 3);
}

BOOST_AUTO_TEST_CASE(ReturnMaxNumCompletionsForWellsInTimestep) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeckWithWellsAndCompletionData();
    Schedule schedule(ParseContext() , grid , deck );

    BOOST_CHECK_EQUAL(schedule.getMaxNumCompletionsForWells(1), 7);
    BOOST_CHECK_EQUAL(schedule.getMaxNumCompletionsForWells(3), 9);
}

BOOST_AUTO_TEST_CASE(TestCrossFlowHandling) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeckForTestingCrossFlow();
    Schedule schedule(ParseContext() , grid , deck );

    auto well_ban = schedule.getWell("BAN");
    BOOST_CHECK_EQUAL(well_ban->getAllowCrossFlow(), false);


    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well_ban->getStatus(0));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, well_ban->getStatus(1));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, well_ban->getStatus(2));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well_ban->getStatus(3));
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well_ban->getStatus(4)); // not allow to open
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, well_ban->getStatus(5));


    {
        auto well_allow = schedule.getWell("ALLOW");
        auto well_default = schedule.getWell("DEFAULT");

        BOOST_CHECK_EQUAL(well_default->getAllowCrossFlow(), true);
        BOOST_CHECK_EQUAL(well_allow->getAllowCrossFlow(), true);
    }
}

static DeckPtr createDeckWithWellsAndCompletionDataWithWELOPEN() {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "1 NOV 1979 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 1 DES 1979/ \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "    'OP_2'       'OP'   8   8 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "    'OP_3'       'OP'   7   7 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_2'  8  8   1   3 'OPEN' 1*    1.168   0.311   107.872 1*  1*  'Y'  21.925 / \n"
                    " 'OP_2'  8  7   3   3 'OPEN' 1*   15.071   0.311  1391.859 1*  1*  'Y'  21.920 / \n"
                    " 'OP_2'  8  7   3   6 'OPEN' 1*    6.242   0.311   576.458 1*  1*  'Y'  21.915 / \n"
                    " 'OP_3'  7  7   1   1 'OPEN' 1*   27.412   0.311  2445.337 1*  1*  'Y'  18.521 / \n"
                    " 'OP_3'  7  7   2   2 'OPEN' 1*   55.195   0.311  4923.842 1*  1*  'Y'  18.524 / \n"
                    "/\n"
                    "DATES             -- 2,3\n"
                    " 10  JUL 2007 / \n"
                    " 10  AUG 2007 / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' SHUT / \n"
                    " '*'    OPEN 0 0 3 / \n"
                    " 'OP_2' SHUT 0 0 0 4 6 / \n "
                    " 'OP_3' SHUT 0 0 0 / \n"
                    "/\n"
                    "DATES             -- 4\n"
                    " 10  JUL 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN / \n"
                    " 'OP_2' OPEN 0 0 0 4 6 / \n "
                    " 'OP_3' OPEN 0 0 0 / \n"
                    "/\n"
                    "DATES             -- 5\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' SHUT 0 0 0 0 0 / \n "
                    "/\n";

    return parser.parseString(input, ParseContext());
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWellsAndCompletionDataWithWELOPEN) {
    EclipseGrid grid(10,10,10);
    DeckPtr deck = createDeckWithWellsAndCompletionDataWithWELOPEN();
    Schedule schedule(ParseContext() , grid , deck );
    auto* well = schedule.getWell("OP_1");
    size_t currentStep = 0;
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well->getStatus(currentStep));
    currentStep = 3;
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well->getStatus(currentStep));

    well = schedule.getWell("OP_2");
    CompletionSetConstPtr completionSet = well->getCompletions(currentStep);

    size_t index = 3;
    CompletionConstPtr completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::SHUT, completion->getState());
    index = 4;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::SHUT, completion->getState());
    index = 5;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::SHUT, completion->getState());
    index = 6;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::OPEN, completion->getState());

    currentStep = 4;
    completionSet = well->getCompletions(currentStep);
    index = 3;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::OPEN, completion->getState());
    index = 4;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::OPEN, completion->getState());
    index = 5;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::OPEN, completion->getState());
    index = 6;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::OPEN, completion->getState());

    well = schedule.getWell("OP_3");
    currentStep = 3;
    completionSet = well->getCompletions(currentStep);

    index = 0;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::SHUT, completion->getState());

    currentStep = 4;
    completionSet = well->getCompletions(currentStep);

    index = 0;
    completion = completionSet->get(index);
    BOOST_CHECK_EQUAL(WellCompletion::StateEnum::OPEN, completion->getState());

    well = schedule.getWell("OP_1");

    currentStep = 3;
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well->getStatus(currentStep));

    currentStep = 4;
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, well->getStatus(currentStep));

    currentStep = 5;
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well->getStatus(currentStep));
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithWELOPEN_TryToOpenWellWithShutCompletionsDoNotOpenWell) {
  Opm::Parser parser;
  std::string input =
          "START             -- 0 \n"
                  "1 NOV 1979 / \n"
                  "SCHEDULE\n"
                  "DATES             -- 1\n"
                  " 1 DES 1979/ \n"
                  "/\n"
                  "WELSPECS\n"
                  "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                  "/\n"
                  "COMPDAT\n"
                  " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                  " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                  " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                  "/\n"
                  "DATES             -- 2\n"
                  " 10  JUL 2008 / \n"
                  "/\n"
                  "WELOPEN\n"
                  " 'OP_1' OPEN / \n"
                  "/\n"
                  "DATES             -- 3\n"
                  " 10  OKT 2008 / \n"
                  "/\n"
                  "WELOPEN\n"
                  " 'OP_1' SHUT 0 0 0 0 0 / \n "
                  "/\n"
                  "DATES             -- 4\n"
                  " 10  NOV 2008 / \n"
                  "/\n"
                  "WELOPEN\n"
                  " 'OP_1' OPEN / \n "
                  "/\n";

  EclipseGrid grid(10,10,10);
  ParseContext parseContext;
  DeckPtr deck = parser.parseString(input, parseContext);
  Schedule schedule(parseContext , grid , deck );
  auto* well = schedule.getWell("OP_1");
  size_t currentStep = 3;
  BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well->getStatus(currentStep));
  currentStep = 4;
  BOOST_CHECK_EQUAL(WellCommon::StatusEnum::SHUT, well->getStatus(currentStep));
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithCOMPLUMPwithC1_ThrowsExcpetion) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "1 NOV 1979 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 1 DES 1979/ \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "DATES             -- 3\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN 0 0 0 1 0 / \n"
                    "/\n"
                    "COMPLUMP\n"
                    " 'OP_1' 0 0 0 0 0 / \n "
                    "/\n"
                    "DATES             -- 4\n"
                    " 10  NOV 2008 / \n"
                    "/\n";


    DeckPtr deck = parser.parseString(input, ParseContext());
    EclipseGrid grid(10,10,10);
    BOOST_CHECK_THROW(Schedule schedule(ParseContext() , grid , deck ), std::exception);
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithCOMPLUMPwithC1andC2_ThrowsExcpetion) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "1 NOV 1979 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 1 DES 1979/ \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "DATES             -- 3\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN 0 0 0 1 4 / \n"
                    "/\n"
                    "COMPLUMP\n"
                    " 'OP_1' 0 0 0 0 0 / \n "
                    "/\n"
                    "DATES             -- 4\n"
                    " 10  NOV 2008 / \n"
                    "/\n";


    DeckPtr deck = parser.parseString(input, ParseContext());
    EclipseGrid grid(10,10,10);
    BOOST_CHECK_THROW(Schedule schedule(ParseContext() , grid , deck ), std::exception);
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithCOMPLUMPwithC2_ThrowsExcpetion) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "1 NOV 1979 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 1 DES 1979/ \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "DATES             -- 3\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN 0 0 0 0 4 / \n"
                    "/\n"
                    "COMPLUMP\n"
                    " 'OP_1' 0 0 0 0 0 / \n "
                    "/\n"
                    "DATES             -- 4\n"
                    " 10  NOV 2008 / \n"
                    "/\n";


    DeckPtr deck = parser.parseString(input, ParseContext());
    EclipseGrid grid(10,10,10);
    BOOST_CHECK_THROW(Schedule schedule(ParseContext() , grid , deck ), std::exception);
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithCOMPLUMPwithDefaultValuesInWELOPEN) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "1 NOV 1979 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 1 DES 1979/ \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "DATES             -- 3\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN/ \n"
                    "/\n"
                    "COMPLUMP\n"
                    " 'OP_1' 0 0 0 0 0 / \n "
                    "/\n"
                    "DATES             -- 4\n"
                    " 10  NOV 2008 / \n"
                    "/\n";

    EclipseGrid grid(10,10,10);
    DeckPtr deck = parser.parseString(input, ParseContext());
    Schedule schedule(ParseContext() , grid , deck );
    auto* well = schedule.getWell("OP_1");
    size_t currentStep = 3;
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, well->getStatus(currentStep));
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithWRFT) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "1 NOV 1979 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 1 DES 1979/ \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "    'OP_2'       'OP'   4   4 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_2'  4  4   4  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "DATES             -- 2\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WRFT \n"
                    "/ \n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN / \n"
                    "/\n"
                    "DATES             -- 3\n"
                    " 10  NOV 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_2' OPEN / \n"
                    "/\n"
                    "DATES             -- 4\n"
                    " 30  NOV 2008 / \n"
                    "/\n";


    EclipseGrid grid(10,10,10);
    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    Schedule schedule(parseContext , grid , deck );

    {
        auto* well = schedule.getWell("OP_1");
        BOOST_CHECK_EQUAL(well->getRFTActive(2),true);
        BOOST_CHECK_EQUAL(2 , well->firstRFTOutput( ));
    }

    {
        auto* well = schedule.getWell("OP_2");
        BOOST_CHECK_EQUAL(well->getRFTActive(3),true);
        BOOST_CHECK_EQUAL(3 , well->firstRFTOutput( ));
    }
}

BOOST_AUTO_TEST_CASE(CreateScheduleDeckWithWRFTPLT) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "1 NOV 1979 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 1 DES 1979/ \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' SHUT / \n"
                    "/\n"
                    "DATES             -- 2\n"
                    " 10  OKT 2006 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' SHUT / \n"
                    "/\n"
                    "WRFTPLT \n"
                    " 'OP_1' FOPN / \n"
                    "/ \n"
                    "DATES             -- 3\n"
                    " 10  OKT 2007 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN 0 0 0 0 0 / \n"
                    "/\n"
                    "DATES             -- 4\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WELOPEN\n"
                    " 'OP_1' OPEN / \n"
                    "/\n"
                    "COMPLUMP\n"
                    " 'OP_1' 0 0 0 0 0 / \n "
                    "/\n"
                    "DATES             -- 5\n"
                    " 10  NOV 2008 / \n"
                    "/\n";
    ParseContext parseContext;
    EclipseGrid grid(10,10,10);
    DeckPtr deck = parser.parseString(input, parseContext);
    Schedule schedule(parseContext , grid , deck );
    auto* well = schedule.getWell("OP_1");

    size_t currentStep = 3;
    BOOST_CHECK_EQUAL(well->getRFTActive(currentStep),false);
    currentStep = 4;
    BOOST_CHECK_EQUAL(well->getRFTActive(currentStep),true);
    BOOST_CHECK_EQUAL(WellCommon::StatusEnum::OPEN, well->getStatus(currentStep));
    currentStep = 5;
    BOOST_CHECK_EQUAL(well->getRFTActive(currentStep),false);
}

BOOST_AUTO_TEST_CASE(createDeckWithWeltArg) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n"
            "WELSPECS\n"
            "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "/\n"
            "COMPDAT\n"
            " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
            " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            "/\n"
            "DATES             -- 2\n"
            " 20  JAN 2010 / \n"
            "/\n"
            "WELTARG\n"
            " OP_1     ORAT        1300 /\n"
            " OP_1     WRAT        1400 /\n"
            " OP_1     GRAT        1500.52 /\n"
            " OP_1     LRAT        1600.58 /\n"
            " OP_1     RESV        1801.05 /\n"
            " OP_1     BHP         1900 /\n"
            " OP_1     THP         2000 /\n"
            " OP_1     VFP         2100.09 /\n"
            " OP_1     GUID        2300.14 /\n"
            "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid , deck );
    auto* well = schedule.getWell("OP_1");

    size_t currentStep = 1;
    WellProductionProperties wpp = well->getProductionProperties(currentStep);
    BOOST_CHECK_EQUAL(wpp.WaterRate,0);

    Opm::UnitSystem unitSystem = deck->getActiveUnitSystem();
    double siFactorL = unitSystem.parse("LiquidSurfaceVolume/Time")->getSIScaling();
    double siFactorG = unitSystem.parse("GasSurfaceVolume/Time")->getSIScaling();
    double siFactorP = unitSystem.parse("Pressure")->getSIScaling();

    currentStep = 2;
    wpp = well->getProductionProperties(currentStep);
    BOOST_CHECK_EQUAL(wpp.OilRate, 1300 * siFactorL);
    BOOST_CHECK_EQUAL(wpp.WaterRate, 1400 * siFactorL);
    BOOST_CHECK_EQUAL(wpp.GasRate, 1500.52 * siFactorG);
    BOOST_CHECK_EQUAL(wpp.LiquidRate, 1600.58 * siFactorL);
    BOOST_CHECK_EQUAL(wpp.ResVRate, 1801.05 * siFactorL);
    BOOST_CHECK_EQUAL(wpp.BHPLimit, 1900 * siFactorP);
    BOOST_CHECK_EQUAL(wpp.THPLimit, 2000 * siFactorP);
    BOOST_CHECK_EQUAL(wpp.VFPTableNumber, 2100);
    BOOST_CHECK_EQUAL(well->getGuideRate(2), 2300.14);
}

BOOST_AUTO_TEST_CASE(createDeckWithWeltArgException) {
    Opm::Parser parser;
    std::string input =
            "SCHEDULE\n"
            "WELTARG\n"
            " OP_1     GRAT        1500.52 /\n"
            " OP_1     LRAT        /\n"
            " OP_1     RESV        1801.05 /\n"
            "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);

    BOOST_CHECK_THROW(Schedule (parseContext , grid , deck ), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(createDeckWithWeltArgException2) {
    Opm::Parser parser;
    std::string input =
            "SCHEDULE\n"
            "WELTARG\n"
            " OP_1     LRAT        /\n"
            " OP_1     RESV        1801.05 /\n"
            "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);

    BOOST_CHECK_THROW(Schedule (parseContext , grid , deck ), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(createDeckWithWPIMULT) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
                    "19 JUN 2007 / \n"
                    "SCHEDULE\n"
                    "DATES             -- 1\n"
                    " 10  OKT 2008 / \n"
                    "/\n"
                    "WELSPECS\n"
                    "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n"
                    "DATES             -- 2\n"
                    " 20  JAN 2010 / \n"
                    "/\n"
                    "WELTARG\n"
                    " OP_1     ORAT        1300 /\n"
                    " OP_1     WRAT        1400 /\n"
                    " OP_1     GRAT        1500.52 /\n"
                    " OP_1     LRAT        1600.58 /\n"
                    " OP_1     RESV        1801.05 /\n"
                    " OP_1     BHP         1900 /\n"
                    " OP_1     THP         2000 /\n"
                    " OP_1     VFP         2100.09 /\n"
                    " OP_1     GUID        2300.14 /\n"
                    "/\n"
                    "WPIMULT\n"
                    "OP_1  1.30 /\n"
                    "/\n"
                    "DATES             -- 3\n"
                    " 20  JAN 2011 / \n"
                    "/\n"
                    "WPIMULT\n"
                    "OP_1  1.30 /\n"
                    "/\n"
                    "DATES             -- 4\n"
                    " 20  JAN 2012 / \n"
                    "/\n"
                    "COMPDAT\n"
                    " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                    " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                    "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid, deck );
    auto* well = schedule.getWell("OP_1");

    size_t currentStep = 2;
    CompletionSetConstPtr currentCompletionSet = well->getCompletions(currentStep);
    size_t completionSize = currentCompletionSet->size();

    for(size_t i = 0; i < completionSize;i++) {
        CompletionConstPtr currentCompletion = currentCompletionSet->get(i);
        BOOST_CHECK_EQUAL(currentCompletion->getWellPi(), 1.3);
    }

    currentStep = 3;
    currentCompletionSet = well->getCompletions(currentStep);
    completionSize = currentCompletionSet->size();

    for(size_t i = 0; i < completionSize;i++) {
        CompletionConstPtr currentCompletion = currentCompletionSet->get(i);
        BOOST_CHECK_EQUAL(currentCompletion->getWellPi(), (1.3*1.3));
    }

    currentStep = 4;
    currentCompletionSet = well->getCompletions(currentStep);
    completionSize = currentCompletionSet->size();

    for(size_t i = 0; i < completionSize;i++) {
        CompletionConstPtr currentCompletion = currentCompletionSet->get(i);
        BOOST_CHECK_EQUAL(currentCompletion->getWellPi(), 1.0);
    }

}

BOOST_AUTO_TEST_CASE(createDeckWithDRSDT) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n"
            "DRSDT\n"
            "0.0003\n"
            "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid, deck );
    size_t currentStep = 1;
    BOOST_CHECK_EQUAL(schedule.hasOilVaporizationProperties(), true);
    const auto& ovap = schedule.getOilVaporizationProperties(currentStep);

    BOOST_CHECK_EQUAL(true,   ovap.getOption());
    BOOST_CHECK_EQUAL(ovap.getType(), Opm::OilVaporizationEnum::DRSDT);
}


BOOST_AUTO_TEST_CASE(createDeckWithDRSDTthenDRVDT) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n"
            "DRSDT\n"
            "0.0003\n"
            "/\n"
            "DATES             -- 1\n"
            " 10  OKT 2009 / \n"
            "/\n"
            "DRVDT\n"
            "0.100\n"
            "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid, deck );
    size_t currentStep = 2;
    BOOST_CHECK_EQUAL(schedule.hasOilVaporizationProperties(), true);
    const OilVaporizationProperties& ovap = schedule.getOilVaporizationProperties(currentStep);
    double value =  ovap.getMaxDRVDT();
    BOOST_CHECK_EQUAL(1.1574074074074074e-06, value);
    BOOST_CHECK_EQUAL(ovap.getType(), Opm::OilVaporizationEnum::DRVDT);
}

BOOST_AUTO_TEST_CASE(createDeckWithVAPPARS) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n"
            "VAPPARS\n"
            "2 0.100\n"
            "/\n";

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid, deck );
    size_t currentStep = 1;
    BOOST_CHECK_EQUAL(schedule.hasOilVaporizationProperties(), true);
    const OilVaporizationProperties& ovap = schedule.getOilVaporizationProperties(currentStep);
    BOOST_CHECK_EQUAL(ovap.getType(), Opm::OilVaporizationEnum::VAPPARS);
    double vap1 =  ovap.getVap1();
    BOOST_CHECK_EQUAL(2, vap1);
    double vap2 =  ovap.getVap2();
    BOOST_CHECK_EQUAL(0.100, vap2);

}


BOOST_AUTO_TEST_CASE(createDeckWithOutOilVaporizationProperties) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n";


    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid, deck );

    BOOST_CHECK_EQUAL(schedule.hasOilVaporizationProperties(), false);


}

BOOST_AUTO_TEST_CASE(changeBhpLimitInHistoryModeWithWeltarg) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n"
            "WELSPECS\n"
            "    'P'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "    'I'       'OP'   1   1 1*     'WATER' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "/\n"
            "COMPDAT\n"
            " 'P'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            " 'P'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
            " 'I'  1  1   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            "/\n"
            "WCONHIST\n"
            " 'P' 'OPEN' 'RESV' 6*  500 / \n"
            "/\n"
            "WCONINJH\n"
            " 'I' 'WATER' 1* 100 250 / \n"
            "/\n"
            "WELTARG\n"
            "   'P' 'BHP' 50 / \n"
            "   'I' 'BHP' 600 / \n"
            "/\n"
            "DATES             -- 2\n"
            " 15  OKT 2008 / \n"
            "/\n"
            "WCONHIST\n"
            "   'P' 'OPEN' 'RESV' 6*  500/\n/\n"
            "WCONINJH\n"
            " 'I' 'WATER' 1* 100 250 / \n"
            "/\n"
            "DATES             -- 3\n"
            " 18  OKT 2008 / \n"
            "/\n"
            "WCONHIST\n"
            "   'I' 'OPEN' 'RESV' 6*  /\n/\n"
            "DATES             -- 3\n"
            " 20  OKT 2008 / \n"
            "/\n"
            "WCONINJH\n"
            " 'I' 'WATER' 1* 100 250 / \n"
            "/\n"
            ;

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid, deck );
    auto* well_p = schedule.getWell("P");

    BOOST_CHECK_EQUAL(well_p->getProductionProperties(0).BHPLimit, 0); //start
    BOOST_CHECK_EQUAL(well_p->getProductionProperties(1).BHPLimit, 50 * 1e5); // 1
    // The BHP limit should not be effected by WCONHIST
    BOOST_CHECK_EQUAL(well_p->getProductionProperties(2).BHPLimit, 50 * 1e5); // 2

    auto* well_i = schedule.getWell("I");

    BOOST_CHECK_EQUAL(well_i->getInjectionProperties(0).BHPLimit, 0); //start
    BOOST_CHECK_EQUAL(well_i->getInjectionProperties(1).BHPLimit, 600 * 1e5); // 1
    BOOST_CHECK_EQUAL(well_i->getInjectionProperties(2).BHPLimit, 600 * 1e5); // 2

    // Check that the BHP limit is reset when changing between injector and producer.
    BOOST_CHECK_EQUAL(well_i->getInjectionProperties(3).BHPLimit, 0); // 3
    BOOST_CHECK_EQUAL(well_i->getInjectionProperties(4).BHPLimit, 0); // 4

    BOOST_CHECK_EQUAL( true  , well_i->getInjectionProperties(2).hasInjectionControl(Opm::WellInjector::BHP) );
    BOOST_CHECK_EQUAL( false , well_i->getInjectionProperties(3).hasInjectionControl(Opm::WellInjector::BHP) );
    BOOST_CHECK_EQUAL( false , well_i->getInjectionProperties(4).hasInjectionControl(Opm::WellInjector::BHP) );
}

BOOST_AUTO_TEST_CASE(changeModeWithWHISTCTL) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n"
            "WELSPECS\n"
            "    'P1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "    'P2'       'OP'   5   5 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "    'I'       'OP'   1   1 1*     'WATER' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "/\n"
            "COMPDAT\n"
            " 'P1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            " 'P1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
            " 'P2'  5  5   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            " 'P2'  5  5   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
            " 'I'  1  1   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            "/\n"
            "WCONHIST\n"
            " 'P1' 'OPEN' 'ORAT' 5*/ \n"
            " 'P2' 'OPEN' 'ORAT' 5*/ \n"
            "/\n"
            "DATES             -- 2\n"
            " 15  OKT 2008 / \n"
            "/\n"
            "WHISTCTL\n"
            " RESV / \n"
            "WCONHIST\n"
            " 'P1' 'OPEN' 'ORAT' 5*/ \n"
            " 'P2' 'OPEN' 'ORAT' 5*/ \n"
            "/\n"
            "DATES             -- 3\n"
            " 18  OKT 2008 / \n"
            "/\n"
            "WCONHIST\n"
            " 'P1' 'OPEN' 'ORAT' 5*/ \n"
            " 'P2' 'OPEN' 'ORAT' 5*/ \n"
            "/\n"
            "DATES             -- 4\n"
            " 20  OKT 2008 / \n"
            "/\n"
            "WHISTCTL\n"
            " LRAT / \n"
            "WCONHIST\n"
            " 'P1' 'OPEN' 'ORAT' 5*/ \n"
            " 'P2' 'OPEN' 'ORAT' 5*/ \n"
            "/\n"
            "DATES             -- 5\n"
            " 25  OKT 2008 / \n"
            "/\n"
            "WHISTCTL\n"
            " NONE / \n"
            "WCONHIST\n"
            " 'P1' 'OPEN' 'ORAT' 5*/ \n"
            " 'P2' 'OPEN' 'ORAT' 5*/ \n"
            "/\n"
            ;

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    Schedule schedule(parseContext , grid, deck );
    auto* well_p1 = schedule.getWell("P1");
    auto* well_p2 = schedule.getWell("P2");

    //Start
    BOOST_CHECK_EQUAL(well_p1->getProductionProperties(0).controlMode, Opm::WellProducer::CMODE_UNDEFINED);
    BOOST_CHECK_EQUAL(well_p2->getProductionProperties(0).controlMode, Opm::WellProducer::CMODE_UNDEFINED);

    //10  OKT 2008
    BOOST_CHECK_EQUAL(well_p1->getProductionProperties(1).controlMode, Opm::WellProducer::ORAT);
    BOOST_CHECK_EQUAL(well_p2->getProductionProperties(1).controlMode, Opm::WellProducer::ORAT);

    //15  OKT 2008
    BOOST_CHECK_EQUAL(well_p1->getProductionProperties(2).controlMode, Opm::WellProducer::RESV);
    BOOST_CHECK_EQUAL(well_p2->getProductionProperties(2).controlMode, Opm::WellProducer::RESV);

    //18  OKT 2008
    BOOST_CHECK_EQUAL(well_p1->getProductionProperties(3).controlMode, Opm::WellProducer::RESV);
    BOOST_CHECK_EQUAL(well_p2->getProductionProperties(3).controlMode, Opm::WellProducer::RESV);

    // 20 OKT 2008
    BOOST_CHECK_EQUAL(well_p1->getProductionProperties(4).controlMode, Opm::WellProducer::LRAT);
    BOOST_CHECK_EQUAL(well_p2->getProductionProperties(4).controlMode, Opm::WellProducer::LRAT);

    // 25 OKT 2008
    BOOST_CHECK_EQUAL(well_p1->getProductionProperties(5).controlMode, Opm::WellProducer::ORAT);
    BOOST_CHECK_EQUAL(well_p2->getProductionProperties(5).controlMode, Opm::WellProducer::ORAT);
}

BOOST_AUTO_TEST_CASE(unsupportedOptionWHISTCTL) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "19 JUN 2007 / \n"
            "SCHEDULE\n"
            "DATES             -- 1\n"
            " 10  OKT 2008 / \n"
            "/\n"
            "WELSPECS\n"
            "    'P1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "    'P2'       'OP'   5   5 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "    'I'       'OP'   1   1 1*     'WATER' 1*      1*  1*   1*  1*   1*  1*  / \n"
            "/\n"
            "COMPDAT\n"
            " 'P1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            " 'P1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
            " 'P2'  5  5   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            " 'P2'  5  5   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
            " 'I'  1  1   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
            "/\n"
            "WCONHIST\n"
            " 'P1' 'OPEN' 'ORAT' 5*/ \n"
            " 'P2' 'OPEN' 'ORAT' 5*/ \n"
            "/\n"
            "DATES             -- 2\n"
            " 15  OKT 2008 / \n"
            "/\n"
            "WHISTCTL\n"
            " * YES / \n"
            ;

    ParseContext parseContext;
    DeckPtr deck = parser.parseString(input, parseContext);
    EclipseGrid grid(10,10,10);
    BOOST_CHECK_THROW(Schedule schedule(parseContext , grid, deck ), std::invalid_argument);
}

