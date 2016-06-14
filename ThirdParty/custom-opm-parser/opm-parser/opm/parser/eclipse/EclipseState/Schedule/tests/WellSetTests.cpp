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

#define BOOST_TEST_MODULE WellSetTest
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>


static Opm::TimeMapPtr createXDaysTimeMap(size_t numDays) {
    boost::gregorian::date startDate( 2010 , boost::gregorian::Jan , 1);
    Opm::TimeMapPtr timeMap(new Opm::TimeMap(boost::posix_time::ptime(startDate)));
    for (size_t i = 0; i < numDays; i++)
        timeMap->addTStep( boost::posix_time::hours( (i+1) * 24 ));
    return timeMap;
}


BOOST_AUTO_TEST_CASE(CreatEmptyWellSet) {
    Opm::WellSet wellSet;

    BOOST_CHECK_EQUAL(false , wellSet.hasWell("WELL"));
    BOOST_CHECK_EQUAL(0U , wellSet.size());
    BOOST_CHECK_THROW( wellSet.getWell( "WELL") , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(AddAndDeleteWell) {
    Opm::WellSet wellSet;
    Opm::TimeMapPtr timeMap = createXDaysTimeMap(10);

    std::shared_ptr<const Opm::EclipseGrid> grid = std::make_shared<const Opm::EclipseGrid>(10,10,10);
    Opm::WellPtr well(new Opm::Well("WELL1"  , grid , 0, 0, Opm::Value<double>("REF_DEPTH") , Opm::Phase::OIL, timeMap , 0));
    Opm::WellPtr well2(new Opm::Well("WELL2" , grid , 0, 0, Opm::Value<double>("REF_DEPTH") , Opm::Phase::OIL, timeMap , 0));

    wellSet.addWell( well );
    BOOST_CHECK_EQUAL(true , wellSet.hasWell("WELL1"));
    BOOST_CHECK_EQUAL(1U , wellSet.size());
    BOOST_CHECK_EQUAL( well , wellSet.getWell("WELL1"));


    wellSet.addWell( well2 );
    BOOST_CHECK_EQUAL(true , wellSet.hasWell("WELL2"));
    BOOST_CHECK_EQUAL(2U , wellSet.size());
    BOOST_CHECK_EQUAL( well2 , wellSet.getWell("WELL2"));

    wellSet.delWell("WELL1");
    BOOST_CHECK_EQUAL(false , wellSet.hasWell("WELL1"));
    BOOST_CHECK_EQUAL(1U , wellSet.size());
    BOOST_CHECK_EQUAL( well2 , wellSet.getWell("WELL2"));
}


BOOST_AUTO_TEST_CASE(AddWellSameName) {
    Opm::WellSet wellSet;
    Opm::TimeMapPtr timeMap = createXDaysTimeMap(10);

    std::shared_ptr<const Opm::EclipseGrid> grid = std::make_shared<const Opm::EclipseGrid>(10,10,10);
    Opm::WellPtr well1(new Opm::Well("WELL" , grid , 0, 0,Opm::Value<double>("REF_DEPTH"), Opm::Phase::OIL, timeMap , 0));
    Opm::WellPtr well2(new Opm::Well("WELL" , grid , 0, 0, Opm::Value<double>("REF_DEPTH"), Opm::Phase::OIL, timeMap , 0));

    wellSet.addWell( well1 );
    BOOST_CHECK_EQUAL(true , wellSet.hasWell("WELL"));

    BOOST_CHECK_NO_THROW( wellSet.addWell( well1 ));
    BOOST_CHECK_THROW( wellSet.addWell( well2 ) , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(Iterator) {
    Opm::WellSet wellSet;
    Opm::TimeMapPtr timeMap = createXDaysTimeMap(10);

    std::shared_ptr<const Opm::EclipseGrid> grid = std::make_shared<const Opm::EclipseGrid>(10,10,10);
    Opm::WellPtr well1(new Opm::Well("WELL" , grid , 0, 0,Opm::Value<double>("REF_DEPTH"), Opm::Phase::OIL, timeMap , 0));
    Opm::WellPtr well2(new Opm::Well("WELL" , grid , 0, 0, Opm::Value<double>("REF_DEPTH"), Opm::Phase::OIL, timeMap , 0));

    for( const auto& well : wellSet )
        BOOST_CHECK( well.second->isProducer( 0 ) );
}
