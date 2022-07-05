/*
   +   Copyright 2019 Equinor ASA.
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

#include <opm/io/eclipse/ERft.hpp>

#define BOOST_TEST_MODULE Test EGrid
#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/EclIOdata.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdexcept>
#include <stdio.h>
#include <tuple>

#include "tests/WorkArea.hpp"

using namespace Opm::EclIO;

template<typename InputIterator1, typename InputIterator2>
bool
range_equal(InputIterator1 first1, InputIterator1 last1,
            InputIterator2 first2, InputIterator2 last2)
{
    while(first1 != last1 && first2 != last2)
    {
        if(*first1 != *first2) return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

bool compare_files(const std::string& filename1, const std::string& filename2)
{
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    std::istreambuf_iterator<char> end;

    return range_equal(begin1, end, begin2, end);
}


template <typename T>
bool operator==(const std::vector<T> & t1, const std::vector<T> & t2)
{
    return std::equal(t1.begin(), t1.end(), t2.begin(), t2.end());
}

// test is using SPE1CASE1, with minor modifications in order to test API for EGrid class
//  -> 6 cells made inactive, box: 5 7  5 6  1 1

// first pilar at x=0.0, y=0.0 and z=0.0
// dx = 1000 ft, dy = 1000 ft. dz = 20 ft for layer 1, 30 ft for layer 2 and 50 ft for layer 3.
// size of grid is 10x10x3


BOOST_AUTO_TEST_CASE(TestERft_1) {
    using Date = std::tuple<int, int, int>;

    std::vector<std::string> ref_wellList= {"A-1H", "B-2H", "INJ", "PROD"};
    std::vector<Date> ref_dates= {
        Date{2015,1, 1},
        Date{2015,9, 1},
        Date{2016,5,31},
        Date{2017,7,31},
    };

    const std::vector<std::tuple<std::string, Date, float>> ref_rftList = {
        {"PROD", Date{2015,1, 1}, 0.00000000e+00},
        {"INJ" , Date{2015,1, 1}, 0.00000000e+00},
        {"A-1H", Date{2015,9, 1}, 0.24300000E+03},
        {"B-2H", Date{2016,5,31}, 0.51600000E+03},
        {"PROD", Date{2017,7,31}, 0.94200000E+03}
    };

    std::string testFile="SPE1CASE1.RFT";

    ERft rft1(testFile);

    std::vector<std::string> wellList = rft1.listOfWells();
    std::vector<Date> rftDates = rft1.listOfdates();
    std::vector<std::tuple<std::string, Date, float>> rftList = rft1.listOfRftReports();

    BOOST_CHECK_EQUAL(wellList==ref_wellList, true);
    BOOST_CHECK_EQUAL(rftDates==ref_dates, true);

    BOOST_CHECK_EQUAL(rftList==ref_rftList, true);

    BOOST_CHECK_EQUAL(rft1.hasRft("PROD", Date{2015,1,1}), true);
    BOOST_CHECK_EQUAL(rft1.hasRft("PROD",2015,1,1), true);

    BOOST_CHECK_EQUAL(rft1.hasRft("PROD", Date{2015,10,1}), false);
    BOOST_CHECK_EQUAL(rft1.hasRft("XXXX", Date{2015,1,1}), false);
    BOOST_CHECK_EQUAL(rft1.hasRft("PROD",2015,10,1), false);
    BOOST_CHECK_EQUAL(rft1.hasRft("XXXX",2015,1,1), false);

   // test member function hasArray

    BOOST_CHECK_EQUAL(rft1.hasArray("SGAS","B-2H", Date{2016,5,31}), true);
    BOOST_CHECK_EQUAL(rft1.hasArray("XXXX","B-2H", Date{2016,5,31}), false);

    BOOST_CHECK_EQUAL(rft1.hasArray("SGAS","C-2H", Date{2016,5,31}), false);
    BOOST_CHECK_EQUAL(rft1.hasArray("SGAS","B-2H", Date{2016,5,30}), false);
    BOOST_CHECK_EQUAL(rft1.hasArray("SGAS","C-2H", Date{2016,5,30}), false);
    BOOST_CHECK_EQUAL(rft1.hasArray("XXXX","C-2H", Date{2016,5,30}), false);


   //    // test member function getRft(name, wellName, date)

    std::vector<int> vect1=rft1.getRft<int>("CONIPOS","B-2H", Date{2016,5,31});
    std::vector<float> vect2=rft1.getRft<float>("PRESSURE","B-2H", Date{2016,5,31});
    std::vector<std::string> vect3=rft1.getRft<std::string>("WELLETC","B-2H", Date{2016,5,31});

    BOOST_CHECK_EQUAL(vect1.size(), 3U);
    BOOST_CHECK_EQUAL(vect2.size(), 3U);
    BOOST_CHECK_EQUAL(vect3.size(), 16U);

   // test member function getRft(name, reportIndex)

    std::vector<int> vect1a=rft1.getRft<int>("CONIPOS", 3);
    BOOST_CHECK_EQUAL(vect1.size(), vect1a.size());

    std::vector<float> vect2a = rft1.getRft<float>("PRESSURE", 3);
    BOOST_CHECK_EQUAL(vect2.size(), vect2a.size());

    for (size_t t = 0; t < vect2.size(); t++){
        BOOST_CHECK_EQUAL(vect2[t], vect2a[t]);
    }

    std::vector<std::string> vect3a = rft1.getRft<std::string>("WELLETC", 3);
    BOOST_CHECK_EQUAL(vect2.size(), vect2a.size());

    for (size_t t = 0; t < vect3.size(); t++){
        BOOST_CHECK_EQUAL(vect3[t], vect3a[t]);
    }

    // called with invalid argument, array not existing, wrong well name or wrong date
    BOOST_CHECK_THROW(std::vector<int> vect11=rft1.getRft<int>("CONIPOS","C-2H", Date{2016,5,31}),std::invalid_argument);
    BOOST_CHECK_THROW(std::vector<int> vect11=rft1.getRft<int>("CONIPOS","B-2H", Date{2016,5,30}),std::invalid_argument);
    BOOST_CHECK_THROW(std::vector<int> vect11=rft1.getRft<int>("XXXXXXX","B-2H", Date{2016,5,31}),std::invalid_argument);

    // called with wrong type
    BOOST_CHECK_THROW(std::vector<int> vect11=rft1.getRft<int>("SGAS","B-2H", Date{2016,5,31}),std::runtime_error);
    BOOST_CHECK_THROW(std::vector<float> vect11=rft1.getRft<float>("CONIPOS","B-2H", Date{2016,5,31}),std::runtime_error);
    BOOST_CHECK_THROW(std::vector<std::string> vect11=rft1.getRft<std::string>("CONIPOS","B-2H", Date{2016,5,31}), std::runtime_error);
}


BOOST_AUTO_TEST_CASE(TestERft_2) {

    std::string testFile = "SPE1CASE1.RFT";

    std::string outFile = "TEST.RFT";

    {
        WorkArea work;
        work.copyIn(testFile);
        {
            EclOutput eclTest(outFile, false);

            ERft rft1(testFile);

            auto rftList = rft1.listOfRftReports();

            for (auto& rft : rftList) {
                std::string wellName = std::get<0>(rft);
                auto date = std::get<1>(rft);

                auto arrayList = rft1.listOfRftArrays(wellName, date);

                for (auto& array : arrayList) {
                    std::string arrName = std::get<0>(array);
                    eclArrType arrType = std::get<1>(array);

                    if (arrType == INTE) {
                        std::vector<int> vect = rft1.getRft<int>(arrName, wellName, date);
                        eclTest.write(arrName, vect);
                    } else if (arrType == REAL) {
                        std::vector<float> vect = rft1.getRft<float>(arrName, wellName, date);
                        eclTest.write(arrName, vect);
                    } else if (arrType == DOUB) {
                        std::vector<double> vect = rft1.getRft<double>(arrName, wellName, date);
                        eclTest.write(arrName, vect);
                    } else if (arrType == LOGI) {
                        std::vector<bool> vect = rft1.getRft<bool>(arrName, wellName, date);
                        eclTest.write(arrName, vect);
                    } else if (arrType == CHAR) {
                        std::vector<std::string> vect = rft1.getRft<std::string>(arrName, wellName, date);
                        eclTest.write(arrName, vect);
                    } else if (arrType == MESS) {
                        eclTest.write(arrName, std::vector<char>());
                    } else {
                        std::cout << "unknown type " << std::endl;
                        exit(1);
                    }
                }
            }
        }

        BOOST_CHECK_EQUAL(compare_files(testFile, outFile), true);
    }
}
