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

#define BOOST_TEST_MODULE EventTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Events.hpp>
#include <opm/common/utility/TimeService.hpp>



BOOST_AUTO_TEST_CASE(CreateEmpty) {
    std::vector<std::time_t> tp = { Opm::asTimeT(Opm::TimeStampUTC(2010,1,1)) };

    for (int i = 0; i < 11; i++)
        tp.push_back( Opm::asTimeT(Opm::TimeStampUTC(2010,1,i+2)));

    Opm::TimeMap timeMap(tp);
    Opm::Events events( timeMap );
    Opm::DynamicVector<double> vector(timeMap , 9.99);

    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 10));

    events.addEvent( Opm::ScheduleEvents::NEW_WELL ,  0  );
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 0));

    events.addEvent( Opm::ScheduleEvents::NEW_WELL , 10 );
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 9));
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 10));
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 11));

    events.addEvent( Opm::ScheduleEvents::NEW_WELL , 10 );
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 9));
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 10));
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 11));

    events.addEvent( Opm::ScheduleEvents::WELL_STATUS_CHANGE ,  9 );
    events.addEvent( Opm::ScheduleEvents::WELL_STATUS_CHANGE , 10 );
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 9));
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 10));
    BOOST_CHECK_EQUAL( false , events.hasEvent(Opm::ScheduleEvents::NEW_WELL , 11));
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::WELL_STATUS_CHANGE , 9));
    BOOST_CHECK_EQUAL( true  , events.hasEvent(Opm::ScheduleEvents::WELL_STATUS_CHANGE , 10));
}


BOOST_AUTO_TEST_CASE(TestMultiple) {
    const std::time_t startDate = Opm::TimeMap::mkdate(2010, 1, 1);
    Opm::TimeMap timeMap( { startDate } );
    Opm::DynamicVector<double> vector(timeMap , 9.99);
    Opm::Events events( timeMap );

    events.addEvent( Opm::ScheduleEvents::NEW_WELL , 0 );
    BOOST_CHECK( events.hasEvent( Opm::ScheduleEvents::NEW_WELL | Opm::ScheduleEvents::NEW_GROUP , 0 ));
}
