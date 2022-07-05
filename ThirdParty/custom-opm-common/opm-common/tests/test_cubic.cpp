//===========================================================================
//
// File: monotcubicinterpolator_test.cpp
//
// Created: Tue Dec  8 12:25:30 2009
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//            Bård Skaflestad     <bard.skaflestad@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.
  Portions Copyright 2013 Uni Research AS.

  This file is part of The Open Reservoir Simulator Project (OpenRS).

  OpenRS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenRS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenRS.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#define NVERBOSE  // Suppress own messages when throw()ing

#define BOOST_TEST_MODULE CubicTest
#include <boost/test/unit_test.hpp>

/* --- our own headers --- */
#include <opm/common/utility/numeric/MonotCubicInterpolator.hpp>
using namespace Opm;

BOOST_AUTO_TEST_SUITE ()

BOOST_AUTO_TEST_CASE (cubic)
{
    const int num_v = 3;
    double xv[num_v] = {0.0, 1.0, 2.0};
    double fv[num_v] = {10.0, 21.0, 2.0};
    std::vector<double> x(xv, xv + num_v);
    std::vector<double> f(fv, fv + num_v);
    MonotCubicInterpolator interp(x, f);
    BOOST_REQUIRE_CLOSE (interp.evaluate(-1.0), 10., 0.00001);
    BOOST_REQUIRE_CLOSE (interp.evaluate(0.0), 10., 0.00001);
    BOOST_REQUIRE_CLOSE (interp.evaluate(0.0001), 10.0011, 0.00001);
    BOOST_REQUIRE_CLOSE (interp.evaluate(0.5), 17.375, 0.00001);
    BOOST_REQUIRE_CLOSE (interp.evaluate(1.0), 21., 0.00001);
    BOOST_REQUIRE_CLOSE (interp.evaluate(2.0), 2., 0.00001);
    BOOST_REQUIRE_CLOSE (interp.evaluate(4.0), 2., 0.00001);
}

BOOST_AUTO_TEST_SUITE_END()
