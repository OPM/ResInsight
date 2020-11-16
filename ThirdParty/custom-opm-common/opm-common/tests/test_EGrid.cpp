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
