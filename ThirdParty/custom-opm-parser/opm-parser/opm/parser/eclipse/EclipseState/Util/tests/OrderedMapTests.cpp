/*
  Copyright 2014 Statoil ASA.

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

#define BOOST_TEST_MODULE ScheduleTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Util/OrderedMap.hpp>


BOOST_AUTO_TEST_CASE( check_empty) {
    Opm::OrderedMap<std::string> map;
    BOOST_CHECK_EQUAL( 0U , map.size() );
    BOOST_CHECK( !map.hasKey("KEY"));
    BOOST_CHECK_THROW( map.get( 0 ) , std::invalid_argument);
    BOOST_CHECK_THROW( map.get( "KEY" ) , std::invalid_argument);
}


BOOST_AUTO_TEST_CASE( check_order ) {
    Opm::OrderedMap<std::string> map;
    map.insert("CKEY1" , "Value1");
    map.insert("BKEY2" , "Value2");
    map.insert("AKEY3" , "Value3");

    BOOST_CHECK_EQUAL( 3U , map.size() );
    BOOST_CHECK( map.hasKey("CKEY1"));
    BOOST_CHECK( map.hasKey("BKEY2"));
    BOOST_CHECK( map.hasKey("AKEY3"));

    BOOST_CHECK_EQUAL( "Value1" , map.get("CKEY1"));
    BOOST_CHECK_EQUAL( "Value1" , map.get( 0 ));

    BOOST_CHECK_EQUAL( "Value2" , map.get("BKEY2"));
    BOOST_CHECK_EQUAL( "Value2" , map.get( 1 ));

    BOOST_CHECK_EQUAL( "Value3" , map.get("AKEY3"));
    BOOST_CHECK_EQUAL( "Value3" , map.get( 2 ));

    map.insert( "CKEY1" , "NewValue1");
    BOOST_CHECK_EQUAL( "NewValue1" , map.get("CKEY1"));
    BOOST_CHECK_EQUAL( "NewValue1" , map.get( 0 ));

    {
        std::vector<std::string> values;
        for (auto iter = map.begin(); iter != map.end(); ++iter)
            values.push_back( *iter );

        BOOST_CHECK_EQUAL( values[0] , "NewValue1");
        BOOST_CHECK_EQUAL( values[1] , "Value2");
        BOOST_CHECK_EQUAL( values[2] , "Value3");
    }
}
