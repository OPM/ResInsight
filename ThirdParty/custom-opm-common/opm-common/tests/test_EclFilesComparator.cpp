/*
   Copyright 2016 Statoil ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
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

#define BOOST_TEST_MODULE EclFilesComparatorTest

#include <boost/test/unit_test.hpp>

#include <test_util/EclFilesComparator.hpp>

BOOST_AUTO_TEST_CASE(deviation) {
    double a = 1;
    double b = 3;
    const double tol = 1.0e-14;

    Deviation dev = ECLFilesComparator::calculateDeviations(a,b);

    BOOST_CHECK_EQUAL(dev.abs, 2.0);
    BOOST_CHECK_CLOSE(dev.rel, 2.0/3, tol);

    a = 0;

    dev = ECLFilesComparator::calculateDeviations(a,b);

    BOOST_CHECK_EQUAL(dev.abs, 3.0);
    BOOST_CHECK_EQUAL(dev.rel, -1.0);
}



BOOST_AUTO_TEST_CASE(median) {
    std::vector<double> vec = {1,3,4,5};

    double med = ECLFilesComparator::median(vec);

    BOOST_CHECK_EQUAL(med, 3.5);

    vec = {1,4,5};
    med = ECLFilesComparator::median(vec);

    BOOST_CHECK_EQUAL(med, 4);
}



BOOST_AUTO_TEST_CASE(average) {
    std::vector<double> vec = {1,3,4,5};
    const double tol = 1.0e-14;

    double avg = ECLFilesComparator::average(vec);

    BOOST_CHECK_CLOSE(avg, 13.0/4, tol);
}
