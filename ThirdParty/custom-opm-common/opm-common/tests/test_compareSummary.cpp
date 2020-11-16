/*
   +   Copyright 2016 Statoil ASA.
   +
   +   This file is part of the Open Porous Media project (OPM).
   +
   +   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   +   the Free Software Foundation, either version 3 of the License, or
   +   (at your option) any later version.
   +
   +   OPM is distributed in the hope that it will be useful,
   +   but WITHOUT ANY WARRANTY; without even the implied warranty of
   +   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   +   GNU General Public License for more details.
   +
   +   You should have received a copy of the GNU General Public License
   +   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   +   */


#include "config.h"
#include <examples/test_util/summaryComparator.hpp>
#include <examples/test_util/summaryIntegrationTest.hpp>


#define BOOST_TEST_MODULE CalculationTest
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(deviation){
    double a = 5;
    double b = 10;
    const double tol = 1.0e-14;

    Deviation dev = SummaryComparator::calculateDeviations(a,b);
    BOOST_CHECK_EQUAL(dev.abs, 5);
    BOOST_CHECK_CLOSE(dev.rel, 5.0/10.0, tol);
}


BOOST_AUTO_TEST_CASE(area) {
    double width1 = 0;
    double width2 = 2;
    double width3 = 10;

    double area1 = SummaryIntegrationTest::getRectangleArea(1,width2);
    double area2 = SummaryIntegrationTest::getRectangleArea(10,width1);
    double area3 = SummaryIntegrationTest::getRectangleArea(4,width3);

    BOOST_CHECK_EQUAL(area1,2);
    BOOST_CHECK_EQUAL(area2,0);
    BOOST_CHECK_EQUAL(area3,40);
}


BOOST_AUTO_TEST_CASE(operatorOverload) {
    WellProductionVolume volumeA;
    WellProductionVolume volumeB;
    volumeA.total = 2;
    volumeA.error = 2;
    volumeB.total = 3;
    volumeB.error = 1;
    volumeA += volumeB;

    BOOST_CHECK_EQUAL(volumeA.total,5);
    BOOST_CHECK_EQUAL(volumeA.error,3);
}


BOOST_AUTO_TEST_CASE(integration) {
    std::vector<double> data1 = {2,2,2,2,2,2};
    std::vector<double> time1 = {0,1,2,3,4,5};

    double val1 = SummaryIntegrationTest::integrate(time1, data1);

    std::vector<double> data2 = {3, 3, 4, 3, 2, 1, 0, 4};
    std::vector<double> time2 = {0, 0.5, 1, 2, 3, 3.5, 4.5, 5};

    std::vector<double> data3 = {0, 0, 0, 0, 0, 0, 0, 0};

    std::vector<double> data4 = {0, 1, 4, 1, 2, 4, 0, 2, 5, 6, 3};
    std::vector<double> time4 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10};

    std::vector<double> data5 = {0, 4,   3, 2,   5, 6, 3,   1, 0,  4, 2,   1};
    std::vector<double> time5 = {0, 0.5, 1, 2.5, 3, 4, 4.5, 6, 7.5,8, 9.5 ,10};

    double val2 = SummaryIntegrationTest::integrateError(time1, data1, time2, data2);
    double val3 = SummaryIntegrationTest::integrateError(time1, data1, time2, data3);
    double val4 = SummaryIntegrationTest::integrateError(time1, data1, time1, data1);
    double val5 = SummaryIntegrationTest::integrateError(time4, data4, time5, data5);
    BOOST_CHECK_EQUAL(val1,10);
    BOOST_CHECK_EQUAL(val2,6);
    BOOST_CHECK_EQUAL(val3,10);
    BOOST_CHECK_EQUAL(val4,0);
    BOOST_CHECK_EQUAL(val5,24.5);
}
