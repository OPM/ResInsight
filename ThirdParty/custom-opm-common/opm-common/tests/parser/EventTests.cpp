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

#define BOOST_TEST_MODULE EventTests
#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Schedule/Events.hpp>



BOOST_AUTO_TEST_CASE(CreateEmpty) {
    Opm::Events events;

    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL));

    events.addEvent( Opm::ScheduleEvents::NEW_WELL);
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::NEW_WELL));

    events.addEvent( Opm::ScheduleEvents::WELL_STATUS_CHANGE);
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::NEW_WELL));
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::WELL_STATUS_CHANGE | Opm::ScheduleEvents::NEW_WELL));


    events.clearEvent(Opm::ScheduleEvents::NEW_WELL);
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL));

    events.addEvent( Opm::ScheduleEvents::NEW_WELL);
    BOOST_CHECK_EQUAL( true , events.hasEvent(Opm::ScheduleEvents::NEW_WELL));

    events.clearEvent(Opm::ScheduleEvents::NEW_WELL | Opm::ScheduleEvents::WELL_STATUS_CHANGE | Opm::ScheduleEvents::NEW_GROUP);
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL));
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::WELL_STATUS_CHANGE));



    Opm::WellGroupEvents wg_events;
    wg_events.addWell("W1");
    wg_events.addEvent("W1", Opm::ScheduleEvents::WELL_STATUS_CHANGE );
    auto ev = wg_events.at("W1");
    BOOST_CHECK_EQUAL( false , ev.hasEvent(Opm::ScheduleEvents::NEW_GROUP));
    BOOST_CHECK_EQUAL( true , ev.hasEvent(Opm::ScheduleEvents::WELL_STATUS_CHANGE));
    BOOST_CHECK_THROW(wg_events.at("NO_SUCH_WELL"), std::exception);
}

