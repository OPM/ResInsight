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

#include <opm/io/eclipse/EGrid.hpp>
#include <opm/io/eclipse/EInit.hpp>
#include <opm/common/utility/numeric/calculateCellVol.hpp>


#define BOOST_TEST_MODULE Test EGrid
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <tuple>

using Opm::EclIO::EGrid;

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


BOOST_AUTO_TEST_CASE(DimensAndIndices) {

    std::string testFile="SPE1CASE1.EGRID";

    EGrid grid1(testFile);

    int nAct=grid1.activeCells();
    int nTot=grid1.totalNumberOfCells();

    BOOST_CHECK_EQUAL(nAct,294);
    BOOST_CHECK_EQUAL(nTot,300);

    auto dim = grid1.dimension();

    BOOST_CHECK_EQUAL(dim[0],10);
    BOOST_CHECK_EQUAL(dim[1],10);
    BOOST_CHECK_EQUAL(dim[2],3);

    int globInd = grid1.global_index(3, 2, 1);

    BOOST_CHECK_EQUAL(globInd, 123);   // 10*10*1 + 10*2 + 3 = 100+20+3 = 123

    BOOST_CHECK_EQUAL(grid1.global_index(0, 0, 0), 0);
    BOOST_CHECK_EQUAL(grid1.global_index(dim[0] - 1, dim[1] - 1, dim[2] - 1), nTot - 1);

    // check global_index valid range, should throw exception if outside

    BOOST_CHECK_THROW(grid1.global_index(-1, -1, -1), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.global_index(0, -1, 0), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.global_index(0, 0, -1), std::invalid_argument);

    BOOST_CHECK_THROW(grid1.global_index(dim[0], 0, 0), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.global_index(dim[0], dim[1], 0), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.global_index(dim[0], dim[1], dim[2]), std::invalid_argument);

    // 6 inactive cells in first layer, actInd not same as glogInd
    // actInd and globInd are zero based indices

    int actInd = grid1.active_index(3,2,1);

    BOOST_CHECK_EQUAL(actInd, 117);   // global index 123, - 6 inactive
    BOOST_CHECK_EQUAL(grid1.active_index(0, 0, 0), 0);
    BOOST_CHECK_EQUAL(grid1.active_index(dim[0] - 1, dim[1] - 1, dim[2] - 1), nAct - 1);

    // check active_index valid range, should throw exception if outside

    BOOST_CHECK_THROW(grid1.active_index(-1, 0, 0), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.active_index(-1, -1, 0), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.active_index(-1, -1, -1), std::invalid_argument);

    BOOST_CHECK_THROW(grid1.active_index(dim[0], 0, 0), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.active_index(dim[0], dim[1], 0), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.active_index(dim[0], dim[1], dim[2]), std::invalid_argument);

    auto res = grid1.ijk_from_active_index(actInd);

    BOOST_CHECK_EQUAL(res[0], 3);
    BOOST_CHECK_EQUAL(res[1], 2);
    BOOST_CHECK_EQUAL(res[2], 1);

    BOOST_CHECK_THROW(grid1.ijk_from_active_index(-1), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.ijk_from_active_index(nAct), std::invalid_argument);

    res = grid1.ijk_from_global_index(globInd);

    BOOST_CHECK_EQUAL(res[0], 3);
    BOOST_CHECK_EQUAL(res[1], 2);
    BOOST_CHECK_EQUAL(res[2], 1);

    BOOST_CHECK_THROW(grid1.ijk_from_global_index(-1), std::invalid_argument);
    BOOST_CHECK_THROW(grid1.ijk_from_global_index(nTot), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(getCellCorners) {

    std::string testFile="SPE1CASE1.EGRID";

    std::array<double,8> ref_X = {3000,4000,3000,4000,3000,4000,3000,4000};
    std::array<double,8> ref_Y = {2000,2000,3000,3000,2000,2000,3000,3000};
    std::array<double,8> ref_Z = {8345,8345,8345,8345,8375,8375,8375,8375};

    EGrid grid1(testFile);

    std::array<double,8> X = {0.0};
    std::array<double,8> Y = {0.0};
    std::array<double,8> Z = {0.0};

    // cell 4,3,2 => zero based 3,2,1
    grid1.getCellCorners({3, 2, 1}, X, Y, Z);

    BOOST_CHECK_EQUAL(X == ref_X, true);
    BOOST_CHECK_EQUAL(Y == ref_Y, true);
    BOOST_CHECK_EQUAL(Z == ref_Z, true);

    X = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Y = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Z = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    int globInd=grid1.global_index(3,2,1);

    grid1.getCellCorners(globInd,X,Y,Z);

    BOOST_CHECK_EQUAL(X == ref_X, true);
    BOOST_CHECK_EQUAL(Y == ref_Y, true);
    BOOST_CHECK_EQUAL(Z == ref_Z, true);
}


BOOST_AUTO_TEST_CASE(lgr_1) {

    std::string testEgridFile = "LGR_TESTMOD.EGRID";
    std::string testInitFile = "LGR_TESTMOD.INIT";

    std::vector<EGrid::NNCentry> nnc_global_ref = {{0,0,1,0,1,0,8.52637}, {0,0,2,0,1,1,8.52637},
     {0,0,3,0,1,2,8.52637}, {0,0,4,0,1,3,8.52637}, {1,0,1,1,1,0,8.52637}, {1,0,2,1,1,1,8.52637},
     {1,0,3,1,1,2,8.52637}, {1,0,4,1,1,3,8.52637} };

    std::vector<EGrid::NNCentry> nnc_lgr1_ref = {{0,3,1,0,4,0,7.67373}, {0,3,2,0,4,1,7.67373},
      {0,3,3,0,4,2,7.67373}, {1,3,1,1,4,0,7.67373}, {1,3,2,1,4,1,7.67373}, {1,3,3,1,4,2,7.67373},
      {2,3,1,2,4,0,7.67373}, {2,3,2,2,4,1,7.67373}, {2,3,3,2,4,2,7.67373}, {3,3,1,3,4,0,7.67373},
      {3,3,2,3,4,1,7.67373}, {3,3,3,3,4,2,7.67373}};

    const std::vector<std::string> ref_lgr_list = {"LGR1", "LGR2"};

    const std::array<int, 3> ref_dim_global = {2,3,5};
    const std::array<int, 3> ref_dim_lgr1 = {4,8,4};
    const std::array<int, 3> ref_dim_lgr2 = {6,8,4};

    EGrid grid1(testEgridFile);

    auto nijk_global = grid1.dimension();
    BOOST_CHECK_EQUAL(nijk_global == ref_dim_global, true);

    auto lgr_list = grid1.list_of_lgrs();

    BOOST_CHECK_EQUAL(lgr_list.size(), 2);
    BOOST_CHECK_EQUAL(lgr_list[0], "LGR1");
    BOOST_CHECK_EQUAL(lgr_list[1], "LGR2");

    BOOST_CHECK_EQUAL(grid1.is_radial(), false);

    BOOST_CHECK_EQUAL(grid1.activeCells(), 30);
    BOOST_CHECK_EQUAL(grid1.totalNumberOfCells(), 30);

    std::array<double,8> global_X_1_1_0 = { 2099.985, 2199.969, 2099.985, 2199.969, 2099.974, 2199.958, 2099.974, 2199.958 };
    std::array<double,8> global_Y_1_1_0 = { 2100.000, 2100.000, 2200.000, 2200.000, 2100.000, 2100.000, 2200.000, 2200.000 };
    std::array<double,8> global_Z_1_1_0 = { 2002.745, 2004.490, 2002.745, 2004.490, 2005.245, 2006.990, 2005.245, 2006.990 };

    std::array<double,8> X = {0.0};
    std::array<double,8> Y = {0.0};
    std::array<double,8> Z = {0.0};

    // cell 2,2,1 => zero based 1,1,0
    grid1.getCellCorners({1, 1, 0}, X, Y, Z);

    for (size_t n = 0; n < 8; n++){
        BOOST_REQUIRE_CLOSE(global_X_1_1_0[n], X[n], 1e-3);
        BOOST_REQUIRE_CLOSE(global_Y_1_1_0[n], Y[n], 1e-3);
        BOOST_REQUIRE_CLOSE(global_Z_1_1_0[n], Z[n], 1e-3);
    }

    Opm::EclIO::EInit init1(testInitFile);

    {
        auto porv = init1.getInitData<float>("PORV");
        auto poro = init1.getInitData<float>("PORO");

        BOOST_REQUIRE_CLOSE(calculateCellVol(X, Y, Z), porv[3] / poro[3], 1e-2);
    }

    auto nnc_list = grid1.get_nnc_ijk();

    BOOST_CHECK_EQUAL(nnc_list.size(), nnc_global_ref.size());

    for (size_t n=0; n< nnc_list.size(); n++){
        BOOST_CHECK_EQUAL(std::get<0>(nnc_list[n]), std::get<0>(nnc_global_ref[n]));
        BOOST_CHECK_EQUAL(std::get<1>(nnc_list[n]), std::get<1>(nnc_global_ref[n]));
        BOOST_CHECK_EQUAL(std::get<2>(nnc_list[n]), std::get<2>(nnc_global_ref[n]));
        BOOST_CHECK_EQUAL(std::get<3>(nnc_list[n]), std::get<3>(nnc_global_ref[n]));
        BOOST_CHECK_EQUAL(std::get<4>(nnc_list[n]), std::get<4>(nnc_global_ref[n]));
        BOOST_CHECK_EQUAL(std::get<5>(nnc_list[n]), std::get<5>(nnc_global_ref[n]));
        BOOST_REQUIRE_CLOSE(std::get<6>(nnc_list[n]), std::get<6>(nnc_global_ref[n]), 1e-3);
    }

    // testing LGR1 - cartesian grid

    EGrid lgr1(testEgridFile, "LGR1");

    BOOST_CHECK_EQUAL(lgr1.is_radial(), false);

    auto nijk_lgr1 = lgr1.dimension();
    BOOST_CHECK_EQUAL(nijk_lgr1 == ref_dim_lgr1, true);

    BOOST_CHECK_EQUAL(lgr1.activeCells(), 128);
    BOOST_CHECK_EQUAL(lgr1.totalNumberOfCells(), 128);

    std::array<double,8> lgr1_X_1_2_0 = { 2124.983, 2149.979, 2124.984, 2149.980, 2124.978, 2149.974, 2124.979, 2149.975 };
    std::array<double,8> lgr1_Y_1_2_0 = { 2050.000, 2050.000, 2075.000, 2075.000, 2050.000, 2050.000, 2075.000, 2075.000 };
    std::array<double,8> lgr1_Z_1_2_0 = { 2002.182, 2002.618, 2002.182, 2002.618, 2003.431, 2003.868, 2003.431, 2003.868 };

    // cell 2,3,1 => zero based 1,2,0
    lgr1.getCellCorners({1, 2, 0}, X, Y, Z);

    for (size_t n = 0; n < 8; n++){
        BOOST_REQUIRE_CLOSE(lgr1_X_1_2_0[n], X[n], 1e-3);
        BOOST_REQUIRE_CLOSE(lgr1_Y_1_2_0[n], Y[n], 1e-3);
        BOOST_REQUIRE_CLOSE(lgr1_Z_1_2_0[n], Z[n], 1e-3);
    }

    {
        int act_ind = lgr1.active_index(1,2,0);
        auto porv = init1.getInitData<float>("PORV", "LGR1");
        auto poro = init1.getInitData<float>("PORO", "LGR1");

        BOOST_REQUIRE_CLOSE(calculateCellVol(X, Y, Z), porv[act_ind] / poro[act_ind], 1e-2);
    }

    auto nnc_list_lgr1 = lgr1.get_nnc_ijk();

    for (size_t n=0; n< nnc_list_lgr1.size(); n++){
        BOOST_CHECK_EQUAL(std::get<0>(nnc_list_lgr1[n]), std::get<0>(nnc_lgr1_ref[n]));
        BOOST_CHECK_EQUAL(std::get<1>(nnc_list_lgr1[n]), std::get<1>(nnc_lgr1_ref[n]));
        BOOST_CHECK_EQUAL(std::get<2>(nnc_list_lgr1[n]), std::get<2>(nnc_lgr1_ref[n]));
        BOOST_CHECK_EQUAL(std::get<3>(nnc_list_lgr1[n]), std::get<3>(nnc_lgr1_ref[n]));
        BOOST_CHECK_EQUAL(std::get<4>(nnc_list_lgr1[n]), std::get<4>(nnc_lgr1_ref[n]));
        BOOST_CHECK_EQUAL(std::get<5>(nnc_list_lgr1[n]), std::get<5>(nnc_lgr1_ref[n]));
        BOOST_REQUIRE_CLOSE(std::get<6>(nnc_list_lgr1[n]), std::get<6>(nnc_lgr1_ref[n]), 1e-3);
    }


    // testing LGR2 - radial grid

    EGrid lgr2(testEgridFile, "LGR2");

    BOOST_CHECK_EQUAL(lgr2.is_radial(), true);

    auto nijk_lgr2 = lgr2.dimension();
    BOOST_CHECK_EQUAL(nijk_lgr2 == ref_dim_lgr2, true);

    BOOST_CHECK_EQUAL(lgr2.activeCells(), 192);
    BOOST_CHECK_EQUAL(lgr2.totalNumberOfCells(), 192);

    std::array<double,8> lgr2_X_4_1_0 = { 8.469, 23.561, 0.000, 0.000, 8.469, 23.561, 0.000, 0.000 };
    std::array<double,8> lgr2_Y_4_1_0 = { 8.471, 23.564, 11.978, 33.322, 8.471, 23.564, 11.978, 33.322 };
    std::array<double,8> lgr2_Z_4_1_0 = { 2011.892, 2012.155, 2011.744, 2011.744, 2013.892, 2014.155, 2013.744, 2013.744 };

    // cell 5,2,1 => zero based 4,1,0
    lgr2.getCellCorners({4, 1, 0}, X, Y, Z);

    for (size_t n = 0; n < 8; n++){
        BOOST_CHECK_EQUAL(abs(lgr2_X_4_1_0[n]- X[n]) < 1e-3, true);
        BOOST_CHECK_EQUAL(abs(lgr2_Y_4_1_0[n]- Y[n]) < 1e-3, true);
        BOOST_CHECK_EQUAL(abs(lgr2_Z_4_1_0[n]- Z[n]) < 1e-3, true);
    }

    auto hostcells_ijk = lgr1.hostCellsIJK();
    auto hostcells_gind = lgr1.hostCellsGlobalIndex();

    for (size_t n = 0; n < hostcells_gind.size(); n++)
        BOOST_CHECK_EQUAL(grid1.ijk_from_global_index(hostcells_gind[n]) == hostcells_ijk[n], true);
}

