/*
  Copyright 2020 Statoil ASA.

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


#define BOOST_TEST_MODULE "Serializer"
#include <boost/test/unit_test.hpp>
#include <string>
#include <unordered_map>

#include <opm/common/utility/Serializer.hpp>

BOOST_AUTO_TEST_CASE(SERIALIZER) {
    Opm::Serializer ser;

    int int_value = 100;
    double double_value = 3.14;
    std::string string_value = "String";
    std::unordered_map<std::string, int> m = {{"A", 1}, {"B", 2}, {"C", 3}};
    std::vector<int> v = {1,2,3,4,5,6,7,8,9,10};

    ser.put(int_value);
    ser.put(double_value);
    ser.put(string_value);
    ser.put_map(m);
    ser.put_vector(v);

    Opm::Serializer ser2(ser.buffer);
    BOOST_CHECK_EQUAL(ser2.get<int>(), int_value);
    BOOST_CHECK_EQUAL(ser2.get<double>(), double_value);
    BOOST_CHECK_EQUAL(ser2.get<std::string>(), string_value);


    std::unordered_map<std::string, int> m2 = ser2.get_map<std::string,int>();
    BOOST_CHECK(m2 == m);

    std::vector<int> v2 = ser2.get_vector<int>();
    BOOST_CHECK(v2 == v);
}

BOOST_AUTO_TEST_CASE(EMPTY_STRING) {
    Opm::Serializer ser;
    ser.put(std::string{});
    BOOST_CHECK_THROW( ser.put(""), std::logic_error);

    Opm::Serializer ser2(ser.buffer);
    BOOST_CHECK_EQUAL(ser2.get<std::string>(), "");
}

