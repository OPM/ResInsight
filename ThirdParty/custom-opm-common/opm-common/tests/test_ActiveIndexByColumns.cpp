/*
  Copyright 2021 Equinor ASA

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

#define BOOST_TEST_MODULE Active_Index_by_Columns

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/ActiveIndexByColumns.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <array>
#include <utility>
#include <vector>

// =====================================================================

BOOST_AUTO_TEST_SUITE(Basic_Mapping)

BOOST_AUTO_TEST_CASE(Constructor)
{
    const auto cartDims = std::array<int,3>{ { 1, 1, 4 } };
    const auto actIJK = std::vector<std::array<int,3>> {
        { 0, 0, 0 },
        { 0, 0, 1 },
        { 0, 0, 3 },
    };

    const auto map = Opm::ActiveIndexByColumns { actIJK.size(), cartDims,
        [&actIJK](const std::size_t i)
    {
        return actIJK[i];
    }};

    auto map2 = map;
    const auto map3 = std::move(map2);
    BOOST_CHECK_MESSAGE(map3 == map, "Copied Map object must equal initial");
}

BOOST_AUTO_TEST_CASE(Single_Column)
{
    const auto cartDims = std::array<int,3>{ { 1, 1, 4 } };
    const auto actIJK = std::vector<std::array<int,3>> {
        { 0, 0, 0 },
        { 0, 0, 1 },
        { 0, 0, 3 },
    };

    const auto map = Opm::ActiveIndexByColumns { actIJK.size(), cartDims,
        [&actIJK](const std::size_t i)
    {
        return actIJK[i];
    }};

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(0), 0);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(1), 1);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(2), 2);
}

BOOST_AUTO_TEST_CASE(Two_Columns)
{
    const auto cartDims = std::array<int,3>{ { 2, 1, 4 } };
    const auto actIJK = std::vector<std::array<int,3>> {
        { 0, 0, 0 },  { 1, 0, 0 },
        { 0, 0, 1 },  { 1, 0, 1 },
                      { 1, 0, 2 },
        { 0, 0, 3 },  { 1, 0, 3 },
    };

    const auto map = Opm::ActiveIndexByColumns { actIJK.size(), cartDims,
        [&actIJK](const std::size_t i)
    {
        return actIJK[i];
    }};

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(0), 0);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(1), 3);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(2), 1);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(3), 4);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(4), 5);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(5), 2);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(6), 6);
}

BOOST_AUTO_TEST_CASE(Four_Columns)
{
    const auto cartDims = std::array<int,3>{ { 2, 2, 4 } };
    const auto actIJK = std::vector<std::array<int,3>> {
        //   0             2             1             3
        { 0, 0, 0 },  { 1, 0, 0 },  { 0, 1, 0 },  { 1, 1, 0 },
        { 0, 0, 1 },  { 1, 0, 1 },  { 0, 1, 1 },
                      { 1, 0, 2 },                { 1, 1, 2 },
        { 0, 0, 3 },  { 1, 0, 3 },  { 0, 1, 3 },  { 1, 1, 3 },
    };

    const auto map = Opm::ActiveIndexByColumns { actIJK.size(), cartDims,
        [&actIJK](const std::size_t i)
    {
        return actIJK[i];
    }};

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 0),  0);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 1),  6);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 2),  3);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 3), 10);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 4),  1);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 5),  7);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 6),  4);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 7),  8);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 8), 11);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 9),  2);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(10),  9);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(11),  5);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(12), 12);
}

BOOST_AUTO_TEST_SUITE_END()     // Basic_Mapping

// =====================================================================

BOOST_AUTO_TEST_SUITE(Grid_Based)

namespace {
    std::vector<double> coord_3x3x3()
    {
        return {
            0.0, 0.0, 0.0,   0.0, 0.0, 0.0,   1.0, 0.0, 0.0,   1.0, 0.0, 0.0,   2.0, 0.0, 0.0,   2.0, 0.0, 0.0,   3.0, 0.0, 0.0,   3.0, 0.0, 0.0,
            0.0, 1.0, 0.0,   0.0, 1.0, 0.0,   1.0, 1.0, 0.0,   1.0, 1.0, 0.0,   2.0, 1.0, 0.0,   2.0, 1.0, 0.0,   3.0, 1.0, 0.0,   3.0, 1.0, 0.0,
            0.0, 2.0, 0.0,   0.0, 2.0, 0.0,   1.0, 2.0, 0.0,   1.0, 2.0, 0.0,   2.0, 2.0, 0.0,   2.0, 2.0, 0.0,   3.0, 2.0, 0.0,   3.0, 2.0, 0.0,
            0.0, 3.0, 0.0,   0.0, 3.0, 0.0,   1.0, 3.0, 0.0,   1.0, 3.0, 0.0,   2.0, 3.0, 0.0,   2.0, 3.0, 0.0,   3.0, 3.0, 0.0,   3.0, 3.0, 0.0,
        };
    }

    std::vector<double> zcorn_3x3x3()
    {
        return {
            // Top, layer 1
            0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0, //  0.. 2
            0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0, //  3.. 5
            0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0,    0.0, 0.0, //  6.. 8

            // Bottom, layer 1
            1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0, //  0.. 2
            1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0, //  3.. 5
            1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0, //  6.. 8

            // Top, layer 2
            1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0, //  9..11
            1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0, // 12..14
            1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0,    1.0, 1.0, // 15..17

            // Bottom, layer 2
            2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0, //  9..11
            2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0, // 12..14
            2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0, // 15..17

            // Top, layer 3
            2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0, // 18..20
            2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0, // 21..23
            2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0,    2.0, 2.0, // 24..26

            // Bottom, layer 3
            3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0, // 18..20
            3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0, // 21..23
            3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0,    3.0, 3.0, // 24..26
        };
    }

    std::vector<int> actnum_3x3x3_exclude_centre_cell()
    {
        return {
            1, 1, 1,
            1, 1, 1,
            1, 1, 1,

            1, 1, 1,
            1, 0, 1,
            1, 1, 1,

            1, 1, 1,
            1, 1, 1,
            1, 1, 1,
        };
    }

    std::vector<int> actnum_3x3x3_exclude_centre_column()
    {
        return {
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,

            1, 1, 1,
            1, 0, 1,
            1, 1, 1,

            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
        };
    }

    std::vector<int> actnum_3x3x3_exclude_diagonals()
    {
        return {
            0, 1, 0,
            1, 1, 1,
            0, 1, 0,

            1, 1, 1,
            1, 0, 1,
            1, 1, 1,

            0, 1, 0,
            1, 1, 1,
            0, 1, 0,
        };
    }
}

// K = 0
// +------+------+------+
// |   6  |  15  |  24  |
// +------+------+------+
// |   3  |  12  |  21  |
// +------+------+------+
// |   0  |   9  |  18  |
// +------+------+------+
//
// K = 1
// +------+------+------+
// |   7  |  16  |  25  |
// +------+------+------+
// |   4  |  13  |  22  |
// +------+------+------+
// |   1  |  10  |  19  |
// +------+------+------+
//
// K = 2
// +------+------+------+
// |   8  |  17  |  26  |
// +------+------+------+
// |   5  |  14  |  23  |
// +------+------+------+
// |   2  |  11  |  20  |
// +------+------+------+
BOOST_AUTO_TEST_CASE(Cube_3x3x3_Full)
{
    const auto grid = Opm::EclipseGrid {{3, 3, 3}, coord_3x3x3(), zcorn_3x3x3() };

    const auto map = Opm::buildColumnarActiveIndexMappingTables(grid);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 0),  0);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 1),  9);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 2), 18);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 3),  3);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 4), 12);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 5), 21);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 6),  6);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 7), 15);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 8), 24);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 9),  1);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(10), 10);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(11), 19);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(12),  4);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(13), 13);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(14), 22);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(15),  7);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(16), 16);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(17), 25);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(18),  2);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(19), 11);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(20), 20);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(21),  5);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(22), 14);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(23), 23);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(24),  8);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(25), 17);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(26), 26);
}

// K = 0
// +------+------+------+
// |   6  |  14  |  23  |
// +------+------+------+
// |   3  |  12  |  20  |
// +------+------+------+
// |   0  |   9  |  17  |
// +------+------+------+
//
// K = 1
// +------+------+------+
// |   7  |  15  |  24  |
// +------+------+------+
// |   4  | :::: |  21  |
// +------+------+------+
// |   1  |  10  |  18  |
// +------+------+------+
//
// K = 2
// +------+------+------+
// |   8  |  16  |  25  |
// +------+------+------+
// |   5  |  13  |  22  |
// +------+------+------+
// |   2  |  11  |  19  |
// +------+------+------+
BOOST_AUTO_TEST_CASE(Cube_3x3x3_exclude_centre_cell)
{
    const auto actnum = actnum_3x3x3_exclude_centre_cell();
    const auto grid = Opm::EclipseGrid {{3, 3, 3}, coord_3x3x3(), zcorn_3x3x3(), actnum.data() };

    const auto map = Opm::buildColumnarActiveIndexMappingTables(grid);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 0),  0);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 1),  9);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 2), 17);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 3),  3);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 4), 12);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 5), 20);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 6),  6);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 7), 14);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 8), 23);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 9),  1);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(10), 10);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(11), 18);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(12),  4);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(13), 21);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(14),  7);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(15), 15);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(16), 24);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(17),  2);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(18), 11);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(19), 19);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(20),  5);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(21), 13);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(22), 22);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(23),  8);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(24), 16);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(25), 25);
}

// K = 0
// +------+------+------+
// |   6  |  12  |  21  |
// +------+------+------+
// |   3  | :::: |  18  |
// +------+------+------+
// |   0  |   9  |  15  |
// +------+------+------+
//
// K = 1
// +------+------+------+
// |   7  |  13  |  22  |
// +------+------+------+
// |   4  | :::: |  19  |
// +------+------+------+
// |   1  |  10  |  16  |
// +------+------+------+
//
// K = 2
// +------+------+------+
// |   8  |  14  |  23  |
// +------+------+------+
// |   5  | :::: |  20  |
// +------+------+------+
// |   2  |  11  |  17  |
// +------+------+------+
BOOST_AUTO_TEST_CASE(Cube_3x3x3_exclude_centre_column)
{
    const auto actnum = actnum_3x3x3_exclude_centre_column();
    const auto grid = Opm::EclipseGrid {{3, 3, 3}, coord_3x3x3(), zcorn_3x3x3(), actnum.data() };

    const auto map = Opm::buildColumnarActiveIndexMappingTables(grid);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 0),  0);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 1),  9);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 2), 15);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 3),  3);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 4), 18);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 5),  6);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 6), 12);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 7), 21);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 8),  1);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 9), 10);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(10), 16);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(11),  4);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(12), 19);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(13),  7);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(14), 13);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(15), 22);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(16),  2);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(17), 11);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(18), 17);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(19),  5);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(20), 20);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(21),  8);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(22), 14);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(23), 23);
}

// K = 0
// +------+------+------+
// | :::: |  10  | :::: |
// +------+------+------+
// |   1  |   8  |  14  |
// +------+------+------+
// | :::: |   5  | :::: |
// +------+------+------+
//
// K = 1
// +------+------+------+
// |   4  |  11  |  17  |
// +------+------+------+
// |   2  | :::: |  15  |
// +------+------+------+
// |   0  |   6  |  13  |
// +------+------+------+
//
// K = 2
// +------+------+------+
// | :::: |  12  | :::: |
// +------+------+------+
// |   3  |   9  |  16  |
// +------+------+------+
// | :::: |   7  | :::: |
// +------+------+------+
BOOST_AUTO_TEST_CASE(Cube_3x3x3_exclude_diagonals)
{
    const auto actnum = actnum_3x3x3_exclude_diagonals();
    const auto grid = Opm::EclipseGrid {{3, 3, 3}, coord_3x3x3(), zcorn_3x3x3(), actnum.data() };

    const auto map = Opm::buildColumnarActiveIndexMappingTables(grid);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 0),  5);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 1),  1);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 2),  8);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 3), 14);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 4), 10);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 5),  0);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 6),  6);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 7), 13);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 8),  2);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex( 9), 15);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(10),  4);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(11), 11);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(12), 17);

    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(13),  7);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(14),  3);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(15),  9);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(16), 16);
    BOOST_CHECK_EQUAL(map.getColumnarActiveIndex(17), 12);
}

BOOST_AUTO_TEST_SUITE_END()     // Grid_Based
