/*
  Copyright 2015 Statoil ASA.

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



#include <algorithm>

#define BOOST_TEST_MODULE ThresholdPressureTests

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/ThresholdPressure.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>


using namespace Opm;

const std::string& inputStr = "RUNSPEC\n"
                              "EQLOPTS\n"
                              "THPRES /\n "
                              "\n"

                              "SOLUTION\n"
                              "THPRES\n"
                              "1 2 12.0/\n"
                              "1 3 5.0/\n"
                              "2 3 33.0 /\n"
                              "2 3 7.0/\n"
                              "/\n"
                              "\n";

const std::string& inputStrWithEqlNum =
                              "RUNSPEC\n"
                              "EQLOPTS\n"
                              "THPRES /\n "
                              "\n"
                              "REGIONS\n"
                              "EQLNUM\n"
                              "   27*3 /\n"
                              "SOLUTION\n"
                              "THPRES\n"
                              "1 2 12.0/\n"
                              "1 3 5.0/\n"
                              "2 3 33.0 /\n"
                              "2 3 7.0/\n"
                              "/\n"
                              "\n";


const std::string& inputStrWithEqlNumall0 =
                              "RUNSPEC\n"
                              "EQLOPTS\n"
                              "THPRES /\n "
                              "\n"
                              "REGIONS\n"
                              "EQLNUM\n"
                              "   27*0 /\n"
                              "SOLUTION\n"
                              "THPRES\n"
                              "1 2 12.0/\n"
                              "1 3 5.0/\n"
                              "2 3 33.0 /\n"
                              "2 3 7.0/\n"
                              "/\n"
                              "\n";


const std::string& inputStrNoSolutionSection =  "RUNSPEC\n"
                                                "EQLOPTS\n"
                                                "THPRES /\n "
                                                "\n";


const std::string& inputStrNoTHPRESinSolutionNorRUNSPEC = "RUNSPEC\n"
                                                          "\n"
                                                          "SOLUTION\n"
                                                          "\n"
                                                          "SCHEDULE\n";

const std::string& inputStrTHPRESinRUNSPECnotSoultion = "RUNSPEC\n"
                                                        "EQLOPTS\n"
                                                        "ss /\n "
                                                        "\n"
                                                        "SOLUTION\n"
                                                        "\n";


const std::string& inputStrIrrevers = "RUNSPEC\n"
                                      "EQLOPTS\n"
                                      "THPRES IRREVERS/\n "
                                      "\n"

                                      "SOLUTION\n"
                                      "THPRES\n"
                                      "/\n"
                                      "\n";

const std::string& inputStrInconsistency =  "RUNSPEC\n"
                                            "EQLOPTS\n"
                                            "THPRES /\n "
                                            "\n"

                                            "SOLUTION\n"
                                            "\n";

const std::string& inputStrTooHighRegionNumbers = "RUNSPEC\n"
                                                  "EQLOPTS\n"
                                                  "THPRES /\n "
                                                  "\n"

                                                  "SOLUTION\n"
                                                  "THPRES\n"
                                                  "1 2 12.0/\n"
                                                  "4 3 5.0/\n"
                                                  "2 3 7.0/\n"
                                                  "/\n"
                                                  "\n";


const std::string& inputStrMissingData = "RUNSPEC\n"
                                         "EQLOPTS\n"
                                         "THPRES /\n "
                                         "\n"

                                         "SOLUTION\n"
                                         "THPRES\n"
                                         "1 2 12.0/\n"
                                         "2 3 5.0/\n"
                                         "1 /\n"
                                         "/\n"
                                         "\n";


const std::string& inputStrMissingPressure = "RUNSPEC\n"
                                         "EQLOPTS\n"
                                         "THPRES /\n "
                                         "\n"
                                         "REGIONS\n"
                                         "EQLNUM\n"
                                         "   27*3 /\n"
                                         "SOLUTION\n"
                                         "THPRES\n"
                                         "1 2 12.0/\n"
                                         "2 3 5.0/\n"
                                         "2 3 /\n"
                                         "/\n"
                                         "\n";

static DeckPtr createDeck(const ParseContext& parseContext, const std::string& input) {
    Opm::Parser parser;
    return parser.parseString(input, parseContext);
}

/// Setup fixture
struct Setup
{
    ParseContext parseContext;
    DeckPtr deck;
    TableManager tablemanager;
    EclipseGrid grid;
    Eclipse3DProperties props;
    ThresholdPressure threshPres;

    explicit Setup(const std::string& input) :
            parseContext(),
            deck(createDeck(parseContext, input)),
            tablemanager(*deck),
            grid(10, 3, 4),
            props(*deck, tablemanager, grid),
            threshPres(*deck, props)
    {
    }
    explicit Setup(const std::string& input, ParseContext& parseContextArg) :
            parseContext(), // not used
            deck(createDeck(parseContextArg, input)),
            tablemanager(*deck),
            grid(10, 3, 4),
            props(*deck, tablemanager, grid),
            threshPres(*deck, props)
    {
    }

};

BOOST_AUTO_TEST_CASE(ThresholdPressureDeckHasEqlnum) {
    Setup s(inputStrWithEqlNum);

    BOOST_CHECK(s.props.hasDeckIntGridProperty("EQLNUM"));
}

BOOST_AUTO_TEST_CASE(ThresholdPressureTest) {
    Setup s(inputStrWithEqlNum);
    auto thp = s.threshPres;

    BOOST_CHECK_EQUAL(thp.getThresholdPressure(1, 2), 1200000.0);
    BOOST_CHECK_EQUAL(thp.getThresholdPressure(2, 1), 1200000.0);
    BOOST_CHECK_EQUAL(thp.getThresholdPressure(1, 3), 500000.0);
    BOOST_CHECK_EQUAL(thp.getThresholdPressure(3, 1), 500000.0);
    BOOST_CHECK_EQUAL(thp.getThresholdPressure(2, 3), 700000.0);
    BOOST_CHECK_EQUAL(thp.getThresholdPressure(3, 2), 700000.0);
}

BOOST_AUTO_TEST_CASE(ThresholdPressureEmptyTest) {
    Setup s(inputStrNoSolutionSection);
    BOOST_CHECK_EQUAL(0, s.threshPres.size());
}

BOOST_AUTO_TEST_CASE(ThresholdPressureNoTHPREStest) {
    Setup s(inputStrNoTHPRESinSolutionNorRUNSPEC);
    Setup s2(inputStrTHPRESinRUNSPECnotSoultion);

    BOOST_CHECK_EQUAL(0, s.threshPres.size());
    BOOST_CHECK_EQUAL(0, s.threshPres.size());
}

BOOST_AUTO_TEST_CASE(ThresholdPressureThrowTest) {
    BOOST_CHECK_THROW(Setup sx(inputStrIrrevers), std::runtime_error);
    BOOST_CHECK_THROW(Setup sx(inputStrInconsistency), std::runtime_error);
    BOOST_CHECK_THROW(Setup sx(inputStrTooHighRegionNumbers), std::runtime_error);
    BOOST_CHECK_THROW(Setup sx(inputStrMissingData), std::runtime_error);
    BOOST_CHECK_THROW(Setup sx(inputStr), std::runtime_error);
    BOOST_CHECK_THROW(Setup sx(inputStrWithEqlNumall0), std::runtime_error);

    BOOST_CHECK_THROW(Setup sx(inputStr), std::runtime_error);
    ParseContext pc;
    pc.update(ParseContext::UNSUPPORTED_INITIAL_THPRES, InputError::IGNORE);
    BOOST_CHECK_NO_THROW(Setup sx(inputStrMissingPressure, pc));

    Setup s(inputStrMissingPressure, pc);
    BOOST_CHECK( s.threshPres.hasRegionBarrier(2, 3));
    BOOST_CHECK(!s.threshPres.hasThresholdPressure(2, 3));

    pc.update(ParseContext::INTERNAL_ERROR_UNINITIALIZED_THPRES, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(s.threshPres.getThresholdPressure(2, 3), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(HasPair) {
    Setup s(inputStrWithEqlNum);

    BOOST_CHECK( s.threshPres.hasRegionBarrier(1, 2));
    BOOST_CHECK(!s.threshPres.hasRegionBarrier(1, 7));
    BOOST_CHECK( s.threshPres.hasThresholdPressure(1, 2));
    BOOST_CHECK(!s.threshPres.hasThresholdPressure(1, 7));
    BOOST_CHECK_EQUAL(1200000.0, s.threshPres.getThresholdPressure(1, 2));
}
