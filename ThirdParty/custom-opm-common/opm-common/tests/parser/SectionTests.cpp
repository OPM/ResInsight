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


#define BOOST_TEST_MODULE SectionTests

#include <stdexcept>
#include <iostream>
#include <typeinfo>

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(SectionTest) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword("START")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("RUNSPEC")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("WELLDIMS")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("PORO")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("SCHEDULE")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("WELSPECS")));

    DeckSection runspecSection(deck, "RUNSPEC");
    DeckSection gridSection(deck, "GRID");
    BOOST_CHECK(runspecSection.hasKeyword("WELLDIMS"));
    BOOST_CHECK(gridSection.hasKeyword("PORO"));

    BOOST_CHECK(!runspecSection.hasKeyword("START"));
    BOOST_CHECK(!gridSection.hasKeyword("START"));
    BOOST_CHECK(!runspecSection.hasKeyword("WELSPECS"));
    BOOST_CHECK(!gridSection.hasKeyword("WELSPECS"));
    BOOST_CHECK(!runspecSection.hasKeyword("PORO"));
    BOOST_CHECK(!gridSection.hasKeyword("WELLDIMS"));
}

BOOST_AUTO_TEST_CASE(IteratorTest) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword("RUNSPEC")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("WELLDIMS")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("TABDIMS")));
    deck.addKeyword( DeckKeyword(parser.getKeyword("GRID")));
    DeckSection section(deck, "RUNSPEC");

    int numberOfItems = 0;
    for (auto iter=section.begin(); iter != section.end(); ++iter) {
        std::cout << iter->name() << std::endl;
        numberOfItems++;
    }

    // the keywords expected here are RUNSPEC, WELLDIMS and TABDIMS
    BOOST_CHECK_EQUAL(3, numberOfItems);
}

BOOST_AUTO_TEST_CASE(RUNSPECSection_EmptyDeck) {
    Deck deck;
    BOOST_REQUIRE_NO_THROW(RUNSPECSection section(deck));
}

BOOST_AUTO_TEST_CASE(RUNSPECSection_ReadSimpleDeck) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "START")));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "RUNSPEC")));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "WELLDIMS")));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "TABDIMS")));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "GRID")));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "PORO")));

    RUNSPECSection section(deck);
    BOOST_CHECK(!section.hasKeyword("START"));
    BOOST_CHECK(section.hasKeyword("RUNSPEC"));
    BOOST_CHECK(section.hasKeyword("WELLDIMS"));
    BOOST_CHECK(section.hasKeyword("TABDIMS"));
    BOOST_CHECK(!section.hasKeyword("GRID"));
    BOOST_CHECK(!section.hasKeyword("PORO"));
}

BOOST_AUTO_TEST_CASE(RUNSPECSection_ReadSmallestPossibleDeck) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "RUNSPEC" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "GRID")));
    RUNSPECSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("RUNSPEC"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("GRID"));
}

BOOST_AUTO_TEST_CASE(GRIDSection_TerminatedByEDITKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "GRID" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "EDIT" )));
    GRIDSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("GRID"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("EDIT"));
}

BOOST_AUTO_TEST_CASE(GRIDSection_TerminatedByPROPSKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "GRID" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "PROPS" )));
    GRIDSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("GRID"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("PROPS"));
}

BOOST_AUTO_TEST_CASE(EDITSection_TerminatedByPROPSKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "EDIT" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "PROPS" )));
    EDITSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("EDIT"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("PROPS"));
}

BOOST_AUTO_TEST_CASE(PROPSSection_TerminatedByREGIONSKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "PROPS" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "REGIONS" )));
    PROPSSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("PROPS"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("REGIONS"));
}

BOOST_AUTO_TEST_CASE(PROPSSection_TerminatedBySOLUTIONKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "PROPS" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "SOLUTION" )));

    PROPSSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("PROPS"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("SOLUTION"));
}

BOOST_AUTO_TEST_CASE(REGIONSSection_TerminatedBySOLUTIONKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "REGIONS" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "SOLUTION" )));

    REGIONSSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("REGIONS"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("SOLUTION"));
}

BOOST_AUTO_TEST_CASE(SOLUTIONSection_TerminatedBySUMMARYKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "SOLUTION" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "SUMMARY" )));

    SOLUTIONSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("SOLUTION"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("SUMMARY"));
}

BOOST_AUTO_TEST_CASE(SOLUTIONSection_TerminatedBySCHEDULEKeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "SOLUTION" )));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "SCHEDULE" )));

    SOLUTIONSection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("SOLUTION"));
    BOOST_CHECK_EQUAL(false, section.hasKeyword("SCHEDULE"));
}

BOOST_AUTO_TEST_CASE(SCHEDULESection_NotTerminated) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword(parser.getKeyword( "SCHEDULE")));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "WELSPECS" ) ));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "COMPDAT" ) ));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "WCONHIST" ) ));
    deck.addKeyword( DeckKeyword(parser.getKeyword( "WCONPROD" ) ));

    SCHEDULESection section(deck);
    BOOST_CHECK_EQUAL(true, section.hasKeyword("SCHEDULE"));
    BOOST_CHECK_EQUAL(true, section.hasKeyword("WELSPECS"));
    BOOST_CHECK_EQUAL(true, section.hasKeyword("COMPDAT"));
    BOOST_CHECK_EQUAL(true, section.hasKeyword("WCONHIST"));

    BOOST_CHECK( DeckSection::hasSCHEDULE(deck ));
    BOOST_CHECK( !DeckSection::hasREGIONS(deck ));
}

BOOST_AUTO_TEST_CASE(Section_ValidDecks) {

    ParseContext mode { { { ParseContext::PARSE_UNKNOWN_KEYWORD, InputError::IGNORE } } };
    Parser parser;
    ErrorGuard errors;

    const std::string minimal = "RUNSPEC\n"
                                "TEST1\n"
                                "GRID\n"
                                "TEST2\n"
                                "PROPS\n"
                                "TEST3\n"
                                "SOLUTION\n"
                                "TEST4\n"
                                "SCHEDULE\n"
                                "TEST5\n";

    BOOST_CHECK( Opm::DeckSection::checkSectionTopology( parser.parseString( minimal, mode, errors ), parser) );

    const std::string with_opt = "RUNSPEC\n"
                                "TEST1\n"
                                "GRID\n"
                                "TEST2\n"
                                "EDIT\n"
                                "TEST3\n"
                                "PROPS\n"
                                "TEST4\n"
                                "REGIONS\n"
                                "TEST5\n"
                                "SOLUTION\n"
                                "TEST6\n"
                                "SUMMARY\n"
                                "TEST7\n"
                                "SCHEDULE\n"
                                "TEST8\n";

    BOOST_CHECK(Opm::DeckSection::checkSectionTopology( parser.parseString( with_opt, mode, errors ), parser));
}

BOOST_AUTO_TEST_CASE(Section_InvalidDecks) {

    Parser parser;
    ParseContext mode { { { ParseContext::PARSE_UNKNOWN_KEYWORD, InputError::IGNORE } } };
    ErrorGuard errors;

    const std::string keyword_before_RUNSPEC =
                                "WWCT \n /\n"
                                "RUNSPEC\n"
                                "TEST1\n"
                                "GRID\n"
                                "TEST2\n"
                                "PROPS\n"
                                "TEST3\n"
                                "SOLUTION\n"
                                "TEST4\n"
                                "SCHEDULE\n"
                                "TEST5\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( keyword_before_RUNSPEC, mode, errors ), parser));

    const std::string wrong_order = "RUNSPEC\n"
                                    "TEST1\n"
                                    "EDIT\n"
                                    "TEST3\n"
                                    "GRID\n"
                                    "TEST2\n"
                                    "PROPS\n"
                                    "TEST4\n"
                                    "REGIONS\n"
                                    "TEST5\n"
                                    "SOLUTION\n"
                                    "TEST6\n"
                                    "SUMMARY\n"
                                    "TEST7\n"
                                    "SCHEDULE\n"
                                    "TEST8\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( wrong_order, mode, errors ), parser));

    const std::string duplicate = "RUNSPEC\n"
                                  "TEST1\n"
                                  "GRID\n"
                                  "TEST2\n"
                                  "GRID\n"
                                  "TEST21\n"
                                  "EDIT\n"
                                  "TEST3\n"
                                  "PROPS\n"
                                  "TEST4\n"
                                  "REGIONS\n"
                                  "TEST5\n"
                                  "SOLUTION\n"
                                  "TEST6\n"
                                  "SUMMARY\n"
                                  "TEST7\n"
                                  "SCHEDULE\n"
                                  "TEST8\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( duplicate, mode, errors ), parser));

    const std::string section_after_SCHEDULE = "RUNSPEC\n"
                                               "TEST1\n"
                                               "GRID\n"
                                               "TEST2\n"
                                               "PROPS\n"
                                               "TEST4\n"
                                               "REGIONS\n"
                                               "TEST5\n"
                                               "SOLUTION\n"
                                               "TEST6\n"
                                               "SUMMARY\n"
                                               "TEST7\n"
                                               "SCHEDULE\n"
                                               "TEST8\n"
                                               "EDIT\n"
                                               "TEST3\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( section_after_SCHEDULE, mode, errors ), parser));

    const std::string missing_runspec = "GRID\n"
                                        "TEST2\n"
                                        "PROPS\n"
                                        "TEST3\n"
                                        "SOLUTION\n"
                                        "TEST4\n"
                                        "SCHEDULE\n"
                                        "TEST5\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( missing_runspec, mode, errors ), parser));


    const std::string missing_GRID = "RUNSPEC\n"
                                     "TEST1\n"
                                     "PROPS\n"
                                     "TEST3\n"
                                     "SOLUTION\n"
                                     "TEST4\n"
                                     "SCHEDULE\n"
                                     "TEST5\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( missing_GRID, mode, errors ), parser));

   const std::string missing_PROPS = "RUNSPEC\n"
                                     "TEST1\n"
                                     "GRID\n"
                                     "TEST2\n"
                                     "SOLUTION\n"
                                     "TEST4\n"
                                     "SCHEDULE\n"
                                     "TEST5\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( missing_PROPS, mode, errors ), parser));

    const std::string missing_SOLUTION = "RUNSPEC\n"
                                         "TEST1\n"
                                         "GRID\n"
                                         "TEST2\n"
                                         "PROPS\n"
                                         "TEST3\n"
                                         "SCHEDULE\n"
                                         "TEST5\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( missing_SOLUTION, mode, errors ), parser));

    const std::string missing_SCHEDULE = "RUNSPEC\n"
                                         "TEST1\n"
                                         "GRID\n"
                                         "TEST2\n"
                                         "PROPS\n"
                                         "TEST3\n"
                                         "SOLUTION\n"
                                         "TEST4\n";

    BOOST_CHECK(!Opm::DeckSection::checkSectionTopology( parser.parseString( missing_SCHEDULE, mode, errors ), parser));
}
