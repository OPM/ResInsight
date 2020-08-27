
/*
  Copyright 2018 Statoil ASA.

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

#define NVERBOSE  // Suppress own messages when throw()ing

#define BOOST_TEST_MODULE CubicTest
#include <boost/test/unit_test.hpp>

/* --- our own headers --- */
#include <opm/common/utility/numeric/calculateCellVol.hpp>



//using namespace Opm;

BOOST_AUTO_TEST_SUITE ()

BOOST_AUTO_TEST_CASE (calc_cellvol)
{
 
 /* This is four example cells with different geometries.  */
    
    std::array<double,8> x1 {488100.140035, 488196.664549, 488085.584866, 488182.365605, 488099.065709, 488195.880889, 488084.559409, 488181.633495};
    std::array<double,8> y1 {6692539.945578, 6692550.834909, 6692638.574346, 6692650.086244, 6692538.810649, 6692550.080826, 6692637.628127, 6692649.429649};
    std::array<double,8> z1 {2841.856000, 2840.138000, 2842.042000, 2839.816000, 2846.142000, 2844.252000, 2846.244000, 2843.868000};
 
    std::array<double,8> x2 {489539.050892, 489638.562319, 489527.025803, 489627.914272, 489537.173839, 489638.562319, 489524.119410, 489627.914272};
    std::array<double,8> y2 {6695907.426615, 6695923.399538, 6696003.688945, 6696020.073168, 6695908.526577, 6695923.399538, 6696005.533276, 6696020.073168};
    std::array<double,8> z2 {2652.859000, 2651.765000, 2652.381000, 2652.608000, 2655.276000, 2651.765000, 2656.381000, 2652.608000};

    std::array<double,8> x3 {486819.009056, 486904.877179, 486812.975440, 486900.527416, 486818.450563, 486903.978383, 486812.975440, 486900.527416};
    std::array<double,8> y3 {6694966.253697, 6694998.360834, 6695061.104896, 6695086.717018, 6694967.135567, 6695000.197284, 6695061.104896, 6695086.717018};
    std::array<double,8> z3 {2795.193000, 2799.997000, 2792.240000, 2793.972000, 2795.671000, 2800.927000, 2792.240000, 2793.972000};

    std::array<double,8> x4 {489194.711544, 489259.232449, 489206.220219, 489294.099099, 489194.711544, 489254.725623, 489201.723535, 489289.423088};
    std::array<double,8> y4 {6694510.504054, 6694525.862296, 6694608.410134, 6694621.029382, 6694510.504054, 6694526.272988, 6694608.819829, 6694621.467114};
    std::array<double,8> z4 {2740.7340, 2754.7810, 2730.0150, 2726.1080, 2740.7340, 2758.3530, 2733.9490, 2730.1080};
    
    BOOST_REQUIRE_CLOSE (calculateCellVol(x1,y1,z1), 40368.7852157, 1e-9);
    BOOST_REQUIRE_CLOSE (calculateCellVol(x2,y2,z2), 15766.9187847524, 1e-9);
    BOOST_REQUIRE_CLOSE (calculateCellVol(x3,y3,z3), 3268.8819007839, 1e-9);
    BOOST_REQUIRE_CLOSE (calculateCellVol(x4,y4,z4), 23391.4917234564, 1e-9);
}

BOOST_AUTO_TEST_SUITE_END()
