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

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/EclipseState/SimulationConfig/ThresholdPressure.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>


using namespace Opm;

const std::string& inputStr = R"(
RUNSPEC
EQLOPTS
THPRES /


SOLUTION
THPRES
1 2 12.0/
1 3 5.0/
2 3 33.0 /
2 3 7.0/
/
)";

const std::string& inputStr_RESTART = R"(
RUNSPEC
EQLOPTS
THPRES /

SOLUTION
RESTART
 CASE 100 /
)";


const std::string& inputStr_RESTART2 = R"(
RUNSPEC
EQLOPTS
THPRES /

REGIONS
EQLNUM
   120*4 /
SOLUTION
RESTART
  CASE 100/
THPRES
1 3 5.0/
2 3 33.0 /
2 4 7.0/
/
)";

const std::string& inputStrWithEqlNum = R"(
RUNSPEC
EQLOPTS
THPRES /

REGIONS
EQLNUM
   120*3 /
SOLUTION
THPRES
1 2 12.0/
1 3 5.0/
2 3 33.0 /
2 3 7.0/
/
)";

const std::string& inputStrIrrevers2 = R"(
RUNSPEC
EQLOPTS
THPRES IRREVERS /

REGIONS
EQLNUM
   120*3 /
SOLUTION
THPRES
1 2  12.0 /
2 1 -12.0 /
2 3  23.0 /
3 2 -23.0 /
1 3  13.0 /
3 1 -13.0 /
/
)";



const std::string& inputStrWithEqlNumall0 = R"(
RUNSPEC
EQLOPTS
THPRES /

REGIONS
EQLNUM
   120*0 /
SOLUTION
THPRES
1 2 12.0/
1 3 5.0/
2 3 33.0 /
2 3 7.0/
/
)";


const std::string& inputStrNoSolutionSection =  R"(
RUNSPEC
EQLOPTS
THPRES /
)";

const std::string& inputStrNoTHPRESinSolutionNorRUNSPEC = R"(
RUNSPEC

SOLUTION

SCHEDULE
)";

const std::string& inputStrTHPRESinRUNSPECnotSoultion = R"(
RUNSPEC
EQLOPTS
ss /

SOLUTION
)";




const std::string& inputStrTooHighRegionNumbers = R"(
RUNSPEC
EQLOPTS
THPRES /


SOLUTION
THPRES
1 2 12.0/
4 3 5.0/
2 3 7.0/
/
)";


const std::string& inputStrMissingData = R"(
RUNSPEC
EQLOPTS
THPRES /


SOLUTION
THPRES
1 2 12.0/
2 3 5.0/
1 /
/
)";


const std::string& inputStrInconsistency =  R"(
RUNSPEC
EQLOPTS
THPRES /


SOLUTION
)";

const std::string& inputStrMissingPressure = R"(
RUNSPEC
EQLOPTS
THPRES /

REGIONS
EQLNUM
   120*3 /
SOLUTION
THPRES
1 2 12.0/
2 3 5.0/
2 3 /
/
)";

static Deck createDeck(const ParseContext& parseContext, const std::string& input) {
    Opm::Parser parser;
    ErrorGuard errors;
    return parser.parseString(input, parseContext, errors);
}

/// Setup fixture
struct Setup
{
    Deck deck;
    TableManager tablemanager;
    EclipseGrid grid;
    FieldPropsManager fp;
    InitConfig initConfig;
    ThresholdPressure threshPres;

    explicit Setup(const std::string& input) :
            deck(createDeck(ParseContext(), input)),
            tablemanager(deck),
            grid(10, 3, 4),
            fp(deck, Opm::Phases{true, true, true}, grid, tablemanager),
            initConfig(deck),
            threshPres(initConfig.restartRequested(), deck, fp)
    {
    }

    explicit Setup(const std::string& input, const ParseContext& parseContextArg) :
            deck(createDeck(parseContextArg, input)),
            tablemanager(deck),
            grid(10, 3, 4),
            fp(deck, Opm::Phases{true, true, true}, grid, tablemanager),
            initConfig(deck),
            threshPres(initConfig.restartRequested(), deck, fp)
    {
    }

};

BOOST_AUTO_TEST_CASE(ThresholdPressureDeckHasEqlnum) {
    Setup s(inputStrWithEqlNum);

    BOOST_CHECK(s.fp.has_int("EQLNUM"));
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
    BOOST_CHECK_EQUAL(0U, s.threshPres.size());
}

BOOST_AUTO_TEST_CASE(ThresholdPressureNoTHPREStest) {
    Setup s(inputStrNoTHPRESinSolutionNorRUNSPEC);
    Setup s2(inputStrTHPRESinRUNSPECnotSoultion);

    BOOST_CHECK_EQUAL(0U, s.threshPres.size());
    BOOST_CHECK_EQUAL(0U, s.threshPres.size());
}

BOOST_AUTO_TEST_CASE(ThresholdPressureThrowTest) {
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


BOOST_AUTO_TEST_CASE(Restart) {
    Setup sx(inputStr_RESTART);
    BOOST_CHECK(sx.threshPres.active());
    BOOST_CHECK_EQUAL(sx.threshPres.size(), 0U);
    BOOST_CHECK(sx.threshPres.restart());
}

BOOST_AUTO_TEST_CASE(Restart2) {
  Setup sx(inputStr_RESTART2);
  BOOST_CHECK(sx.threshPres.active());
  BOOST_CHECK_EQUAL(sx.threshPres.size(), 3U);
  BOOST_CHECK(sx.threshPres.restart());
}




BOOST_AUTO_TEST_CASE(HasPair) {
    Setup s(inputStrWithEqlNum);

    BOOST_CHECK( s.threshPres.hasRegionBarrier(1, 2));
    BOOST_CHECK(!s.threshPres.hasRegionBarrier(1, 7));
    BOOST_CHECK( s.threshPres.hasThresholdPressure(1, 2));
    BOOST_CHECK(!s.threshPres.hasThresholdPressure(1, 7));
    BOOST_CHECK_EQUAL(1200000.0, s.threshPres.getThresholdPressure(1, 2));
}

BOOST_AUTO_TEST_CASE(Irreversible) {
    Setup s(inputStrIrrevers2);
    const auto& thp = s.threshPres;
    BOOST_CHECK(thp.getThresholdPressure(1,2) == -thp.getThresholdPressure(2,1));
}
