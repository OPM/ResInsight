/*
  Copyright 2019 Equinor ASA.

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

#define NVERBOSE // to suppress our messages when throwing

#define BOOST_TEST_MODULE RootFindersTest
#include <boost/test/unit_test.hpp>

#include <opm/common/utility/numeric/RootFinders.hpp>

using namespace Opm;


template<class Method>
struct Test
{
    template <class Functor>
    static int run(const Functor& f,
                   const double a,
                   const double b,
                   const int max_iter,
                   const double tolerance)
    {
        int iter = 0;
        Method::solve(f, a, b, max_iter, tolerance, iter);
        return iter;
    }
};


BOOST_AUTO_TEST_CASE(SimpleFunction)
{
    auto f = [](double x) { return (x - 1.0)*x; };
    BOOST_CHECK_EQUAL(Test<RegulaFalsi<>>::run(f, -0.5, 0.99, 50, 1e-12), 10);
    BOOST_CHECK_EQUAL(Test<RegulaFalsiBisection<>>::run(f, -0.5, 0.99, 50, 1e-12), 15);
}

BOOST_AUTO_TEST_CASE(ToughFunction)
{
    auto f = [](double x) { return x < 50.0 ? -0.0001 : x - 50.0001; };
    BOOST_CHECK_THROW(Test<RegulaFalsi<>>::run(f, 0.0, 100.0, 50, 1e-12), std::exception);
    BOOST_CHECK_EQUAL(Test<RegulaFalsi<>>::run(f, 0.0, 100.0, 1000000, 1e-12), 90);
    BOOST_CHECK_EQUAL(Test<RegulaFalsiBisection<>>::run(f, 0.0, 100.0, 1000000, 1e-12), 2);
}
