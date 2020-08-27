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

#define BOOST_TEST_MODULE TimeMapTests

#include <boost/test/unit_test.hpp>

#include <opm/common/utility/TimeService.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>


const std::time_t startDateJan1st2010 = Opm::TimeMap::mkdate(2010, 1, 1);

Opm::DeckRecord createDeckRecord(int day, const std::string &month, int year, const std::string &time = "00:00:00.000");

BOOST_AUTO_TEST_CASE(GetStartDate) {
    Opm::TimeMap timeMap({ startDateJan1st2010 });
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2010, 1, 1) , timeMap.getStartTime(/*timeStepIdx=*/0));
}



BOOST_AUTO_TEST_CASE(AddDateNegativeStepThrows) {
    std::vector<std::time_t> time_points = { Opm::asTimeT(Opm::TimeStampUTC(2000,1,1)), Opm::asTimeT(Opm::TimeStampUTC(1999,1,1))};
    BOOST_CHECK_THROW(Opm::TimeMap timeMap(time_points), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(AddStepSizeCorrect) {
    std::vector<std::time_t> time_points = { Opm::asTimeT(Opm::TimeStampUTC(2010,1,1)),
                                             Opm::asTimeT(Opm::TimeStampUTC(2010,1,2)),
                                             Opm::asTimeT(Opm::TimeStampUTC(2010,1,3))};
    Opm::TimeMap timeMap(time_points);
    BOOST_CHECK_EQUAL(3U, timeMap.size());

    BOOST_CHECK_THROW(timeMap[3] , std::invalid_argument );
    BOOST_CHECK_EQUAL(timeMap[0] , Opm::TimeMap::mkdate(2010, 1, 1 ));
    BOOST_CHECK_EQUAL(timeMap[2] , Opm::TimeMap::mkdate(2010, 1, 3 ));
}


BOOST_AUTO_TEST_CASE( dateFromEclipseThrowsInvalidRecord ) {
    Opm::DeckRecord startRecord;
    Opm::DeckItem dayItem("DAY", int() );
    Opm::DeckItem monthItem("MONTH", std::string() );
    Opm::DeckItem yearItem("YEAR", int() );
    Opm::DeckItem timeItem("TIME", std::string() );
    Opm::DeckItem extraItem("EXTRA", int() );

    dayItem.push_back( 10 );
    yearItem.push_back(1987 );
    monthItem.push_back("FEB");
    timeItem.push_back("00:00:00.000");

    BOOST_CHECK_THROW( Opm::TimeMap::timeFromEclipse( startRecord ) , std::out_of_range );

    startRecord.addItem( dayItem );
    BOOST_CHECK_THROW( Opm::TimeMap::timeFromEclipse( startRecord ) , std::out_of_range );

    startRecord.addItem( monthItem );
    BOOST_CHECK_THROW( Opm::TimeMap::timeFromEclipse( startRecord ) , std::out_of_range );

    startRecord.addItem( yearItem );
    BOOST_CHECK_THROW(Opm::TimeMap::timeFromEclipse( startRecord ) , std::out_of_range );

    startRecord.addItem( timeItem );
    BOOST_CHECK_NO_THROW(Opm::TimeMap::timeFromEclipse( startRecord ));
}



BOOST_AUTO_TEST_CASE( dateFromEclipseInvalidMonthThrows ) {
    Opm::DeckRecord startRecord;
    Opm::DeckItem dayItem( "DAY", int() );
    Opm::DeckItem monthItem( "MONTH", std::string() );
    Opm::DeckItem yearItem( "YEAR", int() );
    Opm::DeckItem timeItem( "TIME", std::string() );

    dayItem.push_back( 10 );
    yearItem.push_back(1987 );
    monthItem.push_back("XXX");
    timeItem.push_back("00:00:00.000");

    startRecord.addItem( dayItem );
    startRecord.addItem( monthItem );
    startRecord.addItem( yearItem );
    startRecord.addItem( timeItem );

    BOOST_CHECK_THROW( Opm::TimeMap::timeFromEclipse( startRecord ) , std::out_of_range );
}


BOOST_AUTO_TEST_CASE( timeFromEclipseCheckMonthNames ) {
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 1, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "JAN", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 2, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "FEB", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 3, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "MAR", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 4, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "APR", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 5, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "MAI", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 5, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "MAY", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 6, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "JUN", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 7, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "JUL", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 7, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "JLY", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 8, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "AUG", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 9, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "SEP", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 10, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "OKT", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 10, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "OCT", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 11, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "NOV", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 12, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "DEC", 2000)));
    BOOST_CHECK_EQUAL( Opm::TimeMap::mkdate(2000, 12, 1), Opm::TimeMap::timeFromEclipse(createDeckRecord(1, "DES", 2000)));
}


BOOST_AUTO_TEST_CASE( timeFromEclipseInputRecord ) {
    Opm::DeckRecord  startRecord;
    Opm::DeckItem dayItem( "DAY", int() );
    Opm::DeckItem monthItem( "MONTH", std::string() );
    Opm::DeckItem yearItem("YEAR", int());
    Opm::DeckItem timeItem("TIME", std::string() );

    dayItem.push_back( 10 );
    yearItem.push_back( 1987 );
    monthItem.push_back("JAN");
    timeItem.push_back("00:00:00.000");

    startRecord.addItem( std::move( dayItem ) );
    startRecord.addItem( std::move( monthItem ) );
    startRecord.addItem( std::move( yearItem ) );
    startRecord.addItem( std::move( timeItem ) );

    BOOST_CHECK_EQUAL(Opm::TimeMap::mkdate(1987, 1 , 10 ), Opm::TimeMap::timeFromEclipse( startRecord ));
}




BOOST_AUTO_TEST_CASE(TimeStepsCorrect) {
    const char *deckData =
        "START\n"
        " 21 MAY 1981 /\n"
        "\n"
        "SCHEDULE\n"
        "TSTEP\n"
        " 1 2 3 4 5 /\n"
        "\n"
        "DATES\n"
        " 1 JAN 1982 /\n"
        " 1 JAN 1982 13:55:44 /\n"
        " 3 JAN 1982 14:56:45.123 /\n"
        "/\n"
        "\n"
        "TSTEP\n"
        " 6 7 /\n";

    Opm::Parser parser( true );
    auto deck = parser.parseString(deckData);
    Opm::TimeMap tmap(deck);

    BOOST_CHECK_EQUAL(tmap.getStartTime(0),Opm::TimeMap::mkdate( 1981 , 5 , 21 ));
    BOOST_CHECK_EQUAL(tmap.getTimeStepLength(0), 1*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getTimePassedUntil(1), 1.0*24*60*60);


    BOOST_CHECK_EQUAL(tmap.getStartTime(1),
                      Opm::TimeMap::forward( Opm::TimeMap::mkdate( 1981 , 5 , 21 ) , 3600*24));

    BOOST_CHECK_EQUAL(tmap.getTimeStepLength(1), 2*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getTimePassedUntil(2), (1.0 + 2.0)*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getStartTime(2),
                      Opm::TimeMap::forward( Opm::TimeMap::mkdate( 1981 , 5 , 21 ) , 3*24*3600));

    BOOST_CHECK_EQUAL(tmap.getTimeStepLength(2), 3*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getTimePassedUntil(3), (1.0 + 2.0 + 3.0)*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getStartTime(3),
                      Opm::TimeMap::forward( Opm::TimeMap::mkdate( 1981 , 5 , 21 ) , 6*3600*24));

    BOOST_CHECK_EQUAL(tmap.getTimeStepLength(3), 4*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getTimePassedUntil(4), (1.0 + 2.0 + 3.0 + 4.0)*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getStartTime(4),
                      Opm::TimeMap::forward( Opm::TimeMap::mkdate( 1981 , 5 , 21 ) , 10*3600*24));

    BOOST_CHECK_EQUAL(tmap.getTimeStepLength(4), 5*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getTimePassedUntil(5), (1.0 + 2.0 + 3.0 + 4.0 + 5.0)*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getStartTime(5),
                      Opm::TimeMap::forward( Opm::TimeMap::mkdate( 1981 , 5 , 21 ) , 15*3600*24));

    // timestep 5 is the period between the last step specified using
    // of the TIMES keyword and the first record of DATES
    BOOST_CHECK_EQUAL(tmap.getStartTime(6),
                      Opm::TimeMap::mkdate( 1982 , 1 , 1 ));

    BOOST_CHECK_EQUAL(tmap.getStartTime(7),
                      Opm::TimeMap::forward( Opm::TimeMap::mkdate( 1982 , 1 , 1 ) , 13,55,44 ));

    BOOST_CHECK_EQUAL(tmap.getStartTime(8),
                      Opm::TimeMap::forward( Opm::TimeMap::mkdate( 1982 , 1 , 3 ) , 14,56,45));

    BOOST_CHECK_EQUAL(tmap.getTimeStepLength(8), 6*24*60*60);
    BOOST_CHECK_EQUAL(tmap.getTimeStepLength(9), 7*24*60*60);
    BOOST_CHECK(!tmap.skiprest());
}


BOOST_AUTO_TEST_CASE(initTimestepsYearsAndMonths) {
    const char *deckData =
        "START\n"
        " 21 MAY 1981 /\n"
        "\n"
        "SCHEDULE\n"
        "TSTEP\n"
        " 1 2 3 4 5 /\n"
        "\n"
        "DATES\n"
        " 5 JUL 1981 /\n"
        " 6 JUL 1981 /\n"
        " 5 AUG 1981 /\n"
        " 5 SEP 1981 /\n"
        " 1 OCT 1981 /\n"
        " 1 NOV 1981 /\n"
        " 1 DEC 1981 /\n"
        " 1 JAN 1982 /\n"
        " 1 JAN 1982 13:55:44 /\n"
        " 3 JAN 1982 14:56:45.123 /\n"
        "/\n"
        "\n"
        "TSTEP\n"
        " 6 7 /\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    const Opm::TimeMap tmap(deck);

    /*deckData timesteps:
    0   21 may  1981 START
    1   22 may  1981
    2   24 may  1981
    3   27 may  1981
    4   31 may  1981
    5   5  jun 1981
    6   5  jul 1981
    7   6  jul 1981
    8   5  aug 1981
    9   5  sep 1981
    10  1  oct 1981
    11  1  nov 1981
    12  1  dec 1981
    13  1  jan  1982
    14  1  jan  1982
    15  3  jan  1982
    16  9  jan  1982
    17  16 jan  1982*/

    for (size_t timestep = 0; timestep <= 17; ++timestep) {
        if ((5 == timestep) || (6 == timestep) || (8 == timestep) || (9 == timestep) ||
            (10 == timestep) || (11 == timestep) || (12 == timestep) || (13 == timestep)) {
            BOOST_CHECK_EQUAL(true, tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, false));
        } else {
            BOOST_CHECK_EQUAL(false, tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, false));
        }
    }

    for (size_t timestep = 0; timestep <= 17; ++timestep) {
        if (13 == timestep) {
            BOOST_CHECK_EQUAL(true, tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, true));
        } else {
            BOOST_CHECK_EQUAL(false, tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, true));
        }
    }
}

BOOST_AUTO_TEST_CASE(initTimestepsYearsAndMonthsSkippingMonthsFrequency) {
    const char *deckData =
        "START\n"
        " 21 MAY 1981 /\n"
        "\n"
        "SCHEDULE\n"
        "DATES\n"
        " 5 JUL 1981 /\n"
        " 6 JUL 1981 /\n"
        " 5 AUG 1981 /\n"
        " 5 SEP 1981 /\n"
        " 1 OCT 1981 /\n"
        " 1 NOV 1981 /\n"
        " 1 DEC 1981 /\n"
        " 1 JAN 1982 /\n"
        " 1 JAN 1982 13:55:44 /\n"
        " 3 JAN 1982 14:56:45.123 /\n"
        " 1 JAN 1983 /\n"
        " 1 JAN 1984 /\n"
        " 1 JAN 1985 /\n"
        " 1 JAN 1988 /\n"
        "/\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    const Opm::TimeMap tmap(deck);

    /*deckData timesteps:
    0  21  may 1981 START
    1   5  jul 1981
    2   6  jul 1981
    3   5  aug 1981
    4   5  sep 1981
    5   1  oct 1981
    6   1  nov 1981
    7   1  dec 1981
    8   1  jan 1982
    9   1  jan 1982
    10  3  jan 1982
    11  1  jan 1983
    12  1  jan 1984
    13  1  jan 1985
    14  1  jan 1988*/

    // Month, not set frequency.
    {
        std::vector<bool> expected = {
            false, // 0  21  may 1981 START
            true,  // 1   5  jul 1981
            false, // 2   6  jul 1981
            true,  // 3   5  aug 1981
            true,  // 4   5  sep 1981
            true,  // 5   1  oct 1981
            true,  // 6   1  nov 1981
            true,  // 7   1  dec 1981
            true,  // 8   1  jan 1982
            false, // 9   1  jan 1982
            false, // 10  3  jan 1982
            true,  // 11  1  jan 1983
            true,  // 12  1  jan 1984
            true,  // 13  1  jan 1985
            true   // 14  1  jan 1988
        };

        for (size_t timestep = 0; timestep < expected.size(); ++timestep) {
            const bool ok = tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, false) == expected[timestep];
            BOOST_CHECK_MESSAGE(ok, "failing for timestep " << timestep);
        }
    }

    // Month, frequency 2.
    {
        std::vector<bool> expected = {
            false, // 0  21  may 1981 START
            true,  // 1   5  jul 1981
            false, // 2   6  jul 1981
            false, // 3   5  aug 1981
            true,  // 4   5  sep 1981
            false, // 5   1  oct 1981
            true,  // 6   1  nov 1981
            false, // 7   1  dec 1981
            true,  // 8   1  jan 1982
            false, // 9   1  jan 1982
            false, // 10  3  jan 1982
            true,  // 11  1  jan 1983
            true,  // 12  1  jan 1984
            true,  // 13  1  jan 1985
            true   // 14  1  jan 1988
        };

        for (size_t timestep = 0; timestep < expected.size(); ++timestep) {
            const bool ok = tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, false, 1, 2) == expected[timestep];
            BOOST_CHECK_MESSAGE(ok, "failing for timestep " << timestep);
        }
    }

    // Year, not set frequency.
    {
        std::vector<bool> expected = {
            false, // 0  21  may 1981 START
            false, // 1   5  jul 1981
            false, // 2   6  jul 1981
            false, // 3   5  aug 1981
            false, // 4   5  sep 1981
            false, // 5   1  oct 1981
            false, // 6   1  nov 1981
            false, // 7   1  dec 1981
            true,  // 8   1  jan 1982
            false, // 9   1  jan 1982
            false, // 10  3  jan 1982
            true,  // 11  1  jan 1983
            true,  // 12  1  jan 1984
            true,  // 13  1  jan 1985
            true   // 14  1  jan 1988
        };

        for (size_t timestep = 0; timestep < expected.size(); ++timestep) {
            const bool ok = tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, true) == expected[timestep];
            BOOST_CHECK_MESSAGE(ok, "failing for timestep " << timestep);
        }
    }

    // Year, frequency 2.
    {
        std::vector<bool> expected = {
            false, // 0  21  may 1981 START
            false, // 1   5  jul 1981
            false, // 2   6  jul 1981
            false, // 3   5  aug 1981
            false, // 4   5  sep 1981
            false, // 5   1  oct 1981
            false, // 6   1  nov 1981
            false, // 7   1  dec 1981
            false, // 8   1  jan 1982
            false, // 9   1  jan 1982
            false, // 10  3  jan 1982
            true,  // 11  1  jan 1983
            false, // 12  1  jan 1984
            true,  // 13  1  jan 1985
            true   // 14  1  jan 1988
        };

        for (size_t timestep = 0; timestep < expected.size(); ++timestep) {
            const bool ok = tmap.isTimestepInFirstOfMonthsYearsSequence(timestep, true, 1, 2) == expected[timestep];
            BOOST_CHECK_MESSAGE(ok, "failing for timestep " << timestep);
        }
    }
}


BOOST_AUTO_TEST_CASE(initTimestepsLongStep) {
    const char *deckData =
        "START\n"
        " 1 JAN 1983 /\n"
        "\n"
        "SCHEDULE\n"
        "TSTEP\n"
        " 25550 /\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    const Opm::TimeMap tmap(deck);

    /*deckData timesteps:
    0   1 jan 1983 START
    1   14 dec 2052*/

    const auto tEnd = Opm::TimeStampUTC { tmap.getEndTime() };

    BOOST_CHECK_EQUAL(tEnd.year(), 2052);
    BOOST_CHECK_EQUAL(tEnd.month(), 12);
    BOOST_CHECK_EQUAL(tEnd.day(), 14);
}


BOOST_AUTO_TEST_CASE(TimestepsLabUnit) {
    const char *deckData =
        "START\n"
        " 1 JAN 1983 /\n"
        "\n"
        "LAB\n"
        " \n"
        "SCHEDULE\n"
        "TSTEP\n"
        " 24*10 /\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    const Opm::TimeMap tmap(deck);

    /*deckData timesteps:
    0   1 jan 1983 START
    1   11 jan 1983*/

    const auto tEnd = Opm::TimeStampUTC { tmap.getEndTime() };

    BOOST_CHECK_EQUAL(tEnd.year(), 1983);
    BOOST_CHECK_EQUAL(tEnd.month(), 1);
    BOOST_CHECK_EQUAL(tEnd.day(), 11);
}


BOOST_AUTO_TEST_CASE(initTimestepsDistantDates) {
    const char *deckData =
        "START\n"
        " 1 JAN 1983 /\n"
        "\n"
        "SCHEDULE\n"
        "DATES\n"
        " 1 JAN 2040 /\n"
        " 1 JAN 2050 /\n"
        "/\n";

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);
    const Opm::TimeMap tmap(deck);

    /*deckData timesteps:
    0   1 jan 1983 START
    1   1 jan 2040
    2   1 jan 2050*/

    const auto t1 = Opm::TimeStampUTC { tmap.getStartTime(1) };
    const auto t2 = Opm::TimeStampUTC { tmap.getEndTime() };

    BOOST_CHECK_EQUAL(t1.year(), 2040);
    BOOST_CHECK_EQUAL(t1.month(), 1);
    BOOST_CHECK_EQUAL(t1.day(), 1);

    BOOST_CHECK_EQUAL(t2.year(), 2050);
    BOOST_CHECK_EQUAL(t2.month(), 1);
    BOOST_CHECK_EQUAL(t2.day(), 1);
}


BOOST_AUTO_TEST_CASE(mkdate) {
    BOOST_CHECK_THROW( Opm::TimeMap::mkdate( 2010 , 0 , 0  ) , std::invalid_argument);
    auto t0 = Opm::TimeStampUTC { Opm::TimeMap::mkdate( 2010 , 1, 1) };
    auto t1 = Opm::TimeStampUTC { Opm::TimeMap::forward( asTimeT(t0) , 24*3600) };

    BOOST_CHECK_EQUAL( t1.year() , 2010 );
    BOOST_CHECK_EQUAL( t1.month() , 1 );
    BOOST_CHECK_EQUAL( t1.day() , 2 );

    t1 = Opm::TimeMap::forward( asTimeT(t1) , -24*3600);
    BOOST_CHECK_EQUAL( t1.year() , 2010 );
    BOOST_CHECK_EQUAL( t1.month() , 1 );
    BOOST_CHECK_EQUAL( t1.day() , 1 );

    t1 = Opm::TimeMap::forward( asTimeT(t0) , 23, 55 , 300);
    BOOST_CHECK_EQUAL( t1.year() , 2010 );
    BOOST_CHECK_EQUAL( t1.month() , 1 );
    BOOST_CHECK_EQUAL( t1.day() , 2 );

}

BOOST_AUTO_TEST_CASE(mkdatetime) {
    BOOST_CHECK_THROW(Opm::TimeMap::mkdatetime(2010, 0, 0, 0, 0, 0), std::invalid_argument);
    auto t0 = Opm::TimeStampUTC { Opm::TimeMap::mkdatetime(2010, 1, 1, 0, 0, 0) };
    auto t1 = Opm::TimeStampUTC { Opm::TimeMap::forward(asTimeT(t0), 24 * 3600) };

    BOOST_CHECK_EQUAL(t1.year(), 2010);
    BOOST_CHECK_EQUAL(t1.month(), 1);
    BOOST_CHECK_EQUAL(t1.day(), 2);

    t1 = Opm::TimeMap::forward(asTimeT(t1), -24 * 3600);
    BOOST_CHECK_EQUAL(t1.year(), 2010);
    BOOST_CHECK_EQUAL(t1.month(), 1);
    BOOST_CHECK_EQUAL(t1.day(), 1);

    t1 = Opm::TimeMap::forward(asTimeT(t0), 23, 55, 300);
    BOOST_CHECK_EQUAL(t1.year(), 2010);
    BOOST_CHECK_EQUAL(t1.month(), 1);
    BOOST_CHECK_EQUAL(t1.day(), 2);
}

Opm::DeckRecord createDeckRecord(int day, const std::string &month, int year, const std::string &time) {
    Opm::DeckRecord deckRecord;
    Opm::DeckItem dayItem("DAY", int() );
    Opm::DeckItem monthItem("MONTH", std::string() );
    Opm::DeckItem yearItem("YEAR", int() );
    Opm::DeckItem timeItem("TIME", std::string() );

    yearItem.push_back(year);
    monthItem.push_back(month);
    dayItem.push_back(day);
    timeItem.push_back(time);

    deckRecord.addItem(dayItem);
    deckRecord.addItem(monthItem);
    deckRecord.addItem(yearItem);
    deckRecord.addItem(timeItem);

    return deckRecord;
}


BOOST_AUTO_TEST_CASE(TimeServiceOperatorPlus) {
    Opm::TimeStampUTC t0(Opm::TimeMap::mkdatetime(2010,1,1,0,0,0));
    auto t1 = t0 + std::chrono::duration<double>(3600*24);

    BOOST_CHECK_EQUAL(t1.year(), 2010);
    BOOST_CHECK_EQUAL(t1.month(), 1);
    BOOST_CHECK_EQUAL(t1.day(), 2);

    auto tl = Opm::asLocalTimeT(t0);
    auto tu = Opm::asTimeT(t0);
    auto diff1 = std::difftime(tl, tu);

    auto tml = *std::gmtime(&tl);
    auto tmu = *std::gmtime(&tu);

    auto diff2 = std::difftime( std::mktime(&tml), std::mktime(&tmu) );

    BOOST_CHECK_CLOSE( diff1, diff2, 1e-6);
}


BOOST_AUTO_TEST_CASE(RESTART) {
    std::string deck_string1 = R"(
START
 1 JAN 2000 /

RESTART
  'CASE'  5 /

SCHEDULE

SKIPREST

DATES
 1 JAN 2001 /
 1 JAN 2002 /
 1 JAN 2003 /
 1 JAN 2004 /
/

DATES
 1  JAN 2005 /
/

DATES
 1 JAN 2006 /
 1 JAN 2007 /
 1 JAN 2008 /
 1 JAN 2009 /
 1 JAN 2010 /
/
)";

    std::string deck_string2 = R"(
START
 1 JAN 2000 /

RESTART
  'CASE'  5 /

SCHEDULE

-- The period before the restart dates has been removed - the restart date
-- should still be picked up as report step 5.
--DATES
-- 1 JAN 2001 /
-- 1 JAN 2002 /
-- 1 JAN 2003 /
-- 1 JAN 2004 /
--/

DATES
 1  JUL 2005 /
/

DATES
 1 JAN 2006 /
 1 JAN 2007 /
 1 JAN 2008 /
 1 JAN 2009 /
 1 JAN 2010 /
/
)";

    std::string deck_string3 = R"(
START
 1 JAN 2000 /

RESTART
  'CASE'  5 /

SCHEDULE

-- This test does not have SKIPREST

TSTEP
   1 1 1 /
)";

    Opm::Parser parser;
    const auto deck1 = parser.parseString(deck_string1);
    const auto deck2 = parser.parseString(deck_string2);
    const auto deck3 = parser.parseString(deck_string3);

    // The date 2005-01-02 is not present as a DATES in the deck; invalid input.
    auto invalid_restart = std::make_pair(Opm::asTimeT(Opm::TimeStampUTC(2005, 1, 2)), 5);
    auto valid_restart = std::make_pair(Opm::asTimeT(Opm::TimeStampUTC(2005, 1, 1)), 5);

    BOOST_CHECK_THROW( Opm::TimeMap(deck1, invalid_restart) , std::invalid_argument);
    Opm::TimeMap tm1(deck1, valid_restart);
    BOOST_CHECK_THROW( tm1[1], std::invalid_argument );
    BOOST_CHECK_THROW( tm1[4], std::invalid_argument );
    auto start = tm1[0];
    BOOST_CHECK_EQUAL(start , Opm::asTimeT(Opm::TimeStampUTC(2000,1,1)));
    BOOST_CHECK_EQUAL(tm1[5] , Opm::asTimeT(Opm::TimeStampUTC(2005,1,1)));
    BOOST_CHECK(tm1.skiprest());

    Opm::TimeMap tm2(deck2, valid_restart);
    BOOST_CHECK_EQUAL(tm2[5], Opm::asTimeT(Opm::TimeStampUTC(2005,1,1)));
    BOOST_CHECK_EQUAL(tm2[6], Opm::asTimeT(Opm::TimeStampUTC(2005,7,1)));

    Opm::TimeMap tm3(deck3, valid_restart);
    BOOST_CHECK_EQUAL(tm3[5], Opm::asTimeT(Opm::TimeStampUTC(2005,1,1)));
    BOOST_CHECK_EQUAL(tm3[6], Opm::asTimeT(Opm::TimeStampUTC(2005,1,2)));
    BOOST_CHECK_EQUAL(tm3[7], Opm::asTimeT(Opm::TimeStampUTC(2005,1,3)));
    BOOST_CHECK_EQUAL(tm3[8], Opm::asTimeT(Opm::TimeStampUTC(2005,1,4)));
}
