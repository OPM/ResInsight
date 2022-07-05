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

#include "config.h"

#define BOOST_TEST_MODULE RegionCache
#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <unordered_set>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/output/eclipse/RegionCache.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/output/eclipse/RegionCache.hpp>

using namespace Opm;

const char* path = "summary_deck.DATA";

bool cmp_list(const std::vector<std::string>& l1, const std::vector<std::string>& l2) {
    std::unordered_set<std::string> s1(l1.begin(), l1.end());
    std::unordered_set<std::string> s2(l2.begin(), l2.end());
    return s1 == s2;
}



BOOST_AUTO_TEST_CASE(create) {
    auto python = std::make_shared<Python>();
    Parser parser;
    Deck deck( parser.parseFile( path ));
    EclipseState es(deck);
    const EclipseGrid& grid = es.getInputGrid();
    Schedule schedule( deck, es, python);
    out::RegionCache rc({"FIPNUM"}, es.fieldProps(), grid, schedule);
    {
        const auto& empty = rc.connections( "FIPNUM", 4 );
        BOOST_CHECK_EQUAL( empty.size() , 0U );
    }

    {
        const auto& top_layer = rc.connections(  "FIPNUM", 1 );
        BOOST_CHECK_EQUAL( top_layer.size() , 4U );
        {
            auto pair = top_layer[0];
            BOOST_CHECK_EQUAL( pair.first , "W_1");
            BOOST_CHECK_EQUAL( pair.second , grid.activeIndex( 0,0,0));
        }
    }

    BOOST_CHECK( rc.wells("FIPXYZ", 100).empty() );
    BOOST_CHECK( rc.wells("FIPXYZ", 1).empty() );
    BOOST_CHECK( rc.wells("FIPNUM", 100).empty() );
    BOOST_CHECK( cmp_list(rc.wells("FIPNUM", 1),  {"W_1", "W_2", "W_3", "W_4"}));
    BOOST_CHECK( cmp_list(rc.wells("FIPNUM", 11), {"W_6"}));
}
