/*
  Copyright 2020 Equnor ASA.

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

#define BOOST_TEST_MODULE RFTConfigTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/RFTConfig.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <cstddef>
#include <string>

namespace {
    struct Setup
    {
        Setup(const ::Opm::Deck& deck);

        ::Opm::EclipseState es;
        std::shared_ptr<::Opm::Python> python;
        ::Opm::Schedule     sched;
    };

    Setup::Setup(const ::Opm::Deck& deck)
        : es   (deck)
        , python(std::make_shared<::Opm::Python>() )
        , sched(deck, es, python)
    {}

    Setup parseDeckString(const std::string& input)
    {
        return { ::Opm::Parser{}.parseString(input) };
    }

    std::string basesetup_5x5x5()
    {
        return R"(RUNSPEC
DIMENS
  5 5 5 /

WELLDIMS
 2 5 2 2 /

TABDIMS
/

OIL
WATER
GAS
DISGAS
METRIC

GRID
INIT

DXV
  5*100.0 /
DYV
  5*100.0 /
DZV
  5*1.0 /
DEPTHZ
  36*2000.0 /

PORO
  125*0.3 /

PERMX
  125*100.0 /
COPY
  'PERMX' 'PERMY' /
  'PERMX' 'PERMZ' /
/
MULTIPLY
  'PERMZ' 0.1 /
/

PROPS
DENSITY
  812.3  1024.5  1.0 /

SCHEDULE
WELSPECS
 'I' 'I' 1 1 2000.0 WATER /
 'P' 'P' 5 5 2005.0 OIL /
/

COMPDAT
 'I' 0 0 1 5 'OPEN' 2* 0.5 /
 'P' 0 0 1 5 'OPEN' 2* 0.5 /
/
)";
    }
} // Anonymous

BOOST_AUTO_TEST_SUITE(No_RFT_Keywords)

namespace {

    std::string simple_tstep_all_open()
    {
        return R"(
WCONINJE
  'I' 'WATER' 'OPEN' 'RATE'  1000.0  1*  500.0 /
/

WCONPROD
  'P' 'OPEN' 'ORAT'  750.0  750.0  10.0E+3  1250.0  1*  75.0 /
/

TSTEP
-- 1   2   3   4   5   6  7 8  9  10..19
  0.1 0.2 0.3 0.4 0.5 1.5 5 10 20 10*30 /

)";
    }

    std::string simple_tstep_deferred_open()
    {
        return R"(
WCONINJE
  'I' 'WATER' 'SHUT' 'RATE'  1000.0  1*  500.0 /
/

WCONPROD
  'P' 'OPEN' 'ORAT'  750.0  750.0  10.0E+3  1250.0  1*  75.0 /
/

TSTEP
-- 1   2   3   4   5   6  7 8  9
  0.1 0.2 0.3 0.4 0.5 1.5 5 10 20 /

WELOPEN
  'I' 'OPEN' /
/

TSTEP
-- 10..19
   10*30 /

)";
    }
}

BOOST_AUTO_TEST_CASE(Simple)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_all_open());
    const auto& rftcfg = cse.sched.rftConfig();

    BOOST_CHECK_EQUAL(rftcfg.timeMap().size(), std::size_t(20));
    BOOST_CHECK_EQUAL(rftcfg.firstRFTOutput(), std::size_t(20));

    for (auto nstep = std::size_t(20), step = 0*nstep; step < nstep; ++step) {
        BOOST_CHECK_MESSAGE(!rftcfg.active(step), "RFT Config must be Inactive");

        BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", step),
                            R"(Well "I" must not have a Well Open RFT request")");

        BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", step),
                            R"(Well "P" must not have a Well Open RFT request")");
    }
}

BOOST_AUTO_TEST_CASE(Simple_Deferred_Open)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_deferred_open());
    const auto& rftcfg = cse.sched.rftConfig();

    BOOST_CHECK_EQUAL(rftcfg.timeMap().size(), std::size_t(20));
    BOOST_CHECK_EQUAL(rftcfg.firstRFTOutput(), std::size_t(20));

    for (auto nstep = std::size_t(20), step = 0*nstep; step < nstep; ++step) {
        BOOST_CHECK_MESSAGE(!rftcfg.active(step), "RFT Config must be Inactive");

        // WELOPEN does not imply RFT output
        BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", step),
                            R"(Well "I" must not have a Well Open RFT request")");

        BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", step),
                            R"(Well "P" must not have a Well Open RFT request")");
    }
}

BOOST_AUTO_TEST_SUITE_END() // No_RFT_Keywords

// =====================================================================

BOOST_AUTO_TEST_SUITE(WRFT)

namespace {

    std::string simple_tstep_all_open()
    {
        return R"(
WCONINJE
  'I' 'WATER' 'OPEN' 'RATE'  1000.0  1*  500.0 /
/

WCONPROD
  'P' 'OPEN' 'ORAT'  750.0  750.0  10.0E+3  1250.0  1*  75.0 /
/

TSTEP
-- 1   2   3   4  (sim step = 0..3)
  0.1 0.2 0.3 0.4 /

WRFT
-- Request RFT output for all wells SUBSEQUENTLY opened.
/

TSTEP
-- 5..7 (sim step = 4..6)
   3*30 /

WRFT
-- Request RFT output for 'P' and all wells subsequently opened
  'P' /
/

TSTEP
-- 8..10 (sim step = 7..9)
   3*30 /
)";
    }

    std::string simple_tstep_deferred_open()
    {
        return R"(
WCONINJE
  'I' 'WATER' 'SHUT' 'RATE'  1000.0  1*  500.0 /
/

WCONPROD
  'P' 'OPEN' 'ORAT'  750.0  750.0  10.0E+3  1250.0  1*  75.0 /
/

TSTEP
-- 1   2   3   4  (sim step = 0..3)
  0.1 0.2 0.3 0.4 /

WRFT
-- Request RFT output for all wells SUBSEQUENTLY opened.
/

TSTEP
-- 5..7 (sim step = 4..6)
   3*30 /

WELOPEN
  'I' 'OPEN' /
/

TSTEP
-- 8..10 (sim step = 7..9)
   3*30 /

WRFT
-- Request RFT output for 'P' and all wells subsequently opened
  'P' /
/

TSTEP
-- 11..13 (sim step = 10..12)
   3*30 /

)";
    }
}

BOOST_AUTO_TEST_CASE(Simple)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_all_open());
    const auto& rftcfg = cse.sched.rftConfig();

    BOOST_CHECK_EQUAL(rftcfg.timeMap().size(), std::size_t(11));
    BOOST_CHECK_EQUAL(rftcfg.firstRFTOutput(), std::size_t( 7));

    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 0), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 1), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 2), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 3), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 4), R"(Should NOT Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 5), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 6), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 7), R"(Should Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 8), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 9), R"(Should NOT Output RFT Data for "P" at Step 9)");

    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 0), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 1), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 2), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 3), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 4), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 5), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 6), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 7), R"(Should NOT Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 8), R"(Should NOT Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 9), R"(Should NOT Output RFT Data for "I" at Step 9)");

    BOOST_CHECK_MESSAGE(!rftcfg.active(0), "RFT Config must be Inactive at Step 0");
    BOOST_CHECK_MESSAGE(!rftcfg.active(1), "RFT Config must be Inactive at Step 1");
    BOOST_CHECK_MESSAGE(!rftcfg.active(2), "RFT Config must be Inactive at Step 2");
    BOOST_CHECK_MESSAGE(!rftcfg.active(3), "RFT Config must be Inactive at Step 3");
    BOOST_CHECK_MESSAGE(!rftcfg.active(4), "RFT Config must be Inactive at Step 4");
    BOOST_CHECK_MESSAGE(!rftcfg.active(5), "RFT Config must be Inactive at Step 5");
    BOOST_CHECK_MESSAGE(!rftcfg.active(6), "RFT Config must be Inactive at Step 6");
    BOOST_CHECK_MESSAGE( rftcfg.active(7), "RFT Config must be ACTIVE at Step 7");
    BOOST_CHECK_MESSAGE(!rftcfg.active(8), "RFT Config must be Inactive at Step 8");
    BOOST_CHECK_MESSAGE(!rftcfg.active(9), "RFT Config must be Inactive at Step 9");

    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 0),
                        R"(Well "I" must not have a Well Open RFT request at step 0")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 1),
                        R"(Well "I" must not have a Well Open RFT request at step 1")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 2),
                        R"(Well "I" must not have a Well Open RFT request at step 2")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 3),
                        R"(Well "I" must not have a Well Open RFT request at step 3")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 4),
                        R"(Well "I" must have a Well Open RFT request at step 4")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 5),
                        R"(Well "I" must have a Well Open RFT request at step 5")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 6),
                        R"(Well "I" must have a Well Open RFT request at step 6")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 7),
                        R"(Well "I" must have a Well Open RFT request at step 7")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 8),
                        R"(Well "I" must have a Well Open RFT request at step 8")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 9),
                        R"(Well "I" must have a Well Open RFT request at step 9")");

    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 0),
                        R"(Well "P" must not have a Well Open RFT request at step 0")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 1),
                        R"(Well "P" must not have a Well Open RFT request at step 1")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 2),
                        R"(Well "P" must not have a Well Open RFT request at step 2")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 3),
                        R"(Well "P" must not have a Well Open RFT request at step 3")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 4),
                        R"(Well "P" must have a Well Open RFT request at step 4")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 5),
                        R"(Well "P" must have a Well Open RFT request at step 5")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 6),
                        R"(Well "P" must have a Well Open RFT request at step 6")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 7),
                        R"(Well "P" must have a Well Open RFT request at step 7")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 8),
                        R"(Well "P" must have a Well Open RFT request at step 8")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 9),
                        R"(Well "P" must have a Well Open RFT request at step 9")");
}

BOOST_AUTO_TEST_CASE(Deferred_Open)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_deferred_open());
    const auto& rftcfg = cse.sched.rftConfig();

    BOOST_CHECK_EQUAL(rftcfg.timeMap().size(), std::size_t(14));
    BOOST_CHECK_EQUAL(rftcfg.firstRFTOutput(), std::size_t( 7));

    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  0), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  1), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  2), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  3), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  4), R"(Should NOT Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  5), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  6), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  7), R"(Should NOT Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  8), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  9), R"(Should NOT Output RFT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 10), R"(Should Output RFT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 11), R"(Should NOT Output RFT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 12), R"(Should NOT Output RFT Data for "P" at Step 12)");

    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  0), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  1), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  2), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  3), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  4), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  5), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  6), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I",  7), R"(Should Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  8), R"(Should NOT Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  9), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 10), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 11), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 12), R"(Should NOT Output RFT Data for "I" at Step 9)");

    BOOST_CHECK_MESSAGE(!rftcfg.active( 0), "RFT Config must be Inactive at Step 0");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 1), "RFT Config must be Inactive at Step 1");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 2), "RFT Config must be Inactive at Step 2");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 3), "RFT Config must be Inactive at Step 3");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 4), "RFT Config must be Inactive at Step 4");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 5), "RFT Config must be Inactive at Step 5");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 6), "RFT Config must be Inactive at Step 6");
    BOOST_CHECK_MESSAGE( rftcfg.active( 7), "RFT Config must be ACTIVE at Step 7");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 8), "RFT Config must be Inactive at Step 8");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 9), "RFT Config must be Inactive at Step 9");
    BOOST_CHECK_MESSAGE( rftcfg.active(10), "RFT Config must be ACTIVE at Step 10");
    BOOST_CHECK_MESSAGE(!rftcfg.active(11), "RFT Config must be Inactive at Step 11");
    BOOST_CHECK_MESSAGE(!rftcfg.active(12), "RFT Config must be Inactive at Step 12");

    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 0),
                        R"(Well "I" must not have a Well Open RFT request at step 0")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 1),
                        R"(Well "I" must not have a Well Open RFT request at step 1")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 2),
                        R"(Well "I" must not have a Well Open RFT request at step 2")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("I", 3),
                        R"(Well "I" must not have a Well Open RFT request at step 3")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 4),
                        R"(Well "I" must have a Well Open RFT request at step 4")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 5),
                        R"(Well "I" must have a Well Open RFT request at step 5")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 6),
                        R"(Well "I" must have a Well Open RFT request at step 6")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 7),
                        R"(Well "I" must have a Well Open RFT request at step 7")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 8),
                        R"(Well "I" must have a Well Open RFT request at step 8")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 9),
                        R"(Well "I" must have a Well Open RFT request at step 9")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 10),
                        R"(Well "I" must have a Well Open RFT request at step 10")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 11),
                        R"(Well "I" must have a Well Open RFT request at step 11")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("I", 12),
                        R"(Well "I" must have a Well Open RFT request at step 12")");

    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 0),
                        R"(Well "P" must not have a Well Open RFT request at step 0")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 1),
                        R"(Well "P" must not have a Well Open RFT request at step 1")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 2),
                        R"(Well "P" must not have a Well Open RFT request at step 2")");
    BOOST_CHECK_MESSAGE(!rftcfg.getWellOpenRFT("P", 3),
                        R"(Well "P" must not have a Well Open RFT request at step 3")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 4),
                        R"(Well "P" must have a Well Open RFT request at step 4")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 5),
                        R"(Well "P" must have a Well Open RFT request at step 5")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 6),
                        R"(Well "P" must have a Well Open RFT request at step 6")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 7),
                        R"(Well "P" must have a Well Open RFT request at step 7")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 8),
                        R"(Well "P" must have a Well Open RFT request at step 8")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 9),
                        R"(Well "P" must have a Well Open RFT request at step 9")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 10),
                        R"(Well "P" must have a Well Open RFT request at step 10")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 11),
                        R"(Well "P" must have a Well Open RFT request at step 11")");
    BOOST_CHECK_MESSAGE(rftcfg.getWellOpenRFT("P", 12),
                        R"(Well "P" must have a Well Open RFT request at step 12")");
}

BOOST_AUTO_TEST_SUITE_END() // WRFT
// =====================================================================

BOOST_AUTO_TEST_SUITE(WRFTPLT)

namespace {

    std::string simple_tstep_all_open()
    {
        return R"(
WCONINJE
  'I' 'WATER' 'OPEN' 'RATE'  1000.0  1*  500.0 /
/

WCONPROD
  'P' 'OPEN' 'ORAT'  750.0  750.0  10.0E+3  1250.0  1*  75.0 /
/

TSTEP
-- 1   2   3   4  (sim step = 0..3)
  0.1 0.2 0.3 0.4 /

WRFTPLT
  'P' FOPN /
/

TSTEP
-- 5..7 (sim step = 4..6)
   3*30 /

WRFTPLT
  'P' 1* YES /
  'I' REPT /
/

TSTEP
-- 8..10 (sim step = 7..9)
   3*30 /

WRFTPLT
  'P' YES /
  'I' NO YES /
/

TSTEP
-- 11..13 (sim step = 10..12)
   3*30 /

WRFTPLT
  '*' TIMESTEP /
/

TSTEP
-- 14..15 (sim step = 13..14)
   30 30 /
)";
    }

    std::string simple_tstep_deferred_open()
    {
        return R"(
WCONINJE
  'I' 'WATER' 'SHUT' 'RATE'  1000.0  1*  500.0 /
/

WCONPROD
  'P' 'OPEN' 'ORAT'  500.0  500.0  10.0E+3  1250.0  1*  75.0 /
/

TSTEP
-- 1   2   3   4  (sim step = 0..3)
  0.1 0.2 0.3 0.4 /

WRFTPLT
  '*' FOPN /
/

TSTEP
-- 5..7 (sim step = 4..6)
   3*30 /

WELOPEN
  'I' 'OPEN' /
/

TSTEP
-- 8..10 (sim step = 7..9)
   3*30 /

WRFTPLT
  'P' 1* YES /
  'I' REPT /
/

WELSPECS
 'P2' 'P' 1 5 2005.0 OIL /
/

COMPDAT
 'P2' 0 0 1 5 'OPEN' 2* 0.5 /
/

TSTEP
-- 11..13 (sim step = 10..12)
   3*30 /

WRFTPLT
  'P' YES /
  'I' NO YES /
/

TSTEP
-- 14..16 (sim step = 13..15)
   3*30 /

WCONPROD
  'P2' 'OPEN' 'ORAT'  250.0  250.0  10.0E+3  1250.0  1*  75.0 /
/

TSTEP
-- 17..18 (sim step = 16..17)
  30 30 /

WRFTPLT
  '*' TIMESTEP /
/

TSTEP
-- 19..20 (sim step = 18..19)
   30 30 /
)";
    }
}

BOOST_AUTO_TEST_CASE(All_Open)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_all_open());
    const auto& rftcfg = cse.sched.rftConfig();

    BOOST_CHECK_EQUAL(rftcfg.timeMap().size(), std::size_t(16));
    BOOST_CHECK_EQUAL(rftcfg.firstRFTOutput(), std::size_t( 4));

    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  0), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  1), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  2), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  3), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P",  4), R"(Should Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  5), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  6), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  7), R"(Should NOT Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  8), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  9), R"(Should NOT Output RFT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 10), R"(Should Output RFT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 11), R"(Should NOT Output RFT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 12), R"(Should NOT Output RFT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 13), R"(Should Output RFT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 14), R"(Should Output RFT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 15), R"(Should Output RFT Data for "P" at Step 15)");

    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  0), R"(Should NOT Output PLT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  1), R"(Should NOT Output PLT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  2), R"(Should NOT Output PLT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  3), R"(Should NOT Output PLT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  4), R"(Should NOT Output PLT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  5), R"(Should NOT Output PLT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  6), R"(Should NOT Output PLT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE( rftcfg.plt("P",  7), R"(Should Output PLT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  8), R"(Should NOT Output PLT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  9), R"(Should NOT Output PLT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 10), R"(Should NOT Output PLT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 11), R"(Should NOT Output PLT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 12), R"(Should NOT Output PLT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 13), R"(Should NOT Output PLT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 14), R"(Should NOT Output PLT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 15), R"(Should NOT Output PLT Data for "P" at Step 15)");

    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  0), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  1), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  2), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  3), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  4), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  5), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  6), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I",  7), R"(Should Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I",  8), R"(Should Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I",  9), R"(Should Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 10), R"(Should NOT Output RFT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 11), R"(Should NOT Output RFT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 12), R"(Should NOT Output RFT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 13), R"(Should Output RFT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 14), R"(Should Output RFT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 15), R"(Should Output RFT Data for "I" at Step 15)");

    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  0), R"(Should NOT Output PLT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  1), R"(Should NOT Output PLT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  2), R"(Should NOT Output PLT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  3), R"(Should NOT Output PLT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  4), R"(Should NOT Output PLT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  5), R"(Should NOT Output PLT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  6), R"(Should NOT Output PLT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  7), R"(Should NOT Output PLT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  8), R"(Should NOT Output PLT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  9), R"(Should NOT Output PLT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE( rftcfg.plt("I", 10), R"(Should Output PLT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 11), R"(Should NOT Output PLT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 12), R"(Should NOT Output PLT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 13), R"(Should NOT Output PLT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 14), R"(Should NOT Output PLT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 15), R"(Should NOT Output PLT Data for "I" at Step 15)");

    BOOST_CHECK_MESSAGE(!rftcfg.active( 0), R"(RFT Config must be Inactive at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 1), R"(RFT Config must be Inactive at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 2), R"(RFT Config must be Inactive at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 3), R"(RFT Config must be Inactive at Step 3)");
    BOOST_CHECK_MESSAGE( rftcfg.active( 4), R"(RFT Config must be Active at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 5), R"(RFT Config must be Inactive at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 6), R"(RFT Config must be Inactive at Step 6)");
    BOOST_CHECK_MESSAGE( rftcfg.active( 7), R"(RFT Config must be Active at Step 7)");
    BOOST_CHECK_MESSAGE( rftcfg.active( 8), R"(RFT Config must be Active at Step 8)");
    BOOST_CHECK_MESSAGE( rftcfg.active( 9), R"(RFT Config must be Active at Step 9)");
    BOOST_CHECK_MESSAGE( rftcfg.active(10), R"(RFT Config must be Active at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.active(11), R"(RFT Config must be Inactive at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.active(12), R"(RFT Config must be Inactive at Step 12)");
    BOOST_CHECK_MESSAGE( rftcfg.active(13), R"(RFT Config must be Active at Step 13)");
    BOOST_CHECK_MESSAGE( rftcfg.active(14), R"(RFT Config must be Active at Step 14)");
    BOOST_CHECK_MESSAGE( rftcfg.active(15), R"(RFT Config must be Active at Step 15)");
}

BOOST_AUTO_TEST_CASE(Deferred_Open)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_deferred_open());
    const auto& rftcfg = cse.sched.rftConfig();

    BOOST_CHECK_EQUAL(rftcfg.timeMap().size(), std::size_t(21));
    BOOST_CHECK_EQUAL(rftcfg.firstRFTOutput(), std::size_t( 4));

    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  0), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  1), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  2), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  3), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P",  4), R"(Should Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  5), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  6), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  7), R"(Should NOT Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  8), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P",  9), R"(Should NOT Output RFT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 10), R"(Should NOT Output RFT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 11), R"(Should NOT Output RFT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 12), R"(Should NOT Output RFT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 13), R"(Should Output RFT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 14), R"(Should NOT Output RFT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 15), R"(Should NOT Output RFT Data for "P" at Step 15)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 16), R"(Should NOT Output RFT Data for "P" at Step 16)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P", 17), R"(Should NOT Output RFT Data for "P" at Step 17)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 18), R"(Should Output RFT Data for "P" at Step 18)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 19), R"(Should Output RFT Data for "P" at Step 19)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P", 20), R"(Should Output RFT Data for "P" at Step 20)");

    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  0), R"(Should NOT Output PLT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  1), R"(Should NOT Output PLT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  2), R"(Should NOT Output PLT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  3), R"(Should NOT Output PLT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  4), R"(Should NOT Output PLT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  5), R"(Should NOT Output PLT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  6), R"(Should NOT Output PLT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  7), R"(Should NOT Output PLT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  8), R"(Should NOT Output PLT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P",  9), R"(Should NOT Output PLT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE( rftcfg.plt("P", 10), R"(Should Output PLT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 11), R"(Should NOT Output PLT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 12), R"(Should NOT Output PLT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 13), R"(Should NOT Output PLT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 14), R"(Should NOT Output PLT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 15), R"(Should NOT Output PLT Data for "P" at Step 15)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 16), R"(Should NOT Output PLT Data for "P" at Step 16)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 17), R"(Should NOT Output PLT Data for "P" at Step 17)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 18), R"(Should NOT Output PLT Data for "P" at Step 18)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 19), R"(Should NOT Output PLT Data for "P" at Step 19)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P", 20), R"(Should NOT Output PLT Data for "P" at Step 20)");

    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  0), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  1), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  2), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  3), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  4), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  5), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  6), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I",  7), R"(Should Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  8), R"(Should NOT Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I",  9), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 10), R"(Should Output RFT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 11), R"(Should Output RFT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 12), R"(Should Output RFT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 13), R"(Should Output RFT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 14), R"(Should NOT Output RFT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 15), R"(Should NOT Output RFT Data for "I" at Step 15)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 16), R"(Should NOT Output RFT Data for "I" at Step 16)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("I", 17), R"(Should NOT Output RFT Data for "I" at Step 17)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 18), R"(Should Output RFT Data for "I" at Step 18)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 19), R"(Should Output RFT Data for "I" at Step 19)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("I", 20), R"(Should Output RFT Data for "I" at Step 20)");

    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  0), R"(Should NOT Output PLT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  1), R"(Should NOT Output PLT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  2), R"(Should NOT Output PLT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  3), R"(Should NOT Output PLT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  4), R"(Should NOT Output PLT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  5), R"(Should NOT Output PLT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  6), R"(Should NOT Output PLT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  7), R"(Should NOT Output PLT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  8), R"(Should NOT Output PLT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I",  9), R"(Should NOT Output PLT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 10), R"(Should NOT Output PLT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 11), R"(Should NOT Output PLT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 12), R"(Should NOT Output PLT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE( rftcfg.plt("I", 13), R"(Should Output PLT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 14), R"(Should NOT Output PLT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 15), R"(Should NOT Output PLT Data for "I" at Step 15)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 16), R"(Should NOT Output PLT Data for "I" at Step 16)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 17), R"(Should NOT Output PLT Data for "I" at Step 17)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 18), R"(Should NOT Output PLT Data for "I" at Step 18)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 19), R"(Should NOT Output PLT Data for "I" at Step 19)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("I", 20), R"(Should NOT Output PLT Data for "I" at Step 20)");

    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  0), R"(Should NOT Output RFT Data for "P2" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  1), R"(Should NOT Output RFT Data for "P2" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  2), R"(Should NOT Output RFT Data for "P2" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  3), R"(Should NOT Output RFT Data for "P2" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  4), R"(Should NOT Output RFT Data for "P2" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  5), R"(Should NOT Output RFT Data for "P2" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  6), R"(Should NOT Output RFT Data for "P2" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  7), R"(Should NOT Output RFT Data for "P2" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  8), R"(Should NOT Output RFT Data for "P2" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2",  9), R"(Should NOT Output RFT Data for "P2" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 10), R"(Should NOT Output RFT Data for "P2" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 11), R"(Should NOT Output RFT Data for "P2" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 12), R"(Should NOT Output RFT Data for "P2" at Step 12)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 13), R"(Should NOT Output RFT Data for "P2" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 14), R"(Should NOT Output RFT Data for "P2" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 15), R"(Should NOT Output RFT Data for "P2" at Step 15)");

    // NOTE: Not at FOPN because P2 was not declared at WRFTPLT:FOPN time.
    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 16), R"(Should NOT Output RFT Data for "P2" at Step 16)");

    BOOST_CHECK_MESSAGE(!rftcfg.rft("P2", 17), R"(Should NOT Output RFT Data for "P2" at Step 17)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P2", 18), R"(Should Output RFT Data for "P2" at Step 18)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P2", 19), R"(Should Output RFT Data for "P2" at Step 19)");
    BOOST_CHECK_MESSAGE( rftcfg.rft("P2", 20), R"(Should Output RFT Data for "P2" at Step 20)");

    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  0), R"(Should NOT Output PLT Data for "P2" at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  1), R"(Should NOT Output PLT Data for "P2" at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  2), R"(Should NOT Output PLT Data for "P2" at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  3), R"(Should NOT Output PLT Data for "P2" at Step 3)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  4), R"(Should NOT Output PLT Data for "P2" at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  5), R"(Should NOT Output PLT Data for "P2" at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  6), R"(Should NOT Output PLT Data for "P2" at Step 6)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  7), R"(Should NOT Output PLT Data for "P2" at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  8), R"(Should NOT Output PLT Data for "P2" at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2",  9), R"(Should NOT Output PLT Data for "P2" at Step 9)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 10), R"(Should NOT Output PLT Data for "P2" at Step 10)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 11), R"(Should NOT Output PLT Data for "P2" at Step 11)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 12), R"(Should NOT Output PLT Data for "P2" at Step 12)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 13), R"(Should NOT Output PLT Data for "P2" at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 14), R"(Should NOT Output PLT Data for "P2" at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 15), R"(Should NOT Output PLT Data for "P2" at Step 15)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 16), R"(Should NOT Output PLT Data for "P2" at Step 16)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 17), R"(Should NOT Output PLT Data for "P2" at Step 17)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 18), R"(Should NOT Output PLT Data for "P2" at Step 18)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 19), R"(Should NOT Output PLT Data for "P2" at Step 19)");
    BOOST_CHECK_MESSAGE(!rftcfg.plt("P2", 20), R"(Should NOT Output PLT Data for "P2" at Step 20)");

    BOOST_CHECK_MESSAGE(!rftcfg.active( 0), R"(RFT Config must be Inactive at Step 0)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 1), R"(RFT Config must be Inactive at Step 1)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 2), R"(RFT Config must be Inactive at Step 2)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 3), R"(RFT Config must be Inactive at Step 3)");
    BOOST_CHECK_MESSAGE( rftcfg.active( 4), R"(RFT Config must be Active at Step 4)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 5), R"(RFT Config must be Inactive at Step 5)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 6), R"(RFT Config must be Inactive at Step 6)");
    BOOST_CHECK_MESSAGE( rftcfg.active( 7), R"(RFT Config must be Active at Step 7)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 8), R"(RFT Config must be Inactive at Step 8)");
    BOOST_CHECK_MESSAGE(!rftcfg.active( 9), R"(RFT Config must be Inactive at Step 9)");
    BOOST_CHECK_MESSAGE( rftcfg.active(10), R"(RFT Config must be Active at Step 10)");
    BOOST_CHECK_MESSAGE( rftcfg.active(11), R"(RFT Config must be Active at Step 11)");
    BOOST_CHECK_MESSAGE( rftcfg.active(12), R"(RFT Config must be Active at Step 12)");
    BOOST_CHECK_MESSAGE( rftcfg.active(13), R"(RFT Config must be Active at Step 13)");
    BOOST_CHECK_MESSAGE(!rftcfg.active(14), R"(RFT Config must be Inactive at Step 14)");
    BOOST_CHECK_MESSAGE(!rftcfg.active(15), R"(RFT Config must be Inactive at Step 15)");
    BOOST_CHECK_MESSAGE(!rftcfg.active(16), R"(RFT Config must be Inactive at Step 15)");
    BOOST_CHECK_MESSAGE(!rftcfg.active(17), R"(RFT Config must be Inactive at Step 15)");
    BOOST_CHECK_MESSAGE( rftcfg.active(18), R"(RFT Config must be Active at Step 15)");
    BOOST_CHECK_MESSAGE( rftcfg.active(19), R"(RFT Config must be Active at Step 15)");
    BOOST_CHECK_MESSAGE( rftcfg.active(20), R"(RFT Config must be Active at Step 15)");
}

BOOST_AUTO_TEST_SUITE_END() // WRFTPLT
