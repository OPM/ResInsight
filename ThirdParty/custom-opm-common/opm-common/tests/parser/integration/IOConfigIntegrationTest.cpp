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

#define BOOST_TEST_MODULE IOCONFIG_INTEGRATION_TEST
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <ctime>
#include <map>
#include <tuple>
#include <vector>

#include <boost/date_time.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

using namespace Opm;

namespace {

std::string path_prefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

void verifyRestartConfig( const Schedule& sched, std::map<int, boost::gregorian::date>& rptConfig) {
    auto last = *rptConfig.rbegin();
    for (int step = 0; step <= last.first; step++) {
        if (rptConfig.count(step) == 1) {
            BOOST_CHECK( sched.write_rst_file(step) );

            const auto report_date = rptConfig.at(step);
            const std::time_t t = sched.simTime(step);
            const boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
            const boost::posix_time::ptime report_date_ptime(report_date);
            const boost::posix_time::time_duration::sec_type duration = (report_date_ptime - epoch).total_seconds();

            BOOST_CHECK_EQUAL( duration , t );
        } else
            BOOST_CHECK_MESSAGE(! sched.write_rst_file(step),
                                "Must not write restart file for report step " << step);
    }
}

}

BOOST_AUTO_TEST_CASE( NorneRestartConfig ) {
    std::map<int, boost::gregorian::date> rptConfig{};

    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(  0), std::forward_as_tuple(1997,11, 6));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(  1), std::forward_as_tuple(1997,11,14));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(  2), std::forward_as_tuple(1997,12, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(  3), std::forward_as_tuple(1997,12,17));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(  4), std::forward_as_tuple(1998, 1, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(  5), std::forward_as_tuple(1998, 2, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 10), std::forward_as_tuple(1998, 4,23));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 19), std::forward_as_tuple(1998, 7,16));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 27), std::forward_as_tuple(1998,10,13));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 33), std::forward_as_tuple(1999, 1, 4));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 44), std::forward_as_tuple(1999, 5, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 53), std::forward_as_tuple(1999, 7,15));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 62), std::forward_as_tuple(1999,10, 3));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 72), std::forward_as_tuple(2000, 2, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 77), std::forward_as_tuple(2000, 5, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 83), std::forward_as_tuple(2000, 8, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 95), std::forward_as_tuple(2000,11, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple( 98), std::forward_as_tuple(2001, 2, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(101), std::forward_as_tuple(2001, 5, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(109), std::forward_as_tuple(2001, 7, 2));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(112), std::forward_as_tuple(2001, 7,16));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(113), std::forward_as_tuple(2001, 7,30));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(114), std::forward_as_tuple(2001, 8, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(115), std::forward_as_tuple(2001, 8,10));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(116), std::forward_as_tuple(2001, 8,16));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(117), std::forward_as_tuple(2001, 9, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(118), std::forward_as_tuple(2001, 9,10));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(119), std::forward_as_tuple(2001,10, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(120), std::forward_as_tuple(2001,11, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(124), std::forward_as_tuple(2002, 2, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(129), std::forward_as_tuple(2002, 5, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(132), std::forward_as_tuple(2002, 7, 8));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(141), std::forward_as_tuple(2002,10, 7));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(148), std::forward_as_tuple(2003, 1, 2));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(157), std::forward_as_tuple(2003, 5, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(161), std::forward_as_tuple(2003, 7,10));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(164), std::forward_as_tuple(2003, 8,12));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(165), std::forward_as_tuple(2003, 9, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(166), std::forward_as_tuple(2003, 9, 2));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(167), std::forward_as_tuple(2003, 9,10));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(168), std::forward_as_tuple(2003, 9,12));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(169), std::forward_as_tuple(2003, 9,13));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(170), std::forward_as_tuple(2003, 9,16));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(171), std::forward_as_tuple(2003,10, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(172), std::forward_as_tuple(2003,10,23));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(180), std::forward_as_tuple(2004, 1,19));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(185), std::forward_as_tuple(2004, 5, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(188), std::forward_as_tuple(2004, 7, 3));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(192), std::forward_as_tuple(2004, 8,16));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(193), std::forward_as_tuple(2004, 9, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(194), std::forward_as_tuple(2004, 9,20));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(195), std::forward_as_tuple(2004,10, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(196), std::forward_as_tuple(2004,11, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(199), std::forward_as_tuple(2005, 1,12));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(206), std::forward_as_tuple(2005, 4,24));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(212), std::forward_as_tuple(2005, 7,10));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(221), std::forward_as_tuple(2005,11, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(226), std::forward_as_tuple(2006, 1,18));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(231), std::forward_as_tuple(2006, 4,25));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(235), std::forward_as_tuple(2006, 8, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(237), std::forward_as_tuple(2006, 8,16));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(238), std::forward_as_tuple(2006, 9, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(239), std::forward_as_tuple(2006, 9,14));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(240), std::forward_as_tuple(2006,10, 1));
    rptConfig.emplace(std::piecewise_construct, std::forward_as_tuple(241), std::forward_as_tuple(2006,10,10));

    const auto deck = Parser{}.parseFile( path_prefix() + "IOConfig/RPTRST_DECK.DATA");
    const EclipseState state(deck);
    const Schedule schedule(deck, state, std::make_shared<Python>());

    verifyRestartConfig(schedule, rptConfig);
}




BOOST_AUTO_TEST_CASE( RestartConfig2 ) {
    const auto deck = Parser{}.parseFile(path_prefix() + "IOConfig/RPT_TEST2.DATA");
    const EclipseState state( deck);
    const Schedule schedule(deck, state, std::make_shared<Python>());

    const auto keywords0 = schedule.rst_keywords(0);
    const std::map<std::string, int> expected0 = {
        {"BG", 1},
        {"BO", 1},
        {"BW", 1},
        {"KRG", 1},
        {"KRO", 1},
        {"KRW", 1},
        {"VOIL", 1},
        {"VGAS", 1},
        {"VWAT", 1},
        {"DEN", 1},
        {"RVSAT", 1},
        {"RSSAT", 1},
        {"PBPD", 1},
        {"NORST", 1},
    };

    for (const auto& [kw, num] : expected0)
        BOOST_CHECK_EQUAL( keywords0.at(kw), num );

    const auto keywords1 = schedule.rst_keywords(1);
    const std::map<std::string, int> expected1 = {
        {"BG", 1},
        {"BO", 1},
        {"BW", 1},
        {"KRG", 1},
        {"KRO", 1},
        {"KRW", 1},
        {"VOIL", 1},
        {"VGAS", 1},
        {"VWAT", 1},
        {"DEN", 1},
        {"RVSAT", 1},
        {"RSSAT", 1},
        {"PBPD", 1},
        {"NORST", 1},
        {"FIP", 3},
        {"WELSPECS", 1},
        {"WELLS", 0},
        {"NEWTON", 1},
        {"SUMMARY", 1},
        {"CPU", 1},
        {"CONV", 10},
    };

    for (const auto& [kw, num] : expected1)
        BOOST_CHECK_EQUAL( keywords1.at(kw), num );

    BOOST_CHECK_EQUAL(expected1.size(), keywords1.size());

    const auto keywords10 = schedule.rst_keywords(10);
    BOOST_CHECK( keywords10 == keywords1 );
}



BOOST_AUTO_TEST_CASE( SPE9END ) {
    const auto deck = Parser{}.parseFile(path_prefix() + "IOConfig/SPE9_END.DATA");
    BOOST_CHECK_NO_THROW( EclipseState state( deck) );
}
