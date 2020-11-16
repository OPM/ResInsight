/*
  Copyright 2018 Statoil IT

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

#define BOOST_TEST_MODULE Windowed_Array

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/WindowedArray.hpp>

#include <exception>
#include <iterator>
#include <stdexcept>
#include <vector>

BOOST_AUTO_TEST_SUITE(WriteOperations)

BOOST_AUTO_TEST_CASE(EmptyArray)
{
    using Wa = Opm::RestartIO::Helpers::WindowedArray<int>;
    using Wm = Opm::RestartIO::Helpers::WindowedMatrix<int>;
    BOOST_CHECK_NO_THROW( Wa( Wa::NumWindows{ 0 }, Wa::WindowSize{ 1 }) );
    BOOST_CHECK_NO_THROW( Wm( Wm::NumRows{ 0 }, Wm::NumCols{ 2 }, Wm::WindowSize{ 3 } ));

    BOOST_CHECK_THROW( Wa(Wa::NumWindows{ 5 }, Wa::WindowSize{ 0 }), std::invalid_argument);
    BOOST_CHECK_THROW( Wm(Wm::NumRows{ 3 }, Wm::NumCols{ 0 }, Wm::WindowSize{ 4 } ), std::invalid_argument);
    BOOST_CHECK_THROW( Wm(Wm::NumRows{ 3 }, Wm::NumCols{ 2 }, Wm::WindowSize{ 0 } ), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(Array)
{
    using Wa = Opm::RestartIO::Helpers::WindowedArray<int>;

    auto wa = Wa{ Wa::NumWindows{ 5 }, Wa::WindowSize{ 7 } };

    for (auto m = wa.numWindows(), i = 0*m; i < m; ++i) {
        auto w = wa[i];

        std::fill(std::begin(w), std::end(w), 10*i);
    }

    BOOST_CHECK_EQUAL(wa.numWindows(), Wa::Idx{5});
    BOOST_CHECK_EQUAL(wa.windowSize(), Wa::Idx{7});

    {
        const auto expect = std::vector<int>{
          // 0   1   2   3   4   5   6
             0,  0,  0,  0,  0,  0,  0, // 0
            10, 10, 10, 10, 10, 10, 10, // 1
            20, 20, 20, 20, 20, 20, 20, // 2
            30, 30, 30, 30, 30, 30, 30, // 3
            40, 40, 40, 40, 40, 40, 40, // 4
        };

        const auto& actual = wa.data();

        BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(actual), std::end(actual),
                                      std::begin(expect), std::end(expect));
    }

    for (auto m = wa.numWindows(), i = 0*m; i < m; ++i) {
        auto w = wa[i];

        for (auto n = w.size(), j = 0*n; j < n; ++j) {
            w[j] = 10*i - 3*j;
        }
    }

    {
        const auto expect = std::vector<int>{
          // 0    1    2    3    4    5    6
             0, - 3, - 6, - 9, -12, -15, -18, // 0
            10,   7,   4,   1, - 2, - 5, - 8, // 1
            20,  17,  14,  11,   8,   5,   2, // 2
            30,  27,  24,  21,  18,  15,  12, // 3
            40,  37,  34,  31,  28,  25,  22, // 4
        };

        const auto& actual = wa.data();

        BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(actual), std::end(actual),
                                      std::begin(expect), std::end(expect));
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(Matrix)
{
    using Wm = Opm::RestartIO::Helpers::WindowedMatrix<int>;

    auto wm = Wm{ Wm::NumRows{ 3 }, Wm::NumCols{ 2 }, Wm::WindowSize{ 4 } };

    BOOST_CHECK_EQUAL(wm.numCols(), Wm::Idx{2});
    BOOST_CHECK_EQUAL(wm.numRows(), Wm::Idx{3});

    for (auto m = wm.numRows(), i = 0*m; i < m; ++i) {
        for (auto n = wm.numCols(), j = 0*n; j < n; ++j) {
            auto w = wm(i, j);

            std::fill(std::begin(w), std::end(w), 100*i + 10*j);
        }
    }

    {
        const auto expect = std::vector<int> {
              0,   0,   0,   0,    10,  10,  10,  10,
            100, 100, 100, 100,   110, 110, 110, 110,
            200, 200, 200, 200,   210, 210, 210, 210,
        };

        const auto& actual = wm.data();

        BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(actual), std::end(actual),
                                      std::begin(expect), std::end(expect));
    }

    for (auto m = wm.numRows(), i = 0*m; i < m; ++i) {
        for (auto n = wm.numCols(), j = 0*n; j < n; ++j) {
            auto w = wm(i, j);

            for (auto sz = w.size(), k = 0*sz; k < sz; ++k) {
                w[k] = 100*i + 10*j - 13*k;
            }
        }
    }

    {
        const auto expect = std::vector<int> {
              0, - 13, - 26, - 39,       10, -  3, - 16, - 29,
            100,   87,   74,   61,      110,   97,   84,   71,
            200,  187,  174,  161,      210,  197,  184,  171,
        };

        const auto& actual = wm.data();

        BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(actual), std::end(actual),
                                      std::begin(expect), std::end(expect));
    }
}

BOOST_AUTO_TEST_SUITE_END ()
