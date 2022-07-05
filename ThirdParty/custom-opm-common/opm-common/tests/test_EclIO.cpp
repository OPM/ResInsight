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
#include <math.h>
#include <stdio.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <tuple>
#include <cmath>
#include <numeric>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclUtil.hpp>
#include "WorkArea.hpp"

#include <opm/io/eclipse/EclOutput.hpp>

#define BOOST_TEST_MODULE Test EclIO
#include <boost/test/unit_test.hpp>



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

void write_header(std::ofstream& ofileH, std::string& arrName, int size, std::string arrtype){

    int bhead = flipEndianInt(16);
    int fsize = flipEndianInt(size);

    ofileH.write(reinterpret_cast<char*>(&bhead), sizeof(bhead));
    ofileH.write(arrName.c_str(), 8);
    ofileH.write(reinterpret_cast<char*>(&fsize), sizeof(fsize));
    ofileH.write(arrtype.c_str(), 4);
    ofileH.write(reinterpret_cast<char*>(&bhead), sizeof(bhead));
}


BOOST_AUTO_TEST_CASE(TestEclFile_X231) {
    WorkArea work;

    std::string filename = "TEST.DAT";
    std::string arrName = "TESTX231";

    std::vector<int> ivect(10);
    std::iota(ivect.begin(), ivect.end(), -4);

    {
        std::ofstream ofileH;
        ofileH.open(filename, std::ios_base::binary);

        int size = static_cast<int>((-1) * std::pow(2, 31) + 10);

        write_header(ofileH, arrName, -1, std::string("X231"));
        write_header(ofileH, arrName, size, std::string("INTE"));

        int sizeData = ivect.size() * sizeof(int);
        sizeData = flipEndianInt(sizeData);

        ofileH.write(reinterpret_cast<char*>(&sizeData), sizeof(sizeData));

        for (auto v : ivect) {
            int fval = flipEndianInt(v);
            ofileH.write(reinterpret_cast<char*>(&fval), sizeof(fval));
        }

        ofileH.write(reinterpret_cast<char*>(&sizeData), sizeof(sizeData));
        ofileH.close();
    }

    EclFile test1(filename);
    auto array = test1.get<int>(arrName);

    for (size_t n = 0; n < 10; n++) {
        BOOST_CHECK_EQUAL(array[n], ivect[n]);
    }
}


BOOST_AUTO_TEST_CASE(TestEclFile_BINARY) {

    std::string testFile="ECLFILE.INIT";

    // check that exception is thrown when input file doesn't exist

    BOOST_CHECK_THROW(EclFile file1("DUMMY.DAT") , std::exception );

    EclFile file1(testFile);

    // check that exeption is thrown when member function get is used with wrong type

    BOOST_CHECK_THROW(std::vector<int> vect1=file1.get<int>(2) , std::runtime_error );
    BOOST_CHECK_THROW(std::vector<int> vect1=file1.get<int>("PORV") , std::runtime_error );

    BOOST_CHECK_THROW(std::vector<float> vect1=file1.get<float>(0) , std::runtime_error );
    BOOST_CHECK_THROW(std::vector<float> vect1=file1.get<float>("ICON") , std::runtime_error );

    BOOST_CHECK_THROW(std::vector<double> vect1=file1.get<double>(0) , std::runtime_error );
    BOOST_CHECK_THROW(std::vector<double> vect1=file1.get<double>("KEYWORDS") , std::runtime_error );

    BOOST_CHECK_THROW(std::vector<bool> vect1=file1.get<bool>(0) , std::runtime_error );
    BOOST_CHECK_THROW(std::vector<bool> vect1=file1.get<bool>("XCON") , std::runtime_error );

    BOOST_CHECK_THROW(std::vector<std::string> vect1=file1.get<std::string>(0) , std::runtime_error );
    BOOST_CHECK_THROW(std::vector<std::string> vect1=file1.get<std::string>("XCON") , std::runtime_error );

    // check member function hasKey

    BOOST_CHECK_EQUAL(file1.hasKey("PORV"), true);
    BOOST_CHECK_EQUAL(file1.hasKey("XPORV"), false);

    // test member functon get, use size of vector to confirm that vectror is ok

    std::vector<int> vect1a=file1.get<int>(0);
    std::vector<int> vect1b=file1.get<int>("ICON");

    BOOST_CHECK_EQUAL(vect1a.size(), 1875U);
    BOOST_CHECK_EQUAL(vect1b.size(), 1875U);

    std::vector<bool> vect2a=file1.get<bool>(1);
    std::vector<bool> vect2b=file1.get<bool>("LOGIHEAD");

    BOOST_CHECK_EQUAL(vect2a.size(), 121U);
    BOOST_CHECK_EQUAL(vect2b.size(), 121U);

    std::vector<float> vect3a=file1.get<float>(2);
    std::vector<float> vect3b=file1.get<float>("PORV");

    BOOST_CHECK_EQUAL(vect3a.size(), 3146U);
    BOOST_CHECK_EQUAL(vect3b.size(), 3146U);

    std::vector<double> vect4a=file1.get<double>(3);
    std::vector<double> vect4b=file1.get<double>("XCON");

    BOOST_CHECK_EQUAL(vect4a.size(), 1740U);
    BOOST_CHECK_EQUAL(vect4b.size(), 1740U);

    std::vector<std::string> vect5a=file1.get<std::string>(4);
    std::vector<std::string> vect5b=file1.get<std::string>("KEYWORDS");

    BOOST_CHECK_EQUAL(vect5a.size(), 312U);
    BOOST_CHECK_EQUAL(vect5b.size(), 312U);
}


BOOST_AUTO_TEST_CASE(TestEclFile_FORMATTED) {

    std::string testFile1="ECLFILE.INIT";
    std::string testFile2="ECLFILE.FINIT";

    // loading data both from binary and formatted file. Check that
    // date vectors are identical

    EclFile file1(testFile1);
    file1.loadData();

    EclFile file2(testFile2);
    file2.loadData();

    std::vector<int> vect1a=file1.get<int>("ICON");
    std::vector<int> vect1b=file2.get<int>("ICON");

    BOOST_CHECK_EQUAL(vect1a.size(), vect1b.size());
    BOOST_CHECK(vect1a == vect1b);

    std::vector<float> vect2a=file1.get<float>("PORV");
    std::vector<float> vect2b=file2.get<float>("PORV");

    BOOST_CHECK_EQUAL(vect2a.size(), vect2b.size());
    BOOST_CHECK(vect2a == vect2b);

    std::vector<double> vect3a=file1.get<double>("XCON");
    std::vector<double> vect3b=file2.get<double>("XCON");

    BOOST_CHECK(vect3a == vect3b);

    std::vector<bool> vect4a=file1.get<bool>("LOGIHEAD");
    std::vector<bool> vect4b=file2.get<bool>("LOGIHEAD");

    BOOST_CHECK_EQUAL(vect4a.size(), vect4b.size());
    BOOST_CHECK(vect4a == vect4b);

    std::vector<std::string> vect5a=file1.get<std::string>("KEYWORDS");
    std::vector<std::string> vect5b=file2.get<std::string>("KEYWORDS");

    BOOST_CHECK_EQUAL(vect5a.size(), vect5b.size());
    BOOST_CHECK(vect5a == vect5b);

}


BOOST_AUTO_TEST_CASE(TestEclFile_IX) {

    // file MODEL1_IX.INIT is output from comercial simulator ix with
    // BASE_MODEL_1.DATA in opm-tests

    // binary representation of true value for data type LOGI is different
    // compared with output from Eclipse and OPM-Flow

    std::string testInitFile="MODEL1_IX.INIT";

    EclFile file1(testInitFile);
    file1.loadData();

    std::vector<bool> refLogihead = {true, true, false,  false, true, false, false, false, false, false, false,
         false, false, false, false, false, true, false,  true, false };

    auto logih = file1.get<bool>("LOGIHEAD");

    for (size_t n = 0 ; n < refLogihead.size(); n++)
        BOOST_CHECK_EQUAL(refLogihead[n], logih[n]);
}



BOOST_AUTO_TEST_CASE(TestEcl_Write_binary) {

    std::string inputFile="ECLFILE.INIT";
    std::string testFile="TEST.DAT";

    // loading vectors from binary file and write these back to a binary file1
    // compare input and output file and delete file.

    EclFile file1(inputFile);
    file1.loadData();

    std::vector<int> icon=file1.get<int>("ICON");
    std::vector<float> porv=file1.get<float>("PORV");
    std::vector<double> xcon=file1.get<double>("XCON");
    std::vector<bool> logihead=file1.get<bool>("LOGIHEAD");
    std::vector<std::string> keywords=file1.get<std::string>("KEYWORDS");

    // writing vectors to test file (TEST.DAT) using class EclOutput

    WorkArea work;
    {
        EclOutput eclTest(testFile, false);

        eclTest.write("ICON",icon);
        eclTest.write("LOGIHEAD",logihead);
        eclTest.write("PORV",porv);
        eclTest.write("XCON",xcon);
        eclTest.write("KEYWORDS",keywords);
        eclTest.write("ENDSOL",std::vector<char>());

    }
    work.copyIn(inputFile);
    BOOST_CHECK_EQUAL(compare_files(inputFile, testFile), true);
}

BOOST_AUTO_TEST_CASE(TestEcl_Write_formatted) {

    std::string inputFile="ECLFILE.FINIT";
    std::string testFile="TEST.FDAT";

    // loading vectors from formatted input file and write data back to a formatted file1
    // compare input and output file, and delete file.

    EclFile file1(inputFile, true);

    std::vector<int> icon = file1.get<int>("ICON");
    std::vector<float> porv = file1.get<float>("PORV");
    std::vector<double> xcon = file1.get<double>("XCON");
    std::vector<bool> logihead = file1.get<bool>("LOGIHEAD");
    std::vector<std::string> keywords = file1.get<std::string>("KEYWORDS");

    // writing vectors to test file (TEST.FDAT) using class EclOutput

    {
        WorkArea work;
        EclOutput eclTest(testFile, true);

        eclTest.write("ICON",icon);
        eclTest.write("LOGIHEAD",logihead);
        eclTest.write("PORV",porv);
        eclTest.write("XCON",xcon);
        eclTest.write("KEYWORDS",keywords);
        eclTest.write("ENDSOL",std::vector<char>());

        work.copyIn(inputFile);
        BOOST_CHECK_EQUAL(compare_files(inputFile, testFile), true);
    }
}

BOOST_AUTO_TEST_CASE(TestEcl_Write_formatted_not_finite) {
    WorkArea wa;
    std::vector<float>  float_vector{std::numeric_limits<float>::infinity()  , std::numeric_limits<float>::quiet_NaN()};
    std::vector<double> double_vector{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()};

    {
        EclOutput testfile("TEST.FINIT", true);
        testfile.write("FLOAT", float_vector);
        testfile.write("DOUBLE", double_vector);
    }

    EclFile file1("TEST.FINIT");
    file1.loadData();
    auto f = file1.get<float>("FLOAT");
    auto d = file1.get<double>("DOUBLE");
    BOOST_CHECK(std::isinf(f[0]));
    BOOST_CHECK(std::isinf(d[0]));
    BOOST_CHECK(std::isnan(f[1]));
    BOOST_CHECK(std::isnan(d[1]));

    BOOST_CHECK_EQUAL(file1.size(), 2U);
}


BOOST_AUTO_TEST_CASE(TestEcl_getList) {

    std::string inputFile="ECLFILE.INIT";
    std::string testFile="TEST.DAT";

    // use EclFile to read/open a binary file
    // Use API for class EclFile together with class EclOutput to write an
    // identical eclfile
    // EclFile::getList(), EclFile::get<T>(int)

    EclFile file1(inputFile);
    file1.loadData();

    WorkArea work;
    {
        EclOutput eclTest(testFile, false);

        auto arrayList = file1.getList();
        int n=0;
        for (auto array : arrayList) {
            std::string name = std::get<0>(array);
            eclArrType arrType = std::get<1>(array);

            if (arrType == INTE) {
                std::vector<int> vect = file1.get<int>(n);
                eclTest.write(name, vect);
            } else if (arrType == REAL) {
                std::vector<float> vect = file1.get<float>(n);
                eclTest.write(name, vect);
            } else if (arrType == DOUB) {
                std::vector<double> vect = file1.get<double>(n);
                eclTest.write(name, vect);
            } else if (arrType == LOGI) {
                std::vector<bool> vect = file1.get<bool>(n);
                eclTest.write(name, vect);
            } else if (arrType == CHAR) {
                std::vector<std::string> vect = file1.get<std::string>(n);
                eclTest.write(name, vect);
            } else if (arrType == MESS) {
                eclTest.write(name, std::vector<char>());
            } else {
                std::cout << "unknown type " << std::endl;
                exit(1);
            }

            n++;
        }
    }

    work.copyIn(inputFile);
    BOOST_CHECK_EQUAL(compare_files(inputFile, testFile), true);
}


BOOST_AUTO_TEST_CASE(TestEcl_Write_CHAR) {
    WorkArea work;
    std::string testFile1="TEST.FDAT";
    std::string testFile2="TEST2.DAT";

    std::vector<std::string> refStrList1 = {"This", "is", "a test.", "", "charact", "er >'<", "can be", "part of", "a string"};
    std::vector<std::string> refStrList2 = {"strings with length", "beyone 8 character","is also possible", "will use type C0nn"};

    {
        EclOutput eclTest(testFile1, true);
        eclTest.write("TEST1",refStrList1);
    }

    {
        EclFile file1(testFile1);
        std::vector<std::string> strList=file1.get<std::string>("TEST1");

        for (size_t n = 0; n < refStrList1.size(); n++) {
            BOOST_CHECK(refStrList1[n] == strList[n]);
        }

        auto arrayList =file1.getList();
        BOOST_CHECK(std::get<1>(arrayList[0]) == Opm::EclIO::CHAR);
    }

    // the next should automatically use C019
    {
        EclOutput eclTest(testFile1, true);
        eclTest.write("TEST2",refStrList2);
    }

    {
        EclFile file1(testFile1);
        std::vector<std::string> strList=file1.get<std::string>("TEST2");

        for (size_t n = 0; n < refStrList2.size(); n++) {
            BOOST_CHECK(refStrList2[n] == strList[n]);
        }

        auto arrayList =file1.getList();
        BOOST_CHECK(std::get<1>(arrayList[0]) == Opm::EclIO::C0NN);
    }

    // the next should automatically use C008 (and not CHAR)
    // will use C0nn since element size is specified in function call
    {
        EclOutput eclTest(testFile1, true);
        eclTest.write("TEST3",refStrList1, 8);
    }

    {
        EclFile file1(testFile1);
        std::vector<std::string> strList=file1.get<std::string>("TEST3");

        for (size_t n = 0; n < refStrList1.size(); n++) {
            BOOST_CHECK(refStrList1[n] == strList[n]);
        }

        auto arrayList =file1.getList();
        BOOST_CHECK(std::get<1>(arrayList[0]) == Opm::EclIO::C0NN);
    }

    // testing binary CHAR/C0nn format

    {
        EclOutput eclTest(testFile2, false);
        eclTest.write("TEST4",refStrList1);
    }

    {
        EclFile file1(testFile2);
        std::vector<std::string> strList=file1.get<std::string>("TEST4");

        for (size_t n = 0; n < refStrList1.size(); n++) {
            BOOST_CHECK(refStrList1[n] == strList[n]);
        }

        auto arrayList =file1.getList();
        BOOST_CHECK(std::get<1>(arrayList[0]) == Opm::EclIO::CHAR);
    }

    // the next should automatically use C019
    {
        EclOutput eclTest(testFile2, false);
        eclTest.write("TEST5",refStrList2);
    }

    {
        EclFile file1(testFile2);
        std::vector<std::string> strList=file1.get<std::string>("TEST5");

        for (size_t n = 0; n < refStrList2.size(); n++) {
            BOOST_CHECK(refStrList2[n] == strList[n]);
        }

        auto arrayList =file1.getList();
        BOOST_CHECK(std::get<1>(arrayList[0]) == Opm::EclIO::C0NN);
    }

    // the next should automatically use C008 (and not CHAR)
    // will use C0nn since element size is specified in function call
    {
        EclOutput eclTest(testFile2, false);
        eclTest.write("TEST6",refStrList1, 8);
    }

    {
        EclFile file1(testFile2);
        std::vector<std::string> strList=file1.get<std::string>("TEST6");

        for (size_t n = 0; n < refStrList1.size(); n++) {
            BOOST_CHECK(refStrList1[n] == strList[n]);
        }

        auto arrayList =file1.getList();
        BOOST_CHECK(std::get<1>(arrayList[0]) == Opm::EclIO::C0NN);
    }
}

BOOST_AUTO_TEST_CASE(CombinedVectorID) {
    BOOST_CHECK_EQUAL(combineSummaryNumbers(1, 2), 393'217);
    BOOST_CHECK_EQUAL(combineSummaryNumbers(10, 1), 360'458);

    {
        const auto [n1, n2] = splitSummaryNumber(393'217);
        BOOST_CHECK_EQUAL(n1, 1);
        BOOST_CHECK_EQUAL(n2, 2);
    }

    {
        const auto [n1, n2] = splitSummaryNumber(360'458);
        BOOST_CHECK_EQUAL(n1, 10);
        BOOST_CHECK_EQUAL(n2,  1);
    }
}
