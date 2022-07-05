/*
  Copyright 2016 Statoil ASA.

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

#define BOOST_TEST_MODULE RestartConfigTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include <cstddef>
#include <map>
#include <string>
#include <vector>

using namespace Opm;

namespace {

std::vector<std::string> filter_keywords(const std::map<std::string, int>& keywords) {
    std::vector<std::string> kwlist;
    for (const auto& [kw, value] : keywords) {
        if (kw == "BASIC" || kw == "FREQ" || kw == "RESTART")
            continue;

        if (value == 0)
            continue;

        kwlist.push_back(kw);
    }

    return kwlist;
}

std::string grid()
{
    return { R"(RUNSPEC
DIMENS
 10 10 10 /
START
 21 MAY 1981 /

GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /
)" };
}

Schedule make_schedule(std::string sched_input, bool add_grid = true) {
    if (add_grid)
        sched_input = grid() + sched_input;

    auto deck = Parser{}.parseString( sched_input );
    EclipseState es(deck);
    return Schedule(deck, es);
}

} // namespace Anonymous

BOOST_AUTO_TEST_CASE(TestIOConfigCreation) {
    const std::string deckData  = R"(
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 FREQ=2 /
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  JAN 2011 /
/
)";

    auto sched = make_schedule(deckData);

    BOOST_CHECK_MESSAGE(! sched.write_rst_file(0), "Must NOT write restart output on step 0");
    BOOST_CHECK_MESSAGE(! sched.write_rst_file(1), "Must NOT write restart output on step 1");
    BOOST_CHECK_MESSAGE(  sched.write_rst_file(2), "Must write restart output on step 2");
    BOOST_CHECK_MESSAGE(! sched.write_rst_file(3), "Must NOT write restart output on step 3");
}


BOOST_AUTO_TEST_CASE(TestIOConfigCreationWithSolutionRPTRST) {
    const std::string deckData  = R"(
SOLUTION

RPTRST
BASIC=1/

RPTRST
BASIC=3 FREQ=5 /

SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
DATES             -- 2
 20  JAN 2010 /
/
RPTRST
BASIC=3 FREQ=2 /
DATES             -- 3
 20  JAN 2011 /
/
)";

    auto sched = make_schedule(deckData);

    BOOST_CHECK_MESSAGE(  sched.write_rst_file(0), "Must write restart file for report step 0");
    BOOST_CHECK_MESSAGE(! sched.write_rst_file(1), "Must NOT write restart file for report step 1");
    BOOST_CHECK_MESSAGE(! sched.write_rst_file(2), "Must NOT write restart file for report step 2");
    BOOST_CHECK_MESSAGE(! sched.write_rst_file(3), "Must NOT write restart file for report step 3");
}


BOOST_AUTO_TEST_CASE(TestIOConfigCreationWithSolutionRPTSOL) {
    const std::string deckData = R"(
SOLUTION

RPTSOL
RESTART=2
/

SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 FREQ=3
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
RESTART=1
/
)";

    const std::string deckData2 = R"(
SOLUTION

RPTSOL
0 0 0 0 0 0 2
/

SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 FREQ=3
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
RESTART=1
/
)";

    auto sched1 = make_schedule(deckData);
    auto sched2 = make_schedule(deckData2);

    BOOST_CHECK_MESSAGE(sched1.write_rst_file(0), "SCHED1 must write restart file for report step 0");
    BOOST_CHECK_MESSAGE(sched2.write_rst_file(0), "SCHED2 must write restart file for report step 0");
}


BOOST_AUTO_TEST_CASE(RPTRST_AND_RPTSOL_SOLUTION)
{
    const auto input = std::string { R"(RUNSPEC
DIMENS
  10 10 10 /
START
  6 JUN 2020 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /

SOLUTION
RPTSOL
  'RESTART=2' 'FIP=3' 'FIPRESV' 'THPRES' /
SCHEDULE
RPTRST
  'BASIC=5' 'FREQ=6' 'CONV=10' /
--SCHEDULE
DATES
  7 'JLY' 2020 /          ( 1)
 10 'JLY' 2020 /          ( 2)
 20 'JLY' 2020 /          ( 3)
 30 'JLY' 2020 /          ( 4)
  5 'AUG' 2020 /          ( 5)
 20 'AUG' 2020 /          ( 6)
  5 'SEP' 2020 /          ( 7)
  1 'OCT' 2020 /          ( 8)
  1 'NOV' 2020 /          ( 9)
  1 'DEC' 2020 / -- WRITE (10)
  5 'JAN' 2021 /          (11)
  1 'FEB' 2021 /          (12)
 17 'MAY' 2021 /          (13)
  6 'JLY' 2021 / -- WRITE (14)
  1 'DEC' 2021 /          (15)
 31 'DEC' 2021 /          (16)
 21 'JAN' 2022 / -- WRITE (17)
 31 'JAN' 2022 /          (18)
/
END
)" };

    auto sched = make_schedule(input, false);

    for (const std::size_t stepID : { 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 15, 16, 18 }) {
        BOOST_CHECK_MESSAGE(! sched.write_rst_file(stepID),
                            "Must not write restart information for excluded step " << stepID);
    }

    for (const std::size_t stepID : { 0, 10, 14, 17 }) {
        BOOST_CHECK_MESSAGE(sched.write_rst_file(stepID),
                            "Must write restart information for included step " << stepID);
    }

    const auto expected = std::vector<std::tuple<bool, bool, TimeStampUTC>> {
        {true , true , TimeStampUTC(2020,  6,  6)},    //  0
        {true , false, TimeStampUTC(2020,  7,  7)},    //  1
        {false, false, TimeStampUTC(2020,  7, 10)},    //  2
        {false, false, TimeStampUTC(2020,  7, 20)},    //  3
        {false, false, TimeStampUTC(2020,  7, 30)},    //  4
        {true , false, TimeStampUTC(2020,  8,  5)},    //  5
        {false, false, TimeStampUTC(2020,  8, 20)},    //  6
        {true , false, TimeStampUTC(2020,  9,  5)},    //  7
        {true , false, TimeStampUTC(2020, 10,  1)},    //  8
        {true , false, TimeStampUTC(2020, 11,  1)},    //  9
        {true , false, TimeStampUTC(2020, 12,  1)},    // 10
        {true , true , TimeStampUTC(2021,  1,  5)},    // 11
        {true , false, TimeStampUTC(2021,  2,  1)},    // 12
        {true , false, TimeStampUTC(2021,  5, 17)},    // 13
        {true , false, TimeStampUTC(2021,  7,  6)},    // 14
        {true , false, TimeStampUTC(2021, 12,  1)},    // 15
        {false, false, TimeStampUTC(2021, 12, 31)},    // 16
        {true,  true , TimeStampUTC(2022,  1, 21)},    // 17
        {false, false, TimeStampUTC(2022,  1, 31)},    // 18
    };

    for (std::size_t index = 0; index < sched.size(); ++index) {
        const auto& state = sched[index];
        const auto& [first_in_month, first_in_year, ts] = expected[index];

        BOOST_CHECK_EQUAL( state.month_num(), ts.month() - 1);
        BOOST_CHECK_EQUAL( state.first_in_month(), first_in_month );
        BOOST_CHECK_EQUAL( state.first_in_year(), first_in_year );
        BOOST_CHECK_MESSAGE( ts == TimeStampUTC( TimeService::to_time_t(state.start_time() )),
                             "Time stamp does not match expected");
    }
}


BOOST_AUTO_TEST_CASE(RPTRST_AND_RPTSOL_SOLUTION2)
{
    const auto input = std::string { R"(RUNSPEC
DIMENS
  10 10 10 /
START
  6 JLY 2019 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /

SOLUTION
-- basic = 5, every month
RPTRST
 'BASIC=5'  'FREQ=6'   'CONV=10' /

RPTSOL
 'RESTART=2'  'FIP=3'  'FIPRESV'  'THPRES' /

SCHEDULE

DATES
  1 AUG 2019 /        ( 1)
  2 AUG 2019 /        ( 2)
  3 AUG 2019 /        ( 3)
  4 AUG 2019 /        ( 4)
 12 AUG 2019 /        ( 5)
 13 AUG 2019 /        ( 6)
 14 AUG 2019 /        ( 7)
 22 AUG 2019 /        ( 8)
 23 AUG 2019 /        ( 9)
 24 AUG 2019 /        (10)
  1 SEP 2019 /        (11)
 11 SEP 2019 /        (12)
 21 SEP 2019 /        (13)
 22 SEP 2019 /        (14)
 23 SEP 2019 /        (15)
  1 OCT 2019 /        (16)
  2 OCT 2019 /        (17)
  3 OCT 2019 /        (18)
 11 OCT 2019 /        (19)
 12 OCT 2019 /        (20)
 13 OCT 2019 /        (21)
 21 OCT 2019 /        (22)
 31 OCT 2019 /        (23)
  1 NOV 2019 /        (24)
  2 NOV 2019 /        (25)
 10 NOV 2019 /        (26)
 20 NOV 2019 /        (27)
 21 NOV 2019 /        (28)
 22 NOV 2019 /        (29)
 30 NOV 2019 /        (30)
  1 DEC 2019 /        (31)
 11 DEC 2019 /        (32)
 12 DEC 2019 /        (33)
 13 DEC 2019 /        (34)
 22 DEC 2019 /        (35)
 23 DEC 2019 /        (36)
 24 DEC 2019 /        (37)
  1 JAN 2020 / Write  (38)
  2 JAN 2020 /        (39)
 12 JAN 2020 /        (40)
 13 JAN 2020 /        (41)
 14 JAN 2020 /        (42)
 23 JAN 2020 /        (43)
 24 JAN 2020 /        (44)
 25 JAN 2020 /        (45)
  1 FEB 2020 /        (46)
  1 MAR 2020 /        (47)
  1 APR 2020 /        (48)
  1 MAY 2020 /        (49)
  1 JUN 2020 /        (50)
  1 JUL 2020 / Write  (51)
  1 AUG 2020 /        (52)
  1 SEP 2020 /        (53)
  1 OCT 2020 /        (54)
  1 NOV 2020 /        (55)
/

END
)" };

    auto sched = make_schedule(input, false);

    for (std::size_t step = 0; step < sched.size(); ++step) {
        if (step == 0 || step == 38 || step == 51)
            BOOST_CHECK_MESSAGE( sched.write_rst_file(step),
                                 "Restart file expected for step: " << step );
        else
            BOOST_CHECK_MESSAGE( !sched.write_rst_file(step),
                                 "Should *not* have restart file for step: " << step);
    }
}


BOOST_AUTO_TEST_CASE(RPTSCHED_INTEGER) {

    const std::string deckData1 = R"(
RUNSPEC
START             -- 0
19 JUN 2007 /
DIMENS
 10 10 10 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /
SOLUTION
RPTRST  -- PRES,DEN,PCOW,PCOG,RK,VELOCITY,COMPRESS
  6*0 1 0 1 9*0 1 7*0 1 0 3*1 / -- Static

SCHEDULE
-- 0
DATES             -- 1
 10  OKT 2008 /
/
RPTSCHED   -- 1
RESTART=1
/
DATES             -- 2
 20  JAN 2010 /
/
RPTRST  -- RK,VELOCITY,COMPRESS --2
  18*0 0 8*0 /
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED   -- 3
RESTART=0
/
)";

    auto sched = make_schedule(deckData1, false);

    BOOST_CHECK_MESSAGE( sched.write_rst_file( 0 ),
                         "Must write restart for report step 0" );

    // By comparison with ECLIPSE it turns out that we should write a restart
    // file for report step 1, there is clearly a bug in the old RPTRST
    // implementation which has been unnoticed for years. While reimplementing
    // the schedule implementation to use a snapshots based approach the test is
    // switched to require a restart file for report step 0 - and temporarily
    // disabled.
    //
    // BOOST_CHECK_MESSAGE( sched.write_rst_file( 1 ), "Must write restart for report step 1" );
    BOOST_CHECK_MESSAGE(  sched.write_rst_file( 2 ), "Must write restart for report step 2" );
    BOOST_CHECK_MESSAGE( !sched.write_rst_file( 3 ), "Must NOT write restart for report step 3" );

    const auto& kw_list1 = filter_keywords(sched.rst_keywords(1));
    const auto expected1 = {"BG","BO","BW","COMPRESS","DEN","KRG","KRO","KRW","PCOG","PCOW","PRES","RK","VELOCITY","VGAS","VOIL","VWAT"};
    BOOST_CHECK_EQUAL_COLLECTIONS( expected1.begin(), expected1.end(),
                                   kw_list1.begin(), kw_list1.end() );

    const auto& kw_list2 = filter_keywords( sched.rst_keywords(3));
    const auto expected2 = { "COMPRESS", "RK", "VELOCITY" };
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );
}

BOOST_AUTO_TEST_CASE(RPTRST_mixed_mnemonics_int_list) {
    const char* data = R"(
RUNSPEC
DIMENS
 10 10 10 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /

START             -- 0
19 JUN 2007 /
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 0 1 2
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
BASIC=1
/
)";

    ParseContext parseContext;
    ErrorGuard errors;
    auto deck = Parser().parseString( data);
    EclipseState es(deck);
    parseContext.update(ParseContext::RPT_MIXED_STYLE, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( Schedule( deck, es, parseContext, errors, {} ), std::exception );
}

BOOST_AUTO_TEST_CASE(RPTRST) {

    const std::string deckData1 = R"(
RUNSPEC
START             -- 0
19 JUN 2007 /
DIMENS
 10 10 10 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /
SOLUTION
RPTRST
 ACIP KRG KRO KRW NORST SFREQ=10 ALLPROPS/
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=1
/
DATES             -- 2
 20  JAN 2010 /
/
)";

    const std::string deckData2 = R"(
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 FREQ=2 FLOWS RUBBISH=5
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  JAN 2011 /
/
)";

    const char* deckData3 = R"(
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
3 0 0 0 0 2
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  JAN 2011 /
/
)";

    auto sched1 = make_schedule(deckData1, false);

    // Observe that this is true due to some undocumented guessing that
    // the initial restart file should be written if a RPTRST keyword is
    // found in the SOLUTION section, irrespective of the content of that
    // keyword.
    BOOST_CHECK(  sched1.write_rst_file( 0 ) );
    BOOST_CHECK( !sched1.write_rst_file( 1 ) );
    BOOST_CHECK(  sched1.write_rst_file( 2 ) );


    std::vector<std::string> expected = { "ACIP","BG","BO","BW","DEN","KRG", "KRO", "KRW", "NORST", "SFREQ", "VGAS", "VOIL", "VWAT"};
    const auto kw_list = filter_keywords( sched1.rst_keywords(2) );

    BOOST_CHECK_EQUAL_COLLECTIONS( expected.begin() ,expected.end(),
                                   kw_list.begin() , kw_list.end() );

    auto sched2 = make_schedule(deckData2);
    const auto expected2 = { "FLOWS" };
    const auto kw_list2 = filter_keywords( sched2.rst_keywords( 2 ) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );

    BOOST_CHECK( !sched2.write_rst_file( 0 ) );
    BOOST_CHECK( !sched2.write_rst_file( 1 ) );
    BOOST_CHECK(  sched2.write_rst_file( 2 ) );
    BOOST_CHECK( !sched2.write_rst_file( 3 ) );

    auto sched3 = make_schedule(deckData3);
    BOOST_CHECK( !sched3.write_rst_file( 0 ) );
    BOOST_CHECK( !sched3.write_rst_file( 1 ) );
    BOOST_CHECK(  sched3.write_rst_file( 2 ) );
    BOOST_CHECK( !sched3.write_rst_file( 3 ) );
}



BOOST_AUTO_TEST_CASE(RPTRST_FORMAT_ERROR) {

    const std::string deckData0 = R"(
RUNSPEC
START             -- 0
19 JUN 2007 /
DIMENS
 10 10 10 /
GRID
DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /
SOLUTION
RPTRST
 ACIP KRG KRO KRW NORST SFREQ=10 ALLPROPS/
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC 1
/
DATES             -- 2
 20  JAN 2010 /
/
)";

    const std::string deckData1 = R"(
RUNSPEC
START             -- 0
19 JUN 2007 /
DIMENS
 10 10 10 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /
SOLUTION
RPTRST
 ACIP KRG KRO KRW NORST SFREQ = 10 ALLPROPS/
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC = 1
/
DATES             -- 2
 20  JAN 2010 /
/
)";

    const std::string deckData2 = R"(
RUNSPEC
START             -- 0
19 JUN 2007 /
DIMENS
 10 10 10 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC = 3 FREQ = 2 FLOWS RUBBISH = 5
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  JAN 2011 /
/
)";

    const std::string deckData3 = R"(
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
3 0 0 0 0 2
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  JAN 2011 /
/
)";

    Opm::Parser parser;
    ParseContext ctx;
    ErrorGuard errors;

    auto deck0 = parser.parseString( deckData0, ctx, errors );
    auto deck1 = parser.parseString( deckData1, ctx, errors );
    auto deck2 = parser.parseString( deckData2, ctx, errors );
    EclipseState es0(deck0);
    EclipseState es1(deck1);
    EclipseState es2(deck2);

    ctx.update(ParseContext::RPT_UNKNOWN_MNEMONIC, InputError::IGNORE);
    ctx.update(ParseContext::RPT_MIXED_STYLE, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( Schedule( deck1, es1, ctx, errors, {} ), std::exception );


    ctx.update(ParseContext::RPT_MIXED_STYLE, InputError::IGNORE);
    Schedule sched1(deck1, es1, ctx, errors, {});


    // The case "BASIC 1" - i.e. without '=' can not be salvaged; this should
    // give an exception whatever is the value of ParseContext::RPT_MIXED_STYLE:
    BOOST_CHECK_THROW( Schedule( deck0, es0, ctx, errors, {} ), std::exception );


    // Observe that this is true due to some undocumented guessing that
    // the initial restart file should be written if a RPTRST keyword is
    // found in the SOLUTION section, irrespective of the content of that
    // keyword.
    BOOST_CHECK(  sched1.write_rst_file( 0 ) );
    BOOST_CHECK( !sched1.write_rst_file( 1 ) );
    BOOST_CHECK(  sched1.write_rst_file( 2 ) );


    std::vector<std::string> expected = { "ACIP","BG","BO","BW","DEN","KRG", "KRO", "KRW", "NORST", "SFREQ", "VGAS", "VOIL", "VWAT"};
    const auto kw_list = filter_keywords( sched1.rst_keywords(2) );

    BOOST_CHECK_EQUAL_COLLECTIONS( expected.begin() ,expected.end(),
                                   kw_list.begin() , kw_list.end() );

    ctx.update(ParseContext::RPT_UNKNOWN_MNEMONIC, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW( Schedule( deck2, es2, ctx, errors, {} ), std::exception );
    ctx.update(ParseContext::RPT_UNKNOWN_MNEMONIC, InputError::IGNORE);

    Schedule sched2(deck2, es2, ctx, errors, {});


    const auto expected2 = { "FLOWS"};
    const auto kw_list2 = filter_keywords( sched2.rst_keywords( 2 ) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );

    BOOST_CHECK( !sched2.write_rst_file( 0 ) );
    BOOST_CHECK( !sched2.write_rst_file( 1 ) );
    BOOST_CHECK(  sched2.write_rst_file( 2 ) );
    BOOST_CHECK( !sched2.write_rst_file( 3 ) );

    auto sched3 = make_schedule(deckData3);

    BOOST_CHECK( !sched3.write_rst_file( 0 ) );
    BOOST_CHECK( !sched3.write_rst_file( 1 ) );
    BOOST_CHECK(  sched3.write_rst_file( 2 ) );
    BOOST_CHECK( !sched3.write_rst_file( 3 ) );
}



BOOST_AUTO_TEST_CASE(RPTSCHED) {

    const std::string deckData1 = R"(
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTSCHED
RESTART=1
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
RESTART=0
/
)";


    const std::string deckData2 = R"(
RUNSPEC
DIMENS
 10 10 10 /
GRID
START             -- 0
19 JUN 2007 /
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTSCHED
RESTART=3 FIP
/
DATES             -- 2
 20  JAN 2010 /
/
RPTSCHED
RESTART=4
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
NOTHING RUBBISH
/
)";

    const std::string deckData3 = R"(
RUNSPEC
DIMENS
 10 10 10 /
GRID
START             -- 0
19 JUN 2007 /
SOLUTION
RPTSOL
  RESTART=4 /
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 FREQ=1 RUBBISH=5
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
0 0 0 0 0 0 0 0
/
)";

    auto sched1 = make_schedule(deckData1);
    BOOST_CHECK( !sched1.write_rst_file( 0 ) );
    BOOST_CHECK( !sched1.write_rst_file( 1 ) );
    BOOST_CHECK(  sched1.write_rst_file( 2 ) );
    BOOST_CHECK(  sched1.write_rst_file( 3 ) );


    auto sched2 = make_schedule(deckData2);
    BOOST_CHECK( !sched2.write_rst_file( 0 ) );
    BOOST_CHECK( !sched2.write_rst_file( 1 ) );
    BOOST_CHECK(  sched2.write_rst_file( 2 ) );
    BOOST_CHECK(  sched2.write_rst_file( 3 ) );

    const auto expected2 = { "FIP"};
    const auto kw_list2 = filter_keywords( sched2.rst_keywords( 2 ) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );


    auto sched3 = make_schedule(deckData3);
    //Older ECLIPSE 100 data set may use integer controls instead of mnemonics
    BOOST_CHECK(  sched3.write_rst_file( 0 ) );
    BOOST_CHECK( !sched3.write_rst_file( 1 ) );
    BOOST_CHECK(  sched3.write_rst_file( 2 ) );
    BOOST_CHECK(  sched3.write_rst_file( 3 ) );

    std::vector<std::string> expected3 = {};
    const auto kw_list3 = filter_keywords( sched3.rst_keywords(2) );
    BOOST_CHECK_EQUAL_COLLECTIONS( expected3.begin() , expected3.end() , kw_list3.begin() , kw_list3.end() );
}


BOOST_AUTO_TEST_CASE(RPTSCHED_and_RPTRST) {
    const std::string deckData = R"(
RUNSPEC
DIMENS
 10 10 10 /
GRID
START             -- 0
19 JUN 2007 /
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 FREQ=3 BG BO
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
RESTART=1
/
)";


    auto sched3 = make_schedule(deckData);

    BOOST_CHECK( !sched3.write_rst_file( 0 ) );
    BOOST_CHECK( !sched3.write_rst_file( 1 ) );
    BOOST_CHECK( !sched3.write_rst_file( 2 ) );
    BOOST_CHECK(  sched3.write_rst_file( 3 ) );
}


BOOST_AUTO_TEST_CASE(NO_BASIC) {
    const std::string data = R"(
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
/
)";

    auto sched = make_schedule(data);
    for( size_t ts = 0; ts < 4; ++ts )
        BOOST_CHECK( !sched.write_rst_file( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_1) {
    const std::string data = R"(
SCHEDULE
DATES             -- 1
 10  OKT 2008 /
/
RPTRST
BASIC=3 FREQ=3
/
DATES             -- 2
 20  JAN 2010 /
/
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
BASIC=1
/
)";

    auto sched = make_schedule(data);
    for( size_t ts = 0; ts < 3; ++ts )
        BOOST_CHECK( !sched.write_rst_file( ts ) );

    BOOST_CHECK( sched.write_rst_file( 3 ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_3) {
    const std::string data =  R"(
SCHEDULE
RPTRST
BASIC=3 FREQ=3
/
DATES
 22 MAY 1981 /              -- timestep 1
 23 MAY 1981 /              -- timestep 2
 24 MAY 1981 /              -- timestep 3
 25 MAY 1981 /              -- timestep 4
 26 MAY 1981 /              -- timestep 5
 1 JAN 1982 /               -- timestep 6
 1 JAN 1982 13:55:44 /      -- timestep 7
 3 JAN 1982 14:56:45.123 /  -- timestep 8
 4 JAN 1982 14:56:45.123 /  -- timestep 9
 5 JAN 1982 14:56:45.123 /  -- timestep 10
 6 JAN 1982 14:56:45.123 /  -- timestep 11
/
)";

    auto sched = make_schedule(data);
    const std::size_t freq = 3;

    /* BASIC=3, restart files are created every nth report time, n=3 */
    for (std::size_t ts = 1; ts < 12; ++ts)
        BOOST_CHECK_EQUAL( ts % freq == 0, sched.write_rst_file( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_4) {
    const std::string data = R"(
SCHEDULE
RPTRST
BASIC=4
/
DATES
 22 MAY 1981 /              -- timestep 1
 23 MAY 1981 /              -- timestep 2
 24 MAY 1981 /              -- timestep 3
 25 MAY 1981 /              -- timestep 4
 26 MAY 1981 /              -- timestep 5
 1 JAN 1982 /               -- timestep 6
 1 JAN 1982 13:55:44 /      -- timestep 7
 3 JAN 1982 14:56:45.123 /  -- timestep 8
 4 JAN 1982 14:56:45.123 /  -- timestep 9
 5 JAN 1982 14:56:45.123 /  -- timestep 10
 6 JAN 1982 14:56:45.123 /  -- timestep 11
 6 JAN 1983 14:56:45.123 /  -- timestep 12
/
)";

    auto sched = make_schedule(data);

    /* BASIC=4, restart file is written at the first report step of each year.
     */
    for (std::size_t ts : { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11 })
        BOOST_CHECK(!sched.write_rst_file( ts ) );

    for (std::size_t ts : { 6, 12 })
        BOOST_CHECK( sched.write_rst_file( ts ) );
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_4_FREQ_2) {
    const std::string data = R"(
SCHEDULE
RPTRST
BASIC=4 FREQ=2
/
DATES
 22 MAY 1981 / --  1
 23 MAY 1981 / --  2
 24 MAY 1981 / --  3
 23 MAY 1982 / --  4
 24 MAY 1982 / --  5
 24 MAY 1983 / --  6 write
 25 MAY 1984 / --  7
 26 MAY 1984 / --  8
 26 MAY 1985 / --  9 write
 27 MAY 1985 / -- 10
  1 JAN 1986 / -- 11
/
)";
    auto sched = make_schedule(data);

    /* BASIC=4, restart file is written at the first report step of each year.
     * Optionally, if the mnemonic FREQ is set >1 the restart is written only
     * every n'th year.
     *
     * FREQ=2
     */
    for (const std::size_t ts : { 1, 2, 3, 4, 5, 7, 8, 10, 11 } )
        BOOST_CHECK_MESSAGE( !sched.write_rst_file( ts ),
                             "Must NOT write restart file for excluded step " << ts);

    for (const std::size_t ts : { 6, 9 } )
        BOOST_CHECK_MESSAGE( sched.write_rst_file( ts ),
                             "Must write restart file for included step " << ts);
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_5) {
    const std::string data =  R"(
START
1 MAY 1981 /
SCHEDULE
RPTRST
BASIC=5 FREQ=2
/
DATES
 22 MAY 1981 / -- 1
 23 MAY 1981 / -- 2
 24 MAY 1981 / -- 3
  1 JUN 1981 / -- 4
  1 JUL 1981 / -- 5  Write
  1 JAN 1982 / -- 6  Write
  2 JAN 1982 / -- 7
  1 FEB 1982 / -- 8
  1 MAR 1982 / -- 9  Write
  1 APR 1983 / --10  Write
 16 APR 1983 / --11
 30 APR 1983 / --12
  1 MAY 1983 / --13
 17 MAY 1983 / --14
 31 MAY 1983 / --15
  2 JUN 1983 / --16  Write
 10 JUN 1983 / --17
 23 JUN 1983 / --18
 30 JUN 1983 / --19
 21 JUL 1983 / --20
 31 JUL 1983 / --21
  5 AUG 1983 / --22  Write
 22 AUG 1983 / --23
/
)";

    auto sched = make_schedule(data);
    /* BASIC=5, restart file is written at the first report step of each month.
     */
    for (std::size_t ts : { 0, 1, 2, 3, 4, 7, 8, 11, 12, 13, 14, 15, 17, 18, 19, 20, 21, 23 }) {
        BOOST_CHECK_MESSAGE( !sched.write_rst_file(ts),
                             "Must not write restart file for excluded step " << ts );
    }

    for (std::size_t ts : { 5, 6, 9, 10, 16, 22 }) {
        BOOST_CHECK_MESSAGE( sched.write_rst_file(ts),
                             "Restart file expected for step: " << ts );
    }
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_5_FREQ_EQ_6)
{
    const auto deckStr =  std::string { R"(RUNSPEC
DIMENS
1 1 1/

START
 1 JAN 2000 /

GRID
DX
1 /
DY
1 /
DZ
1 /
TOPS
0 /

SOLUTION

RPTRST
BASIC=5 FREQ=6 /

SCHEDULE
DATES
  2 JAN 2000 / --  1
  3 JAN 2000 / --  2
  9 JAN 2000 / --  3
  6 FEB 2000 / --  4
 11 AUG 2000 / --  5 Write
  3 SEP 2000 / --  6
 24 SEP 2000 / --  7
 22 DEC 2000 / --  8
 31 DEC 2000 / --  9
  1 JAN 2001 / -- 10
  2 JAN 2001 / -- 11
 30 JAN 2001 / -- 12
 31 JAN 2001 / -- 13
  1 FEB 2001 / -- 14 Write
  2 FEB 2001 / -- 15
 28 FEB 2001 / -- 16
  1 MAR 2001 / -- 17
  2 MAR 2001 / -- 18
  3 MAR 2001 / -- 19
 31 MAR 2001 / -- 20
  1 APR 2001 / -- 21
 30 APR 2001 / -- 22
  1 MAY 2001 / -- 23
 31 MAY 2001 / -- 24
  1 JUN 2001 / -- 25
 30 JUN 2001 / -- 26
  1 JLY 2001 / -- 27
  2 JLY 2001 / -- 28
 30 JLY 2001 / -- 29
 31 JLY 2001 / -- 30
  1 AUG 2001 / -- 31 Write
 31 AUG 2001 / -- 32
 10 SEP 2001 / -- 33
 17 OCT 2001 / -- 34
  7 NOV 2001 / -- 35
  8 NOV 2001 / -- 36
 27 NOV 2001 / -- 37
 29 NOV 2001 / -- 38
 30 NOV 2001 / -- 39
  6 DEC 2001 / -- 40
  7 DEC 2001 / -- 41
 30 DEC 2001 / -- 42
 31 DEC 2001 / -- 43
  1 JAN 2002 / -- 44
  2 JAN 2002 / -- 45
 31 JAN 2002 / -- 46
  1 FEB 2002 / -- 47 Write
  2 FEB 2002 / -- 48
 19 JUN 2002 / -- 49
 19 JLY 2002 / -- 50
 31 JLY 2002 / -- 51
  1 AUG 2002 / -- 52 Write
/
END
)" };

    auto sched = make_schedule(deckStr, false);

    BOOST_CHECK_MESSAGE( sched.write_rst_file( 0), "Must write restart file at report step 0");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 1), "Must NOT write restart file at report step 1");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 2), "Must NOT write restart file at report step 2");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 3), "Must NOT write restart file at report step 3");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 4), "Must NOT write restart file at report step 4");
    BOOST_CHECK_MESSAGE( sched.write_rst_file( 5), "Must write restart file at report step 5");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 6), "Must NOT write restart file at report step 6");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 7), "Must NOT write restart file at report step 7");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 8), "Must NOT write restart file at report step 8");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file( 9), "Must NOT write restart file at report step 9");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(10), "Must NOT write restart file at report step 10");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(11), "Must NOT write restart file at report step 11");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(12), "Must NOT write restart file at report step 12");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(13), "Must NOT write restart file at report step 13");
    BOOST_CHECK_MESSAGE( sched.write_rst_file(14), "Must write restart file at report step 14");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(15), "Must NOT write restart file at report step 15");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(16), "Must NOT write restart file at report step 16");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(17), "Must NOT write restart file at report step 17");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(18), "Must NOT write restart file at report step 18");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(19), "Must NOT write restart file at report step 19");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(20), "Must NOT write restart file at report step 20");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(21), "Must NOT write restart file at report step 21");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(22), "Must NOT write restart file at report step 22");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(23), "Must NOT write restart file at report step 23");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(24), "Must NOT write restart file at report step 24");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(25), "Must NOT write restart file at report step 25");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(26), "Must NOT write restart file at report step 26");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(27), "Must NOT write restart file at report step 27");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(28), "Must NOT write restart file at report step 28");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(29), "Must NOT write restart file at report step 29");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(30), "Must NOT write restart file at report step 30");
    BOOST_CHECK_MESSAGE( sched.write_rst_file(31), "Must write restart file at report step 31");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(32), "Must NOT write restart file at report step 32");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(33), "Must NOT write restart file at report step 33");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(34), "Must NOT write restart file at report step 34");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(35), "Must NOT write restart file at report step 35");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(36), "Must NOT write restart file at report step 36");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(37), "Must NOT write restart file at report step 37");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(38), "Must NOT write restart file at report step 38");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(39), "Must NOT write restart file at report step 39");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(40), "Must NOT write restart file at report step 40");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(41), "Must NOT write restart file at report step 41");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(42), "Must NOT write restart file at report step 42");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(43), "Must NOT write restart file at report step 43");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(44), "Must NOT write restart file at report step 44");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(45), "Must NOT write restart file at report step 45");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(46), "Must NOT write restart file at report step 46");
    BOOST_CHECK_MESSAGE( sched.write_rst_file(47), "Must write restart file at report step 47");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(48), "Must NOT write restart file at report step 48");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(49), "Must NOT write restart file at report step 49");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(50), "Must NOT write restart file at report step 50");
    BOOST_CHECK_MESSAGE(!sched.write_rst_file(51), "Must NOT write restart file at report step 51");
    BOOST_CHECK_MESSAGE( sched.write_rst_file(52), "Must write restart file at report step 52");
}

BOOST_AUTO_TEST_CASE(BASIC_EQ_0) {
    const std::string data =  R"(
SCHEDULE
RPTRST
BASIC=0 FREQ=2
/
DATES
 22 MAY 1981 /
 23 MAY 1981 /
 24 MAY 1981 /
  1 JUN 1981 /
  1 JUL 1981 /
  1 JAN 1982 /
  2 JAN 1982 /
  1 FEB 1982 /
  1 MAR 1982 /
  1 APR 1983 /
  2 JUN 1983 /
/
)";

    auto sched = make_schedule(data);
    /* RESTART=0, no restart file is written
     */
    for (std::size_t ts = 0; ts < 11; ++ts)
        BOOST_CHECK( !sched.write_rst_file( ts ) );
}


BOOST_AUTO_TEST_CASE(RESTART_EQ_0) {
    const std::string data =  R"(
RUNSPEC
DIMENS
 10 10 10 /
START
 21 MAY 1981 /

GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /

SCHEDULE
RPTSCHED
RESTART=0
/
DATES
 22 MAY 1981 /
 23 MAY 1981 /
 24 MAY 1981 /
  1 JUN 1981 /
  1 JUL 1981 /
  1 JAN 1982 /
  2 JAN 1982 /
  1 FEB 1982 /
  1 MAR 1982 /
  1 APR 1983 /
  2 JUN 1983 /
/
)";

    /* RESTART=0, no restart file is written
     */
    auto sched = make_schedule(data, false);
    for (std::size_t ts = 0; ts < 11; ++ts)
        BOOST_CHECK( !sched.write_rst_file( ts ) );
}

BOOST_AUTO_TEST_CASE(RESTART_BASIC_GT_2) {
    const std::string data =  R"(
SCHEDULE
RPTRST
BASIC=4 FREQ=2
/
DATES
 22 MAY 1981 /
/
RPTSCHED -- BASIC >2, ignore RPTSCHED RESTART
RESTART=3, FREQ=1
/
DATES
 23 MAY 1981 /
 24 MAY 1981 /
 23 MAY 1982 /
 24 MAY 1982 /
 24 MAY 1983 / -- write
 25 MAY 1984 /
 26 MAY 1984 /
 26 MAY 1985 / -- write
 27 MAY 1985 /
 1 JAN 1986 /
/
)";

    auto sched = make_schedule(data);
    for (std::size_t ts : { 1, 2, 3, 4, 5, 7, 8, 10, 11 })
        BOOST_CHECK( !sched.write_rst_file( ts ) );

    for (std::size_t ts : { 6, 9 })
        BOOST_CHECK( sched.write_rst_file( ts ) );
}

BOOST_AUTO_TEST_CASE(RESTART_BASIC_LEQ_2) {
    const std::string data = R"(
SCHEDULE
RPTRST
BASIC=1"
/
DATES
 22 MAY 1981 /
/
RPTSCHED
RESTART=0
/
DATES
 23 MAY 1981 /
 24 MAY 1981 /
 23 MAY 1982 /
 24 MAY 1982 /
 24 MAY 1983 /
 25 MAY 1984 /
 26 MAY 1984 /
 26 MAY 1985 /
 27 MAY 1985 /
 1 JAN 1986 /
/
)";

    auto sched = make_schedule(data);
    BOOST_CHECK_MESSAGE( sched.write_rst_file( 1 ), "Must write restart for report step 1" );
    for (std::size_t ts = 2; ts < 11; ++ts)
        BOOST_CHECK_MESSAGE( !sched.write_rst_file( ts ),
                             "Must NOT write restart for report step " << ts );
}

BOOST_AUTO_TEST_CASE(RESTART_SAVE) {
    const std::string data = R"(

SCHEDULE
DATES
 22 MAY 1981 /
/
DATES
 23 MAY 1981 /
 24 MAY 1981 /
 23 MAY 1982 /
 24 MAY 1982 /
 24 MAY 1983 /
 25 MAY 1984 /
 26 MAY 1984 /
 26 MAY 1985 /
 27 MAY 1985 /
 1 JAN 1986 /
/
SAVE
TSTEP
 1 /
)";
    auto sched = make_schedule(data);
    for (std::size_t ts = 1; ts < 11; ++ts)
        BOOST_CHECK_MESSAGE( !sched.write_rst_file(ts),
                             "Must NOT write restart for report step " << ts);

    BOOST_CHECK_MESSAGE( sched.write_rst_file(11),
                         "Must write restart for report step 11");

    BOOST_CHECK_MESSAGE( !sched.write_rst_file( 12 ),
                         "Must NOT write restart for report step 12");
}


BOOST_AUTO_TEST_CASE(RPTSCHED_INTEGER2) {

    const auto deckData1 = std::string { R"(
RUNSPEC
START             -- 0
19 JUN 2007 /
DIMENS
 10 10 10 /
GRID

DXV
  10*1 /

DYV
  10*1 /

DZV
  10*1 /

DEPTHZ
  121*1 /

PORO
  1000*0.25 /
SOLUTION
RPTRST  -- PRES,DEN,PCOW,PCOG,RK,VELOCITY,COMPRESS
  1 5*0 1 0 1 9*0 1 7*0 1 0 3*1 / -- Static

SCHEDULE
-- 0
DATES             -- 1
 10  OKT 2008 /
/
RPTSCHED
RESTART=1
/
DATES             -- 2
 20  JAN 2010 /
/
RPTRST  -- RK,VELOCITY,COMPRESS
  18*0 0 8*0 /
DATES             -- 3
 20  FEB 2010 /
/
RPTSCHED
RESTART=0
/

DATES       -- 4
1 MAR 2010 /
/
)" };

    auto sched = make_schedule(deckData1, false);

    BOOST_CHECK_EQUAL( sched.size(), 5);
    BOOST_CHECK(  sched.write_rst_file( 0 ) );
    BOOST_CHECK(  sched.write_rst_file( 1 ) );
    BOOST_CHECK(  sched.write_rst_file( 2 ) );
    BOOST_CHECK( !sched.write_rst_file( 3 ) );

    const auto& kw_list1 = filter_keywords(sched.rst_keywords(1));
    const auto expected1 = {"BG","BO","BW","COMPRESS","DEN","KRG","KRO","KRW","PCOG","PCOW","PRES","RK","VELOCITY","VGAS","VOIL","VWAT"};
    BOOST_CHECK_EQUAL_COLLECTIONS( expected1.begin(), expected1.end(),
                                   kw_list1.begin(), kw_list1.end() );

    const auto& kw_list2 = filter_keywords( sched.rst_keywords(3));
    const auto expected2 = { "COMPRESS", "RK", "VELOCITY" };
    BOOST_CHECK_EQUAL_COLLECTIONS( expected2.begin(), expected2.end(),
                                   kw_list2.begin(), kw_list2.end() );
}
