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

#include <config.h>

#define BOOST_TEST_MODULE FLOAT_CMP_TESTS
#include <boost/test/unit_test.hpp>

#include <stdexcept>

#include <opm/common/utility/numeric/cmp.hpp>

using namespace Opm;

/**
   Ahhh - the joys of comparing floating point numbers ....

   http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
*/

BOOST_AUTO_TEST_CASE(TestSCalarcmp) {
    const double abs_epsilon = cmp::default_abs_epsilon;
    const double rel_epsilon = cmp::default_rel_epsilon;

    BOOST_CHECK( cmp::scalar_equal<double>(1,1));
    BOOST_CHECK_EQUAL( false , cmp::scalar_equal<double>(1,0));
    BOOST_CHECK_EQUAL( false , cmp::scalar_equal<double>(0,1));
    BOOST_CHECK_EQUAL( false , cmp::scalar_equal<double>(-1,1));



    double v1,v2;
    /* Should be equal: */
    {
        v1 = 0.0;
        v2 = 0.0;
        BOOST_CHECK( cmp::scalar_equal<double>( v1 , v2));

        v1 = 1e-12;
        v2 = v1 + 0.5*abs_epsilon;
        BOOST_CHECK( cmp::scalar_equal<double>( v1 , v2));

        v1 = 7.0;
        v2 = 7.0;
        BOOST_CHECK( cmp::scalar_equal<double>( v1 , v2));

        v1 = -7.0;
        v2 = -7.0;
        BOOST_CHECK( cmp::scalar_equal<double>( v1 , v2));

        v1 = 0;
        v2 = 0.5 * abs_epsilon;
        BOOST_CHECK( cmp::scalar_equal<double>( v1 , v2));


        v1 = 1e7;
        v2 = 1e7 + 2*abs_epsilon;
        BOOST_CHECK( cmp::scalar_equal<double>( v1 , v2 ));

        v1 = 1e7*(1 - abs_epsilon);
        v2 = 1e7*(1 + rel_epsilon);
        BOOST_CHECK( !cmp::scalar_equal<double>( v1 , v2 ));

        v1 = 1e7*(1 + abs_epsilon);
        v2 = 1e7*(1 + rel_epsilon);
        BOOST_CHECK( cmp::scalar_equal<double>( v1 , v2 ));
    }

    /* Should be different: */
    {
        v1 = 0;
        v2 = 1.5 * abs_epsilon;
        BOOST_CHECK( !cmp::scalar_equal<double>( v1 , v2 ));

        v1 = 1e-8;
        v2 = v1 + 1.5*abs_epsilon;
        BOOST_CHECK( !cmp::scalar_equal<double>( v1 , v2 ));

        v1 = 1;
        v2 = v1*(1 + 2*rel_epsilon + abs_epsilon);
        BOOST_CHECK( !cmp::scalar_equal<double>( v1 , v2 ));

        v1 = 10;
        v2 = v1*(1 + 2*rel_epsilon + abs_epsilon);
        BOOST_CHECK( !cmp::scalar_equal<double>( v1 , v2 ));

        v1 = 1e7;
        v2 = 1e7*(1 + 2*rel_epsilon + abs_epsilon);
        BOOST_CHECK( !cmp::scalar_equal<double>( v1 , v2 ));
    }
}

/* Ensure that float instantiation works. */
BOOST_AUTO_TEST_CASE(TestFloatcmp) {
    std::vector<float> v1;
    std::vector<float> v2;
    for (size_t i =0; i < 10; i++) {
        v1.push_back( i * 1.0 );
        v2.push_back( i * 1.0 );
    }
    BOOST_CHECK( cmp::vector_equal<float>(v1 , v2 ));
    v1.push_back( 27 );
    BOOST_CHECK( !cmp::vector_equal<float>(v1 , v2 ));
    v2.push_back( 27 );
    BOOST_CHECK( cmp::vector_equal(v1 , v2 ));
}

