/*
  Copyright 2016 Statoil ASA

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

#define BOOST_TEST_MODULE Solution
#include <boost/test/unit_test.hpp>

#include <vector>

#include <opm/output/data/Solution.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(Create)
{
    data::Solution c;
    BOOST_CHECK_EQUAL( c.size() , 0U );
    BOOST_CHECK_EQUAL( c.has("NO") , false);

    {
        std::vector<double> data(100);
        c.insert("NAME", UnitSystem::measure::identity, data , data::TargetType::RESTART_SOLUTION);
        BOOST_CHECK_EQUAL( c.size() , 1U );
        BOOST_CHECK_EQUAL( c.has("NAME") , true);

        BOOST_CHECK_EQUAL( c.find("NAME")->first, "NAME");

        BOOST_CHECK_THROW( c.at("NotLikeThis") , std::out_of_range );


        c.insert( "NAME2" , UnitSystem::measure::identity, data , data::TargetType::RESTART_SOLUTION );
        c.insert( "NAME3" , UnitSystem::measure::identity, data , data::TargetType::RESTART_SOLUTION );
        BOOST_CHECK_EQUAL( c.size() , 3U );

        std::vector<std::string> expected = {"NAME" , "NAME2" , "NAME3"};
        std::vector<std::string> actual;


        for (const auto& prop : c)
            actual.push_back( prop.first );

        BOOST_CHECK_EQUAL_COLLECTIONS( expected.begin() , expected.end() , actual.begin() , actual.end() );
    }
}


BOOST_AUTO_TEST_CASE(Create2)
{

    std::vector<double> data(100);
    data::Solution c = {
        { "TRANX", { UnitSystem::measure::transmissibility, data, data::TargetType::RESTART_SOLUTION } },
        { "TRANY", { UnitSystem::measure::transmissibility, data, data::TargetType::RESTART_SOLUTION } },
        { "TRANZ", { UnitSystem::measure::transmissibility, data, data::TargetType::RESTART_SOLUTION } }
    };

    auto c2 = c;
    BOOST_CHECK_EQUAL( c2.size() , 3U );
    BOOST_CHECK( c2.has("TRANX") );
}



BOOST_AUTO_TEST_CASE(UNITS) {
    std::vector<double> data(100,1);
    data::Solution c;
    auto metric = UnitSystem::newMETRIC();

    c.insert("NAME", UnitSystem::measure::pressure, data , data::TargetType::RESTART_SOLUTION);

    double si0 = c.data("NAME")[0];
    c.convertFromSI( metric );
    double metric0 = c.data("NAME")[0];
    c.convertFromSI( metric );
    BOOST_CHECK_EQUAL( metric0 , c.data("NAME")[0] );
    c.convertToSI( metric );
    BOOST_CHECK_EQUAL( si0 , c.data("NAME")[0] );
}

