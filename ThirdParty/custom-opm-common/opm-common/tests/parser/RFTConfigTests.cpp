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

#include <opm/input/eclipse/Schedule/RFTConfig.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

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
    const auto& sched = cse.sched;
    BOOST_CHECK(!sched.first_RFT().has_value());

    for (auto nstep = std::size_t(20), step = 0*nstep; step < nstep; ++step) {
        BOOST_CHECK_MESSAGE( !sched[step].rft_config().active(), "RFT Config must be Inactive");
        BOOST_CHECK_MESSAGE( !sched[step].rft_config().rft("P"), "RFT must be Inactive");
        BOOST_CHECK_MESSAGE( !sched[step].rft_config().rft("I"), "PLT must be Inactive");
    }
}

BOOST_AUTO_TEST_CASE(Simple_Deferred_Open)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_deferred_open());
    const auto& sched = cse.sched;
    BOOST_CHECK(!sched.first_RFT().has_value());

    for (auto nstep = std::size_t(20), step = 0*nstep; step < nstep; ++step) {
        BOOST_CHECK_MESSAGE( !sched[step].rft_config().active(), "RFT Config must be Inactive");
        BOOST_CHECK_MESSAGE( !sched[step].rft_config().rft("P"), "RFT must be Inactive");
        BOOST_CHECK_MESSAGE( !sched[step].rft_config().rft("I"), "PLT must be Inactive");
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
    const auto& sched = cse.sched;

    BOOST_CHECK_EQUAL(sched.first_RFT().value(), std::size_t( 7));

    BOOST_CHECK_MESSAGE(!sched[0].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[1].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[2].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[3].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[4].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[5].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[6].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE( sched[7].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[8].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[9].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 9)");

    BOOST_CHECK_MESSAGE(!sched[0].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[1].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[2].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[3].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[4].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[5].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[6].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[7].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[8].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[9].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 9)");

    BOOST_CHECK_MESSAGE(!sched[0].rft_config().active(), "RFT Config must be Inactive at Step 0");
    BOOST_CHECK_MESSAGE(!sched[1].rft_config().active(), "RFT Config must be Inactive at Step 1");
    BOOST_CHECK_MESSAGE(!sched[2].rft_config().active(), "RFT Config must be Inactive at Step 2");
    BOOST_CHECK_MESSAGE(!sched[3].rft_config().active(), "RFT Config must be Inactive at Step 3");
    BOOST_CHECK_MESSAGE(!sched[4].rft_config().active(), "RFT Config must be Inactive at Step 4");
    BOOST_CHECK_MESSAGE(!sched[5].rft_config().active(), "RFT Config must be Inactive at Step 5");
    BOOST_CHECK_MESSAGE(!sched[6].rft_config().active(), "RFT Config must be Inactive at Step 6");
    BOOST_CHECK_MESSAGE( sched[7].rft_config().active(), "RFT Config must be ACTIVE at Step 7");
    BOOST_CHECK_MESSAGE(!sched[8].rft_config().active(), "RFT Config must be Inactive at Step 8");
    BOOST_CHECK_MESSAGE(!sched[9].rft_config().active(), "RFT Config must be Inactive at Step 9");
}




BOOST_AUTO_TEST_CASE(Deferred_Open)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_deferred_open());
    const auto& sched = cse.sched;

    BOOST_CHECK_EQUAL(sched.first_RFT().value(), std::size_t( 7));

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 12)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE( sched[ 7].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[10].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 9)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().active(), "RFT Config must be Inactive at Step 0");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().active(), "RFT Config must be Inactive at Step 1");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().active(), "RFT Config must be Inactive at Step 2");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().active(), "RFT Config must be Inactive at Step 3");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().active(), "RFT Config must be Inactive at Step 4");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().active(), "RFT Config must be Inactive at Step 5");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().active(), "RFT Config must be Inactive at Step 6");
    BOOST_CHECK_MESSAGE( sched[ 7].rft_config().active(), "RFT Config must be ACTIVE at Step 7");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().active(), "RFT Config must be Inactive at Step 8");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().active(), "RFT Config must be Inactive at Step 9");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().active(), "RFT Config must be ACTIVE at Step 10");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().active(), "RFT Config must be Inactive at Step 11");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().active(), "RFT Config must be Inactive at Step 12");
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
   30 30 30/

WELOPEN
  'I' 'OPEN' /
/

TSTEP
-- 8..10 (sim step = 7..9)
   30 30 30 /

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
   30 30 30 /

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
    const auto& sched = cse.sched;
    BOOST_CHECK_EQUAL(sched.first_RFT().value(), std::size_t( 4));

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE( sched[ 4].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE( sched[13].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE( sched[14].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE( sched[15].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 15)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE( sched[ 7].rft_config().plt("P"), R"(Should Output PLT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[10].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE(!sched[13].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 15)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE( sched[ 7].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE( sched[ 8].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE( sched[ 9].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[10].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE( sched[13].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE( sched[14].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE( sched[15].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 15)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().plt("I"), R"(Should Output PLT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE(!sched[13].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 15)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().active(), R"(RFT Config must be Inactive at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().active(), R"(RFT Config must be Inactive at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().active(), R"(RFT Config must be Inactive at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().active(), R"(RFT Config must be Inactive at Step 3)");
    BOOST_CHECK_MESSAGE( sched[ 4].rft_config().active(), R"(RFT Config must be Active at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().active(), R"(RFT Config must be Inactive at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().active(), R"(RFT Config must be Inactive at Step 6)");
    BOOST_CHECK_MESSAGE( sched[ 7].rft_config().active(), R"(RFT Config must be Active at Step 7)");
    BOOST_CHECK_MESSAGE( sched[ 8].rft_config().active(), R"(RFT Config must be Active at Step 8)");
    BOOST_CHECK_MESSAGE( sched[ 9].rft_config().active(), R"(RFT Config must be Active at Step 9)");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().active(), R"(RFT Config must be Active at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().active(), R"(RFT Config must be Inactive at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().active(), R"(RFT Config must be Inactive at Step 12)");
    BOOST_CHECK_MESSAGE( sched[13].rft_config().active(), R"(RFT Config must be Active at Step 13)");
    BOOST_CHECK_MESSAGE( sched[14].rft_config().active(), R"(RFT Config must be Active at Step 14)");
    BOOST_CHECK_MESSAGE( sched[15].rft_config().active(), R"(RFT Config must be Active at Step 15)");
}



BOOST_AUTO_TEST_CASE(Deferred_Open)
{
    const auto cse = parseDeckString(basesetup_5x5x5() + simple_tstep_deferred_open());
    const auto& sched = cse.sched;

    BOOST_CHECK_EQUAL(sched.first_RFT().value(), std::size_t( 4));

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE( sched[ 4].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[10].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE( sched[13].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 15)");
    BOOST_CHECK_MESSAGE(!sched[16].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 16)");
    BOOST_CHECK_MESSAGE(!sched[17].rft_config().rft("P"), R"(Should NOT Output RFT Data for "P" at Step 17)");
    BOOST_CHECK_MESSAGE( sched[18].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 18)");
    BOOST_CHECK_MESSAGE( sched[19].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 19)");
    BOOST_CHECK_MESSAGE( sched[20].rft_config().rft("P"), R"(Should Output RFT Data for "P" at Step 20)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 9)");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().plt("P"), R"(Should Output PLT Data for "P" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 12)");
    BOOST_CHECK_MESSAGE(!sched[13].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 15)");
    BOOST_CHECK_MESSAGE(!sched[16].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 16)");
    BOOST_CHECK_MESSAGE(!sched[17].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 17)");
    BOOST_CHECK_MESSAGE(!sched[18].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 18)");
    BOOST_CHECK_MESSAGE(!sched[19].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 19)");
    BOOST_CHECK_MESSAGE(!sched[20].rft_config().plt("P"), R"(Should NOT Output PLT Data for "P" at Step 20)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE( sched[ 7].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE( sched[11].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE( sched[12].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE(!sched[13].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 15)");
    BOOST_CHECK_MESSAGE(!sched[16].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 16)");
    BOOST_CHECK_MESSAGE(!sched[17].rft_config().rft("I"), R"(Should NOT Output RFT Data for "I" at Step 17)");
    BOOST_CHECK_MESSAGE( sched[18].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 18)");
    BOOST_CHECK_MESSAGE( sched[19].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 19)");
    BOOST_CHECK_MESSAGE( sched[20].rft_config().rft("I"), R"(Should Output RFT Data for "I" at Step 20)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[10].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 12)");
    BOOST_CHECK_MESSAGE( sched[13].rft_config().plt("I"), R"(Should Output PLT Data for "I" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 15)");
    BOOST_CHECK_MESSAGE(!sched[16].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 16)");
    BOOST_CHECK_MESSAGE(!sched[17].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 17)");
    BOOST_CHECK_MESSAGE(!sched[18].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 18)");
    BOOST_CHECK_MESSAGE(!sched[19].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 19)");
    BOOST_CHECK_MESSAGE(!sched[20].rft_config().plt("I"), R"(Should NOT Output PLT Data for "I" at Step 20)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[10].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 12)");
    BOOST_CHECK_MESSAGE(!sched[13].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 15)");

    // NOTE: Not at FOPN becaus17e P2 was not declared at WRFTPLT:FOPN time.
    BOOST_CHECK_MESSAGE(!sched[16].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 16)");

    BOOST_CHECK_MESSAGE(!sched[17].rft_config().rft("P2"), R"(Should NOT Output RFT Data for "P2" at Step 17)");
    BOOST_CHECK_MESSAGE( sched[18].rft_config().rft("P2"), R"(Should Output RFT Data for "P2" at Step 18)");
    BOOST_CHECK_MESSAGE( sched[19].rft_config().rft("P2"), R"(Should Output RFT Data for "P2" at Step 19)");
    BOOST_CHECK_MESSAGE( sched[20].rft_config().rft("P2"), R"(Should Output RFT Data for "P2" at Step 20)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 3)");
    BOOST_CHECK_MESSAGE(!sched[ 4].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 6)");
    BOOST_CHECK_MESSAGE(!sched[ 7].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 9)");
    BOOST_CHECK_MESSAGE(!sched[10].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 10)");
    BOOST_CHECK_MESSAGE(!sched[11].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 11)");
    BOOST_CHECK_MESSAGE(!sched[12].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 12)");
    BOOST_CHECK_MESSAGE(!sched[13].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 15)");
    BOOST_CHECK_MESSAGE(!sched[16].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 16)");
    BOOST_CHECK_MESSAGE(!sched[17].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 17)");
    BOOST_CHECK_MESSAGE(!sched[18].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 18)");
    BOOST_CHECK_MESSAGE(!sched[19].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 19)");
    BOOST_CHECK_MESSAGE(!sched[20].rft_config().plt("P2"), R"(Should NOT Output PLT Data for "P2" at Step 20)");

    BOOST_CHECK_MESSAGE(!sched[ 0].rft_config().active(), R"(RFT Config must be Inactive at Step 0)");
    BOOST_CHECK_MESSAGE(!sched[ 1].rft_config().active(), R"(RFT Config must be Inactive at Step 1)");
    BOOST_CHECK_MESSAGE(!sched[ 2].rft_config().active(), R"(RFT Config must be Inactive at Step 2)");
    BOOST_CHECK_MESSAGE(!sched[ 3].rft_config().active(), R"(RFT Config must be Inactive at Step 3)");
    BOOST_CHECK_MESSAGE( sched[ 4].rft_config().active(), R"(RFT Config must be Active at Step 4)");
    BOOST_CHECK_MESSAGE(!sched[ 5].rft_config().active(), R"(RFT Config must be Inactive at Step 5)");
    BOOST_CHECK_MESSAGE(!sched[ 6].rft_config().active(), R"(RFT Config must be Inactive at Step 6)");
    BOOST_CHECK_MESSAGE( sched[ 7].rft_config().active(), R"(RFT Config must be Active at Step 7)");
    BOOST_CHECK_MESSAGE(!sched[ 8].rft_config().active(), R"(RFT Config must be Inactive at Step 8)");
    BOOST_CHECK_MESSAGE(!sched[ 9].rft_config().active(), R"(RFT Config must be Inactive at Step 9)");
    BOOST_CHECK_MESSAGE( sched[10].rft_config().active(), R"(RFT Config must be Active at Step 10)");
    BOOST_CHECK_MESSAGE( sched[11].rft_config().active(), R"(RFT Config must be Active at Step 11)");
    BOOST_CHECK_MESSAGE( sched[12].rft_config().active(), R"(RFT Config must be Active at Step 12)");
    BOOST_CHECK_MESSAGE( sched[13].rft_config().active(), R"(RFT Config must be Active at Step 13)");
    BOOST_CHECK_MESSAGE(!sched[14].rft_config().active(), R"(RFT Config must be Inactive at Step 14)");
    BOOST_CHECK_MESSAGE(!sched[15].rft_config().active(), R"(RFT Config must be Inactive at Step 15)");
    BOOST_CHECK_MESSAGE(!sched[16].rft_config().active(), R"(RFT Config must be Inactive at Step 15)");
    BOOST_CHECK_MESSAGE(!sched[17].rft_config().active(), R"(RFT Config must be Inactive at Step 15)");
    BOOST_CHECK_MESSAGE( sched[18].rft_config().active(), R"(RFT Config must be Active at Step 15)");
    BOOST_CHECK_MESSAGE( sched[19].rft_config().active(), R"(RFT Config must be Active at Step 15)");
    BOOST_CHECK_MESSAGE( sched[20].rft_config().active(), R"(RFT Config must be Active at Step 15)");
}



BOOST_AUTO_TEST_SUITE_END() // WRFTPLT

