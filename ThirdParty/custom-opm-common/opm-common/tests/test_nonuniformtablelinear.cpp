/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

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

#define NVERBOSE // to suppress our messages when throwing


#define BOOST_TEST_MODULE NonuniformTableLinearTests
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <opm/common/utility/numeric/NonuniformTableLinear.hpp>


BOOST_AUTO_TEST_CASE(utility_functions)
{
    // Test isNondecreasing().
    using Opm::isNondecreasing;
    double xva1[] = { -1.0, 2.0, 2.2, 3.0, 5.0 };
    const int numvals1 = sizeof(xva1)/sizeof(xva1[0]);
    BOOST_CHECK(isNondecreasing(xva1, xva1 + numvals1));
    double xva2[] = { -1.0, 2.0, 2.0, 2.0, 5.0 };
    const int numvals2 = sizeof(xva2)/sizeof(xva2[0]);
    BOOST_CHECK(isNondecreasing(xva2, xva2 + numvals2));
    double xva3[] = { -1.0, 2.0, 1.9, 3.0, 5.0 };
    const int numvals3 = sizeof(xva3)/sizeof(xva3[0]);
    BOOST_CHECK(!isNondecreasing(xva3, xva3 + numvals3));
}

BOOST_AUTO_TEST_CASE(table_operations)
{
    // Make a simple table.
    double xva[] = { -1.0, 2.0, 2.2, 3.0, 5.0 };
    const int numvals = sizeof(xva)/sizeof(xva[0]);
    std::vector<double> xv(xva, xva + numvals);
    double yva[numvals] = { 1.0, 2.0, 3.0, 4.0, 2.0 };
    std::vector<double> yv(yva, yva + numvals);
    Opm::NonuniformTableLinear<double> t1(xv, yv);
    Opm::NonuniformTableLinear<double> t1_copy1(xv, yv);
    Opm::NonuniformTableLinear<double> t1_copy2(t1);

    // Check equality.
    BOOST_CHECK(t1 == t1_copy1);
    BOOST_CHECK(t1 == t1_copy2);

    // Check some evaluations.
    for (int i = 0; i < numvals; ++i) {
        BOOST_CHECK_EQUAL(t1(xv[i]), yv[i]);
    }
    BOOST_CHECK_CLOSE(t1(2.6), 3.5, 1e-13);
    BOOST_CHECK_CLOSE(t1(4.0), 3.0, 1e-13);
    BOOST_CHECK_CLOSE(t1.derivative(4.0), -1.0, 1e-13);
    // Derivatives at endpoints.
    BOOST_CHECK_CLOSE(t1.derivative(-1.0), 1.0/3.0, 1e-13);
    BOOST_CHECK_CLOSE(t1.derivative(5.0), -1.0, 1e-13);
    // Extrapolation of values.
    BOOST_CHECK_CLOSE(t1(xv[0] - 1.0), 2.0/3.0, 1e-13);
    BOOST_CHECK_CLOSE(t1(xv.back() + 1.0), 1.0, 1e-13);

    // Domains.
    BOOST_CHECK_EQUAL(t1.domain().first, xv[0]);
    BOOST_CHECK_EQUAL(t1.domain().second, xv.back());
    std::pair<double, double> new_domain(-100.0, 20.0);
    t1.rescaleDomain(new_domain);
    BOOST_CHECK_EQUAL(t1.domain().first, new_domain.first);
    BOOST_CHECK_EQUAL(t1.domain().second, new_domain.second);
    for (int i = 0; i < numvals; ++i) {
        BOOST_CHECK_EQUAL(t1((xv[i] + 1.0)*20.0 - 100.0), yv[i]);
    }
    BOOST_CHECK_CLOSE(t1(0.0), 3.0, 1e-13);
    BOOST_CHECK(std::fabs(t1.derivative(0.0)  + 1.0/20.0) < 1e-11);
}
