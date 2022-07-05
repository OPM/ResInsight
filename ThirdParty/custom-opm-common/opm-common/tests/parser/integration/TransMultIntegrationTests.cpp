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

#define BOOST_TEST_MODULE TransMultTests
#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

using namespace Opm;

inline std::string pathprefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

BOOST_AUTO_TEST_CASE(MULTFLT_IN_SCHEDULE) {
    Parser parser;
    auto python = std::make_shared<Python>();
    std::string scheduleFile(pathprefix() + "TRANS/Deck1");
    ParseContext parseContext;
    auto deck = parser.parseFile(scheduleFile);
    EclipseState state(deck);
    const auto& trans = state.getTransMult();
    Schedule schedule(deck, state, python);

    BOOST_CHECK_EQUAL( 0.10 , trans.getMultiplier( 3,2,0,FaceDir::XPlus ));
    BOOST_CHECK_EQUAL( 0.10 , trans.getMultiplier( 2,2,0,FaceDir::XPlus ));
    BOOST_CHECK( schedule[3].events().hasEvent( ScheduleEvents::GEO_MODIFIER) );
    {
        const auto& keywords = schedule[3].geo_keywords();
        state.apply_schedule_keywords( keywords );
    }
    BOOST_CHECK_EQUAL( 2.00 , trans.getMultiplier( 2,2,0,FaceDir::XPlus ));
    BOOST_CHECK_EQUAL( 0.10 , trans.getMultiplier( 3,2,0,FaceDir::XPlus ));
}
