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

#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/EclFile.hpp>

#define BOOST_TEST_MODULE Test EclIO
#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/io/eclipse/OutputStream.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <math.h>
#include <random>
#include <stdio.h>
#include <tuple>
#include <type_traits>
#include <numeric>

#include <opm/common/utility/FileSystem.hpp>

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

template <typename T>
T calcSum(const std::vector<T>& x)
{
    return std::accumulate(x.begin(), x.end(), T(0));
}


BOOST_AUTO_TEST_CASE(TestERst_1) {

    std::string testFile="SPE1_TESTCASE.UNRST";
    std::vector<int> refReportStepNumbers= {1,2,5,10,15,25,50,100,120};

    std::vector<std::string> ref_zwel_10 = {"PROD","","","INJ","",""};
    std::vector<std::string> ref_zwel_25 = {"PROD","","","INJ","",""};

    std::vector<int> ref_icon_10 = {1,10,10,3,0,1,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,1,1,
                                    1,1,0,1,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0};

    std::vector<int> ref_icon_25 = {1,10,10,3,0,1,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,1,1,
                                    1,1,0,1,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0};

    std::vector<bool> ref_logih_10 = {true,true,false,false,false,false,false,false,false,false,false,false,false,
        false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
        false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
        false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
        false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
        false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
        false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
        false,false,false,false,false,false};


    std::vector<bool> ref_logih_25 = ref_logih_10;

    ERst rst1(testFile);
    rst1.loadReportStepNumber(5);

    std::vector<int> reportStepNumbers = rst1.listOfReportStepNumbers();
    BOOST_CHECK_EQUAL(reportStepNumbers==refReportStepNumbers, true);

    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(4), false);
    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(5), true);

    // try loading non-existing report step, should throw exception
    BOOST_CHECK_THROW(rst1.loadReportStepNumber(4) , std::invalid_argument );

    BOOST_CHECK_EQUAL(rst1.hasArray("XXXX", 5), false);
    BOOST_CHECK_EQUAL(rst1.hasArray("PRESSURE", 5), true);
    BOOST_CHECK_EQUAL(rst1.hasArray("PRESSURE", 4), false);


    // try to get a list of vectors from non-existing report step, should throw exception
    std::vector<std::tuple<std::string, eclArrType, int64_t>> rstArrays; // = rst1.listOfRstArrays(4);
    BOOST_CHECK_THROW(rstArrays = rst1.listOfRstArrays(4), std::invalid_argument);

    // non exising report step number, should throw exception

    BOOST_CHECK_THROW(std::vector<int> vect1=rst1.getRestartData<int>("ICON",0, 0) , std::invalid_argument );
    BOOST_CHECK_THROW(std::vector<float> vect2=rst1.getRestartData<float>("PRESSURE",0, 0) , std::invalid_argument );
    BOOST_CHECK_THROW(std::vector<double> vect3=rst1.getRestartData<double>("XGRP",0, 0) , std::invalid_argument );
    BOOST_CHECK_THROW(std::vector<bool> vect4=rst1.getRestartData<bool>("LOGIHEAD",0, 0) , std::invalid_argument );
    BOOST_CHECK_THROW(std::vector<std::string> vect4=rst1.getRestartData<std::string>("ZWEL",0, 0) , std::invalid_argument );

    // calling getRestartData<T> member function with wrong type, should throw exception

    BOOST_CHECK_THROW(std::vector<float> vect1=rst1.getRestartData<float>("ICON",5, 0) , std::runtime_error );
    BOOST_CHECK_THROW(std::vector<int> vect2=rst1.getRestartData<int>("PRESSURE",5, 0), std::runtime_error );
    BOOST_CHECK_THROW(std::vector<float> vect3=rst1.getRestartData<float>("XGRP",5, 0), std::runtime_error );
    BOOST_CHECK_THROW(std::vector<double> vect4=rst1.getRestartData<double>("LOGIHEAD",5, 0), std::runtime_error );
    BOOST_CHECK_THROW(std::vector<bool> vect5=rst1.getRestartData<bool>("ZWEL",5, 0), std::runtime_error );

    // report step number exists, but data is not loaded. Vector should in this case
    // be loaded on demand. Hence not throwing an exception

    std::vector<int> vect1=rst1.getRestartData<int>("ICON",10, 0);
    std::vector<float> vect2=rst1.getRestartData<float>("PRESSURE",10, 0);
    std::vector<double> vect3=rst1.getRestartData<double>("XGRP",10, 0);
    std::vector<bool> vect4=rst1.getRestartData<bool>("LOGIHEAD",10, 0);
    std::vector<std::string> vect5=rst1.getRestartData<std::string>("ZWEL",10, 0);

    BOOST_CHECK_EQUAL(ref_icon_10==vect1, true);

    BOOST_CHECK_EQUAL(vect2.size()==300, true);
    BOOST_REQUIRE_CLOSE (calcSum(vect2), 1.68803e+06, 1e-3);

    BOOST_CHECK_EQUAL(vect3.size()==360, true);
    BOOST_REQUIRE_CLOSE (calcSum(vect3), 1.81382e+08, 1e-3);

    BOOST_CHECK_EQUAL(ref_logih_10==vect4, true);
    BOOST_CHECK_EQUAL(ref_zwel_10==vect5, true);

    rst1.loadReportStepNumber(25);

    vect1 = rst1.getRestartData<int>("ICON",25, 0);
    vect2 = rst1.getRestartData<float>("PRESSURE",25, 0);
    vect3 = rst1.getRestartData<double>("XGRP",25, 0);
    vect4 = rst1.getRestartData<bool>("LOGIHEAD",25, 0);
    vect5 = rst1.getRestartData<std::string>("ZWEL",25, 0);

    BOOST_CHECK_EQUAL(ref_icon_25==vect1, true);

    BOOST_CHECK_EQUAL(vect2.size()==300, true);
    BOOST_REQUIRE_CLOSE (calcSum(vect2), 1.92496e+06, 1e-3);

    BOOST_CHECK_EQUAL(vect3.size()==360, true);
    BOOST_REQUIRE_CLOSE (calcSum(vect3), 4.58807e+08, 1e-3);

    BOOST_CHECK_EQUAL(ref_logih_25==vect4, true);
    BOOST_CHECK_EQUAL(ref_zwel_25==vect5, true);

}


static void readAndWrite(EclOutput& eclTest, ERst& rst1,
                         const std::string& name, int seqnum,
                         eclArrType arrType)
{
    if (arrType == INTE) {
        std::vector<int> vect = rst1.getRestartData<int>(name, seqnum, 0);
        eclTest.write(name, vect);
    } else if (arrType == REAL) {
        std::vector<float> vect = rst1.getRestartData<float>(name, seqnum, 0);
        eclTest.write(name, vect);
    } else if (arrType == DOUB) {
        std::vector<double> vect = rst1.getRestartData<double>(name, seqnum, 0);
        eclTest.write(name, vect);
    } else if (arrType == LOGI) {
        std::vector<bool> vect = rst1.getRestartData<bool>(name, seqnum, 0);
        eclTest.write(name, vect);
    } else if (arrType == CHAR) {
        std::vector<std::string> vect = rst1.getRestartData<std::string>(name, seqnum, 0);
        eclTest.write(name, vect);
    } else if (arrType == MESS) {
        eclTest.write(name, std::vector<char>());
    } else {
        std::cout << "unknown type " << std::endl;
        exit(1);
    }
}


BOOST_AUTO_TEST_CASE(TestERst_2) {

    std::string testFile="SPE1_TESTCASE.UNRST";
    std::string outFile="TEST.UNRST";

    // using API for ERst to read all array from a binary unified restart file1
    // Then write the data back to a new file and check that new file is identical with input file


    WorkArea work;
    work.copyIn(testFile);
    ERst rst1(testFile);
    {
        EclOutput eclTest(outFile, false);

        std::vector<int> seqnums = rst1.listOfReportStepNumbers();

        for (size_t i = 0; i < seqnums.size(); i++) {
            rst1.loadReportStepNumber(seqnums[i]);
            auto rstArrays = rst1.listOfRstArrays(seqnums[i]);

            for (auto& array : rstArrays) {
                std::string name = std::get<0>(array);
                eclArrType arrType = std::get<1>(array);
                readAndWrite(eclTest, rst1, name, seqnums[i], arrType);
            }
        }
    }

    BOOST_CHECK_EQUAL(compare_files(testFile, outFile), true);
}


BOOST_AUTO_TEST_CASE(TestERst_3) {

    std::string testFile="SPE1_TESTCASE.FUNRST";
    std::string outFile="TEST.FUNRST";

    // using API for ERst to read all array from a formatted unified restart file1
    // Then write the data back to a new file and check that new file is identical with input file

    WorkArea work;
    work.copyIn(testFile);
    ERst rst1(testFile);
    {
        EclOutput eclTest(outFile, true);

        std::vector<int> seqnums = rst1.listOfReportStepNumbers();
        for (unsigned int i = 0; i < seqnums.size(); i++) {
            rst1.loadReportStepNumber(seqnums[i]);

            auto rstArrays = rst1.listOfRstArrays(seqnums[i]);

            for (auto& array : rstArrays) {
                std::string name = std::get<0>(array);
                eclArrType arrType = std::get<1>(array);
                readAndWrite(eclTest, rst1, name, seqnums[i], arrType);
            }
        }
    }
    BOOST_CHECK_EQUAL(compare_files(testFile, outFile), true);
}


BOOST_AUTO_TEST_CASE(TestERst_4) {

    std::string testFile1="./SPE1_TESTCASE.UNRST";
    std::string testFile2="./SPE1_TESTCASE.F0025";
    std::string testFile3="./SPE1_TESTCASE.X0025";

    ERst rst1(testFile1);
    rst1.loadReportStepNumber(25);

    ERst rst2(testFile2);
    rst2.loadReportStepNumber(25);

    ERst rst3(testFile3);
    rst3.loadReportStepNumber(25);

    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(4), false);
    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(25), true);

    BOOST_CHECK_EQUAL(rst2.hasReportStepNumber(4), false);
    BOOST_CHECK_EQUAL(rst2.hasReportStepNumber(25), true);

    BOOST_CHECK_EQUAL(rst3.hasReportStepNumber(4), false);
    BOOST_CHECK_EQUAL(rst3.hasReportStepNumber(25), true);

    std::vector<float> pres1 = rst1.getRestartData<float>("PRESSURE",25, 0);
    std::vector<float> pres2 = rst2.getRestartData<float>("PRESSURE",25, 0);
    std::vector<float> pres3 = rst3.getRestartData<float>("PRESSURE",25, 0);

    BOOST_CHECK_EQUAL(pres1==pres2, true);
    BOOST_CHECK_EQUAL(pres1==pres3, true);
}


BOOST_AUTO_TEST_CASE(TestERst_5a) {

    std::string testRstFile = "LGR_TESTMOD.X0002";

    ERst rst1(testRstFile);

    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(2), true);
    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(0), false);

    // invalid report step number
    BOOST_CHECK_THROW(rst1.hasLGR("LGR1", 99) , std::invalid_argument );

    BOOST_CHECK_EQUAL(rst1.hasLGR("LGR1", 2), true);
    BOOST_CHECK_EQUAL(rst1.hasLGR("XXXX", 2), false);
}


BOOST_AUTO_TEST_CASE(TestERst_5b) {

    std::string testRstFile = "LGR_TESTMOD.UNRST";

    std::vector<int> ref_reports = {0, 1, 2, 3};

    std::vector<std::string> ref_names_global = {"SEQNUM", "INTEHEAD", "LOGIHEAD", "DOUBHEAD",
        "IGRP", "SGRP", "XGRP", "ZGRP", "IWEL", "SWEL", "XWEL", "ZWEL", "ZWLS", "IWLS", "ICON",
        "SCON", "XCON", "DLYTIM", "HIDDEN", "STARTSOL", "PRESSURE", "SWAT", "SGAS", "RS",
        "REGDIMS", "FIPFAMNA", "REGRPT", "FIPOIL", "FIPWAT", "FIPGAS", "PBUB", "LGRNAMES",
        "ENDSOL" };

    std::vector<int> ref_size_global = { 1, 411, 121, 229, 400, 448, 720, 20, 310, 244, 260, 6,
        3, 3, 1250, 2050, 2900, 30, 58, 0, 30, 30, 30, 30, 40, 1, 304, 30, 30, 30, 30, 2, 0 };

    std::vector<std::string> ref_names_lgr1 = { "LGR", "LGRHEADI", "LGRHEADQ", "LGRHEADD", "INTEHEAD",
        "LOGIHEAD", "DOUBHEAD", "IGRP", "SGRP", "XGRP", "ZGRP", "IWEL", "SWEL", "XWEL", "ZWEL", "LGWEL",
        "ZWLS", "IWLS", "ICON", "SCON", "XCON", "DLYTIM", "HIDDEN", "STARTSOL", "PRESSURE", "SWAT",
        "SGAS", "RS", "PBUB", "ENDSOL", "ENDLGR"};

    std::vector<int> ref_size_lgr1 = {1, 45, 5, 5, 411, 121, 229, 200, 224, 360, 10, 155, 122, 130,
        3, 1, 1, 1, 625, 1025, 1450, 30, 58, 0, 128, 128, 128, 128, 128, 0, 1 };

   std::vector<std::string> ref_names_lgr2 = {"LGR", "LGRHEADI", "LGRHEADQ", "LGRHEADD", "INTEHEAD",
       "LOGIHEAD", "DOUBHEAD", "IGRP", "SGRP", "XGRP", "ZGRP", "IWEL", "SWEL", "XWEL", "ZWEL", "LGWEL",
       "ZWLS", "IWLS", "ICON", "SCON", "XCON", "DLYTIM", "HIDDEN", "STARTSOL", "PRESSURE", "SWAT",
       "SGAS", "RS", "PBUB", "ENDSOL", "ENDLGR" };

    std::vector<int> ref_size_lgr2 = {1, 45, 5, 5, 411, 121, 229, 200, 224, 360, 10, 155, 122, 130, 3,
        1, 1, 1, 625, 1025, 1450, 30, 58, 0, 192, 192, 192, 192, 192, 0, 1};

    ERst rst1(testRstFile);

    auto report_list = rst1.listOfReportStepNumbers();

    BOOST_CHECK_EQUAL(report_list==ref_reports, true);

    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(1), true);
    BOOST_CHECK_EQUAL(rst1.hasReportStepNumber(5), false);

    // invalid report step number
    BOOST_CHECK_THROW(rst1.hasLGR("LGR1", 99) , std::invalid_argument );

    BOOST_CHECK_EQUAL(rst1.hasLGR("LGR1", 2), true);

    int rstep = 0;

    auto array_list_1 = rst1.listOfRstArrays(rstep);

    BOOST_CHECK_EQUAL(array_list_1.size(), ref_names_global.size());
    BOOST_CHECK_EQUAL(array_list_1.size(), ref_size_global.size());

    for (size_t n = 0; n < array_list_1.size(); n++){
        BOOST_CHECK_EQUAL(std::get<0>(array_list_1[n]), ref_names_global[n]);
        BOOST_CHECK_EQUAL(std::get<2>(array_list_1[n]), ref_size_global[n]);
    }

    for (size_t index = 0; index < array_list_1.size(); index++){

        std::string name = std::get<0>(array_list_1[index]);

        if (std::get<1>(array_list_1[index]) == Opm::EclIO::INTE){
            auto vect1 = rst1.getRestartData<int>(name, rstep);
            auto vect2 = rst1.getRestartData<int>(index, rstep);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_1[index]) == Opm::EclIO::REAL){
            auto vect1 = rst1.getRestartData<float>(name, rstep);
            auto vect2 = rst1.getRestartData<float>(index, rstep);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_1[index]) == Opm::EclIO::DOUB){
            auto vect1 = rst1.getRestartData<double>(name, rstep);
            auto vect2 = rst1.getRestartData<double>(index, rstep);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_1[index]) == Opm::EclIO::LOGI){
            auto vect1 = rst1.getRestartData<bool>(name, rstep);
            auto vect2 = rst1.getRestartData<bool>(index, rstep);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_1[index]) == Opm::EclIO::CHAR){
            auto vect1 = rst1.getRestartData<std::string>(name, rstep);
            auto vect2 = rst1.getRestartData<std::string>(index, rstep);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }
    }

    // -------------------------------

    std::string lgr_name = "LGR1";

    BOOST_CHECK_THROW(rst1.listOfRstArrays(0, "XXXX") , std::invalid_argument );

    auto array_list_2 = rst1.listOfRstArrays(0, lgr_name);

    BOOST_CHECK_EQUAL(array_list_2.size(), ref_names_lgr1.size());
    BOOST_CHECK_EQUAL(array_list_2.size(), ref_size_lgr1.size());

    for (size_t n = 0; n < array_list_2.size(); n++){
        BOOST_CHECK_EQUAL(std::get<0>(array_list_2[n]), ref_names_lgr1[n]);
        BOOST_CHECK_EQUAL(std::get<2>(array_list_2[n]), ref_size_lgr1[n]);
    }

    for (size_t index = 0; index < array_list_2.size(); index++){

        std::string name = std::get<0>(array_list_2[index]);

        if (std::get<1>(array_list_2[index]) == Opm::EclIO::INTE){
            auto vect1 = rst1.getRestartData<int>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<int>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_2[index]) == Opm::EclIO::REAL){
            auto vect1 = rst1.getRestartData<float>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<float>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_2[index]) == Opm::EclIO::DOUB){
            auto vect1 = rst1.getRestartData<double>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<double>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_2[index]) == Opm::EclIO::LOGI){
            auto vect1 = rst1.getRestartData<bool>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<bool>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_2[index]) == Opm::EclIO::CHAR){
            auto vect1 = rst1.getRestartData<std::string>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<std::string>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }
    }

    // -------------------------------

    lgr_name = "LGR2";

    auto array_list_3 = rst1.listOfRstArrays(0, lgr_name);

    BOOST_CHECK_EQUAL(array_list_3.size(), ref_names_lgr2.size());
    BOOST_CHECK_EQUAL(array_list_3.size(), ref_size_lgr2.size());

    for (size_t n = 0; n < array_list_2.size(); n++){
        BOOST_CHECK_EQUAL(std::get<0>(array_list_3[n]), ref_names_lgr2[n]);
        BOOST_CHECK_EQUAL(std::get<2>(array_list_3[n]), ref_size_lgr2[n]);
    }

    for (size_t index = 0; index < array_list_3.size(); index++){

        std::string name = std::get<0>(array_list_3[index]);

        if (std::get<1>(array_list_3[index]) == Opm::EclIO::INTE){
            auto vect1 = rst1.getRestartData<int>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<int>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_3[index]) == Opm::EclIO::REAL){
            auto vect1 = rst1.getRestartData<float>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<float>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_3[index]) == Opm::EclIO::DOUB){
            auto vect1 = rst1.getRestartData<double>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<double>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_3[index]) == Opm::EclIO::LOGI){
            auto vect1 = rst1.getRestartData<bool>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<bool>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }

        if (std::get<1>(array_list_3[index]) == Opm::EclIO::CHAR){
            auto vect1 = rst1.getRestartData<std::string>(name, rstep, lgr_name);
            auto vect2 = rst1.getRestartData<std::string>(index, rstep, lgr_name);
            BOOST_CHECK_EQUAL(vect1 == vect2, true);
        }
    }
}


// ====================================================================
class RSet
{
public:
    explicit RSet(std::string base)
        : odir_(std::filesystem::temp_directory_path() /
                Opm::unique_path("rset-%%%%"))
        , base_(std::move(base))
    {
        std::filesystem::create_directories(this->odir_);
    }

    ~RSet()
    {
        std::filesystem::remove_all(this->odir_);
    }

    operator ::Opm::EclIO::OutputStream::ResultSet() const
    {
        return { this->odir_.string(), this->base_ };
    }

private:
    std::filesystem::path odir_;
    std::string             base_;
};

namespace {
    template <class Coll>
    void check_is_close(const Coll& c1, const Coll& c2)
    {
        using ElmType = typename std::remove_cv<
            typename std::remove_reference<
                typename std::iterator_traits<
                    decltype(std::begin(c1))
                >::value_type
            >::type
        >::type;

        for (auto b1  = c1.begin(), e1 = c1.end(), b2 = c2.begin();
                  b1 != e1; ++b1, ++b2)
        {
            BOOST_CHECK_CLOSE(*b1, *b2, static_cast<ElmType>(1.0e-7));
        }
    }
} // Anonymous namespace

namespace Opm { namespace EclIO {

    // Needed by BOOST_CHECK_EQUAL_COLLECTIONS.
    std::ostream&
    operator<<(std::ostream& os, const EclFile::EclEntry& e)
    {
        os << "{ " << std::get<0>(e)
           << ", " << static_cast<int>(std::get<1>(e))
           << ", " << std::get<2>(e)
           << " }";

        return os;
    }
}} // Namespace Opm::EclIO

BOOST_AUTO_TEST_SUITE(Separate)

BOOST_AUTO_TEST_CASE(Unformatted)
{
    const auto rset = RSet("CASE");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ false };
    const auto unif = ::Opm::EclIO::OutputStream::Unified  { false };

    {
        const auto seqnum = 1;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 7, 2, 9});
        rst.write("L", std::vector<bool>       {true, false, false, true});
        rst.write("S", std::vector<float>      {3.1f, 4.1f, 59.265f});
        rst.write("D", std::vector<double>     {2.71, 8.21});
        rst.write("Z", std::vector<std::string>{"W1", "W2"});
    }

    {
        const auto seqnum = 13;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {35, 51, 13});
        rst.write("L", std::vector<bool>       {true, true, true, false});
        rst.write("S", std::vector<float>      {17.29e-02f, 1.4142f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180, 123.45e6});
        rst.write("Z", std::vector<std::string>{"G1", "FIELD"});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "X0001");

        auto rst = ::Opm::EclIO::ERst{fname};

        BOOST_CHECK(rst.hasReportStepNumber(1));

        {
            const auto seqnum        = rst.listOfReportStepNumbers();
            const auto expect_seqnum = std::vector<int>{1};

            BOOST_CHECK_EQUAL_COLLECTIONS(seqnum.begin(), seqnum.end(),
                                          expect_seqnum.begin(),
                                          expect_seqnum.end());
        }

        {
            const auto vectors        = rst.listOfRstArrays(1);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 2},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(1);

        {
            const auto& I = rst.getRestartData<int>("I", 1, 0);
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 1, 0);
            const auto  expect_L = std::vector<bool> {
                true, false, false, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 1, 0);
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 1, 0);
            const auto  expect_D = std::vector<double>{
                2.71, 8.21,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 1, 0);
            const auto  expect_Z = std::vector<std::string>{
                "W1", "W2",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    {
        const auto seqnum = 5;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 2, 3, 4});
        rst.write("L", std::vector<bool>       {false, false, false, true});
        rst.write("S", std::vector<float>      {1.23e-04f, 1.234e5f, -5.4321e-9f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180});
        rst.write("Z", std::vector<std::string>{"HELLO", ", ", "WORLD"});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "X0005");

        auto rst = ::Opm::EclIO::ERst{fname};

        BOOST_CHECK(!rst.hasReportStepNumber( 1));
        BOOST_CHECK( rst.hasReportStepNumber( 5));
        BOOST_CHECK(!rst.hasReportStepNumber(13));

        {
            const auto seqnum        = rst.listOfReportStepNumbers();
            const auto expect_seqnum = std::vector<int>{5};

            BOOST_CHECK_EQUAL_COLLECTIONS(seqnum.begin(), seqnum.end(),
                                          expect_seqnum.begin(),
                                          expect_seqnum.end());
        }

        {
            const auto vectors        = rst.listOfRstArrays(5);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 2},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(5);

        {
            const auto& I = rst.getRestartData<int>("I", 5, 0);
            const auto  expect_I = std::vector<int>{ 1, 2, 3, 4 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 5, 0);
            const auto  expect_L = std::vector<bool> {
                false, false, false, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 5, 0);
            const auto  expect_S = std::vector<float>{
                1.23e-04f, 1.234e5f, -5.4321e-9f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 5, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 5, 0);
            const auto  expect_Z = std::vector<std::string>{
                "HELLO", ",", "WORLD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    {
        const auto seqnum = 13;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {35, 51, 13});
        rst.write("L", std::vector<bool>       {true, true, true, false});
        rst.write("S", std::vector<float>      {17.29e-02f, 1.4142f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180, 123.45e6});
        rst.write("Z", std::vector<std::string>{"G1", "FIELD"});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "X0013");

        auto rst = ::Opm::EclIO::ERst{fname};

        BOOST_CHECK(!rst.hasReportStepNumber( 1));
        BOOST_CHECK(!rst.hasReportStepNumber( 5));
        BOOST_CHECK( rst.hasReportStepNumber(13));

        {
            const auto seqnum        = rst.listOfReportStepNumbers();
            const auto expect_seqnum = std::vector<int>{13};

            BOOST_CHECK_EQUAL_COLLECTIONS(seqnum.begin(), seqnum.end(),
                                          expect_seqnum.begin(),
                                          expect_seqnum.end());
        }

        {
            const auto vectors        = rst.listOfRstArrays(13);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 2},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(13);

        {
            const auto& I = rst.getRestartData<int>("I", 13, 0);
            const auto  expect_I = std::vector<int>{ 35, 51, 13};
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 13, 0);
            const auto  expect_L = std::vector<bool> {
                true, true, true, false,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 13, 0);
            const auto  expect_S = std::vector<float>{
                17.29e-02f, 1.4142f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 13, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180, 123.45e6,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 13, 0);
            const auto  expect_Z = std::vector<std::string>{
                "G1", "FIELD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }
}

BOOST_AUTO_TEST_CASE(Formatted)
{
    const auto rset = RSet("CASE.T01.");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ true };
    const auto unif = ::Opm::EclIO::OutputStream::Unified  { false };

    {
        const auto seqnum = 1;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 7, 2, 9});
        rst.write("L", std::vector<bool>       {true, false, false, true});
        rst.write("S", std::vector<float>      {3.1f, 4.1f, 59.265f});
        rst.write("D", std::vector<double>     {2.71, 8.21});
        rst.write("Z", std::vector<std::string>{"W1", "W2"});
    }

    {
        const auto seqnum = 13;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {35, 51, 13});
        rst.write("L", std::vector<bool>       {true, true, true, false});
        rst.write("S", std::vector<float>      {17.29e-02f, 1.4142f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180, 123.45e6});
        rst.write("Z", std::vector<std::string>{"G1", "FIELD"});
    }

    {
        using ::Opm::EclIO::OutputStream::Restart;

        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "F0013");

        auto rst = ::Opm::EclIO::ERst{fname};

        {
            const auto vectors        = rst.listOfRstArrays(13);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                // No SEQNUM in separate output files
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 2},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(13);

        {
            const auto& I = rst.getRestartData<int>("I", 13, 0);
            const auto  expect_I = std::vector<int>{ 35, 51, 13 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 13, 0);
            const auto  expect_L = std::vector<bool> {
                true, true, true, false,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 13, 0);
            const auto  expect_S = std::vector<float>{
                17.29e-02f, 1.4142f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 13, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180, 123.45e6,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 13, 0);
            const auto  expect_Z = std::vector<std::string>{
                "G1", "FIELD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    {
        // Separate output.  Step 13 should be unaffected.
        const auto seqnum = 5;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 2, 3, 4});
        rst.write("L", std::vector<bool>       {false, false, false, true});
        rst.write("S", std::vector<float>      {1.23e-04f, 1.234e5f, -5.4321e-9f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180});
        rst.write("Z", std::vector<std::string>{"HELLO", ", ", "WORLD"});
    }

    {
        using ::Opm::EclIO::OutputStream::Restart;

        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "F0005");

        auto rst = ::Opm::EclIO::ERst{fname};

        {
            const auto vectors        = rst.listOfRstArrays(5);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                // No SEQNUM in separate output files
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 2},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(5);

        {
            const auto& I = rst.getRestartData<int>("I", 5, 0);
            const auto  expect_I = std::vector<int>{ 1, 2, 3, 4 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.get<bool>("L");
            const auto  expect_L = std::vector<bool> {
                false, false, false, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 5, 0);
            const auto  expect_S = std::vector<float>{
                1.23e-04f, 1.234e5f, -5.4321e-9f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 5, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 5, 0);
            const auto  expect_Z = std::vector<std::string>{
                "HELLO", ",", "WORLD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    // -------------------------------------------------------
    // Don't rewrite step 13.  Output file should still exist.
    // -------------------------------------------------------

    {
        using ::Opm::EclIO::OutputStream::Restart;

        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "F0013");

        auto rst = ::Opm::EclIO::ERst{fname};

        BOOST_CHECK(rst.hasReportStepNumber(13));

        {
            const auto vectors        = rst.listOfRstArrays(13);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                // No SEQNUM in separate output files.
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 2},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(13);

        {
            const auto& I = rst.getRestartData<int>("I", 13, 0);
            const auto  expect_I = std::vector<int>{ 35, 51, 13 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 13, 0);
            const auto  expect_L = std::vector<bool> {
                true, true, true, false,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 13, 0);
            const auto  expect_S = std::vector<float>{
                17.29e-02f, 1.4142f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 13, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180, 123.45e6,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 13, 0);
            const auto  expect_Z = std::vector<std::string>{
                "G1", "FIELD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }
}


BOOST_AUTO_TEST_SUITE_END()

