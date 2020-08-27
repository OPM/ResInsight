/*
  Copyright 2017 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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
#include "config.h"

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_LINEARISEDOUTPUTTABLE

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/LinearisedOutputTable.hpp>

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace {
    template <class Collection1, class Collection2>
    void check_is_close(const Collection1& c1, const Collection2& c2)
    {
        BOOST_REQUIRE_EQUAL(c1.size(), c2.size());

        if (! c1.empty()) {
            auto i1 = c1.begin(), e1 = c1.end();
            auto i2 = c2.begin();

            for (; i1 != e1; ++i1, ++i2) {
                BOOST_CHECK_CLOSE(*i1, *i2, 1.0e-10);
            }
        }
    }

    std::vector<double>
    makeTable(const std::size_t             ncol,
              std::initializer_list<double> data)
    {
        auto result = std::vector<double>(data.size(), 0.0);
        const auto nrows = data.size() / ncol;

        auto di = std::begin(data);
        for (auto i = 0*nrows; i < nrows; ++i) {
            for (auto j = 0*ncol; j < ncol; ++j, ++di) {
                result[i + j*nrows] = *di;
            }
        }

        return result;
    }

    std::vector<double> norne_pvto_1_1_incl_der()
    {
        return makeTable(5, {
                5.00e+01,  9.040365230755323e-01,  7.661326466741799e-01,  2.000000000000000e+20,  2.000000000000000e+20,
                7.50e+01,  9.077375549181221e-01,  7.279370929575959e-01,  1.480412737035941e-04, -1.527822148663360e-03,
                1.00e+02,  9.112115468727220e-01,  6.929365375457962e-01,  1.389596781839941e-04, -1.400022216471988e-03,
                1.25e+02,  9.144863787253888e-01,  6.607560539923331e-01,  1.309932741066744e-04, -1.287219342138526e-03,
                1.50e+02,  9.175658812302724e-01,  6.314975094496025e-01,  1.231801001953414e-04, -1.170341781709223e-03,
            });
    }

    std::vector<double> norne_pvto_1_2_incl_der()
    {
        return makeTable(5, {
                7.00e+01,  8.887150957146157e-01,  8.336914593945738e-01, -2.015977284331128e-03,  8.889317463209720e-03,
                9.50e+01,  8.924826189009969e-01,  7.940236822962605e-01,  1.507009274552473e-04, -1.586711083932530e-03,
                1.20e+02,  8.960252320705352e-01,  7.580585719716880e-01,  1.417045267815320e-04, -1.438604412982900e-03,
                1.45e+02,  8.993533649306149e-01,  7.247005358022682e-01,  1.331253144031885e-04, -1.334321446776792e-03,
                1.70e+02,  9.024944947835819e-01,  6.942265344489091e-01,  1.256451941186798e-04, -1.218960054134364e-03,
            });
    }

    std::vector<double> norne_pvto_1_3_incl_der()
    {
        return makeTable(5, {
                9.00e+01,  8.736829229935872e-01,  9.063100860929328e-01, -1.922272726474237e-03,  9.286269398767148e-03,
                1.15e+02,  8.775085776463464e-01,  8.653930746019195e-01,  1.530261861103677e-04, -1.636680459640534e-03,
                1.40e+02,  8.811038468993955e-01,  8.281051192663491e-01,  1.438107701219638e-04, -1.491518213422815e-03,
                1.65e+02,  8.844861135680170e-01,  7.932610884018090e-01,  1.352906667448606e-04, -1.393761234581605e-03,
                1.90e+02,  8.876816418559648e-01,  7.613050101680658e-01,  1.278211315179112e-04, -1.278243129349725e-03,
            });
    }

    std::vector<double> norne_sgfn_incl_der()
    {
        return makeTable(5, {
                                     0,                      0,   0,                        0,  0,
                 5.000000000000000e-02,  1.655000000000000e-03,   0,    3.310000000000000e-02,  0,
                 1.000000000000000e-01,  6.913000000000000e-03,   0,    1.051600000000000e-01,  0,
                 1.500000000000000e-01,  1.621300000000000e-02,   0,    1.860000000000001e-01,  0,
                 2.000000000000000e-01,  2.999000000000000e-02,   0,    2.755399999999998e-01,  0,
                 2.500000000000000e-01,  4.865500000000000e-02,   0,    3.733000000000000e-01,  0,
                 3.000000000000000e-01,  7.257300000000000e-02,   0,    4.783600000000001e-01,  0,
                 3.500000000000000e-01,  1.020460000000000e-01,   0,    5.894600000000001e-01,  0,
                 4.000000000000000e-01,  1.372870000000000e-01,   0,    7.048199999999992e-01,  0,
                 4.500000000000000e-01,  1.784020000000000e-01,   0,    8.223000000000005e-01,  0,
                 5.000000000000000e-01,  2.253680000000000e-01,   0,    9.393200000000004e-01,  0,
                 5.500000000000000e-01,  2.780300000000000e-01,   0,    1.053239999999999e+00,  0,
                 6.000000000000000e-01,  3.360930000000000e-01,   0,    1.161260000000001e+00,  0,
                 6.500000000000000e-01,  3.991350000000000e-01,   0,    1.260840000000000e+00,  0,
                 7.000000000000000e-01,  4.666310000000000e-01,   0,    1.349920000000002e+00,  0,
                 7.500000000000000e-01,  5.380000000000000e-01,   0,    1.427379999999999e+00,  0,
                 8.000000000000000e-01,  6.126650000000000e-01,   0,    1.493299999999998e+00,  0,
                 8.500000000000000e-01,  6.901690000000000e-01,   0,    1.550080000000002e+00,  0,
                 9.000000000000000e-01,  7.703950000000001e-01,   0,    1.604519999999999e+00,  0,
                 9.500000000000000e-01,  8.542180000000000e-01,   0,    1.676460000000002e+00,  0,
                 9.999000000000000e-01,  9.499000000000000e-01,   0,    1.917474949899796e+00,  0,
                 1.000000000000000e+00,  9.500000000000000e-01,   0,    1.000000000000000e+00,  0,
             });
    }
} // Anonymous

// ---------------------------------------------------------------------
// Constructors

BOOST_AUTO_TEST_SUITE (Basic_Operations)

BOOST_AUTO_TEST_CASE (Construct_Defaulted_FillVal)
{
    const auto numTables  = std::size_t{2};
    const auto numPrimary = std::size_t{3};
    const auto numRows    = std::size_t{4};
    const auto numCols    = std::size_t{5};

    auto linTable = ::Opm::LinearisedOutputTable {
        numTables, numPrimary, numRows, numCols
    };

    const auto expect_initial = std::vector<double>(
        numTables * numPrimary * numRows * numCols, 1.0e20);

    check_is_close(linTable.getData(), expect_initial);
}

BOOST_AUTO_TEST_CASE (Construct_UserDefined_FillVal)
{
    const auto numTables  = std::size_t{2};
    const auto numPrimary = std::size_t{3};
    const auto numRows    = std::size_t{4};
    const auto numCols    = std::size_t{5};
    const auto fillVal    = 1.234e-5;

    auto linTable = ::Opm::LinearisedOutputTable {
        numTables, numPrimary, numRows, numCols, fillVal
    };

    const auto expect_initial = std::vector<double>(
        numTables * numPrimary * numRows * numCols, fillVal);

    check_is_close(linTable.getData(), expect_initial);
}

BOOST_AUTO_TEST_SUITE_END ()

// ---------------------------------------------------------------------
// Saturation Functions

BOOST_AUTO_TEST_SUITE (Tabulated_Functions)

BOOST_AUTO_TEST_CASE (SGFN)
{
    const auto inputData = norne_sgfn_incl_der();
    const auto tableRows = inputData.size() / 5;

    const auto numTables  = std::size_t{ 1};
    const auto numPrimary = std::size_t{ 1};
    const auto numRows    = std::size_t{30};
    const auto numCols    = std::size_t{ 5};

    auto sgfn = ::Opm::LinearisedOutputTable {
        numTables, numPrimary, numRows, numCols
    };

    // Sg
    {
        const auto tableID = std::size_t{0};
        const auto primID  = std::size_t{0};
        const auto colID   = std::size_t{0};

        std::copy(inputData.begin() + 0*tableRows,
                  inputData.begin() + 1*tableRows,
                  sgfn.column(tableID, primID, colID));
    }

    // Krg
    {
        const auto tableID = std::size_t{0};
        const auto primID  = std::size_t{0};
        const auto colID   = std::size_t{1};

        std::copy(inputData.begin() + 1*tableRows,
                  inputData.begin() + 2*tableRows,
                  sgfn.column(tableID, primID, colID));
    }

    // Pcgo
    {
        const auto tableID = std::size_t{0};
        const auto primID  = std::size_t{0};
        const auto colID   = std::size_t{2};

        std::copy(inputData.begin() + 2*tableRows,
                  inputData.begin() + 3*tableRows,
                  sgfn.column(tableID, primID, colID));
    }

    // d[Krg]/dSg
    {
        const auto tableID = std::size_t{0};
        const auto primID  = std::size_t{0};
        const auto colID   = std::size_t{3};

        std::copy(inputData.begin() + 3*tableRows,
                  inputData.begin() + 4*tableRows,
                  sgfn.column(tableID, primID, colID));
    }

    // d[Pcgo]/dSg
    {
        const auto tableID = std::size_t{0};
        const auto primID  = std::size_t{0};
        const auto colID   = std::size_t{4};

        std::copy(inputData.begin() + 4*tableRows,
                  inputData.begin() + 5*tableRows,
                  sgfn.column(tableID, primID, colID));
    }

    const auto expect = makeTable(5,
        {
                    0,              0,         0,                        0,         0,
            5.000e-02,    1.65500e-03,         0,    3.310000000000000e-02,         0,
            1.000e-01,    6.91300e-03,         0,    1.051600000000000e-01,         0,
            1.500e-01,    1.62130e-02,         0,    1.860000000000001e-01,         0,
            2.000e-01,    2.99900e-02,         0,    2.755399999999998e-01,         0,
            2.500e-01,    4.86550e-02,         0,    3.733000000000000e-01,         0,
            3.000e-01,    7.25730e-02,         0,    4.783600000000001e-01,         0,
            3.500e-01,    1.02046e-01,         0,    5.894600000000001e-01,         0,
            4.000e-01,    1.37287e-01,         0,    7.048199999999992e-01,         0,
            4.500e-01,    1.78402e-01,         0,    8.223000000000005e-01,         0,
            5.000e-01,    2.25368e-01,         0,    9.393200000000004e-01,         0,
            5.500e-01,    2.78030e-01,         0,    1.053239999999999e+00,         0,
            6.000e-01,    3.36093e-01,         0,    1.161260000000001e+00,         0,
            6.500e-01,    3.99135e-01,         0,    1.260840000000000e+00,         0,
            7.000e-01,    4.66631e-01,         0,    1.349920000000002e+00,         0,
            7.500e-01,    5.38000e-01,         0,    1.427379999999999e+00,         0,
            8.000e-01,    6.12665e-01,         0,    1.493299999999998e+00,         0,
            8.500e-01,    6.90169e-01,         0,    1.550080000000002e+00,         0,
            9.000e-01,    7.70395e-01,         0,    1.604519999999999e+00,         0,
            9.500e-01,    8.54218e-01,         0,    1.676460000000002e+00,         0,
            9.999e-01,    9.49900e-01,         0,    1.917474949899796e+00,         0,
            1.000e+00,    9.50000e-01,         0,    1.000000000000000e+00,         0,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
            1.000e+20,    1.00000e+20,    1.e+20,    1.000000000000000e+20,    1.e+20,
        });

    check_is_close(sgfn.getData(), expect);
}

// ---------------------------------------------------------------------
// PVT

BOOST_AUTO_TEST_CASE (PVTO)
{
    const auto inputData = std::vector< std::vector<double> >
        {
            norne_pvto_1_1_incl_der(),
            norne_pvto_1_2_incl_der(),
            norne_pvto_1_3_incl_der(),
        };

    const auto numTables  = std::size_t{1};
    const auto numPrimary = std::size_t{5};
    const auto numRows    = std::size_t{8};
    const auto numCols    = std::size_t{5};

    auto pvto = ::Opm::LinearisedOutputTable {
        numTables, numPrimary, numRows, numCols
    };

    {
        auto primID = std::size_t{0};
        for (const auto& subTab : inputData) {
            const auto tableRows = subTab.size() / numCols;

            // Po
            {
                const auto tableID = std::size_t{0};
                const auto colID   = std::size_t{0};

                std::copy(subTab.begin() + 0*tableRows,
                          subTab.begin() + 1*tableRows,
                          pvto.column(tableID, primID, colID));
            }

            // 1/Bo
            {
                const auto tableID = std::size_t{0};
                const auto colID   = std::size_t{1};

                std::copy(subTab.begin() + 1*tableRows,
                          subTab.begin() + 2*tableRows,
                          pvto.column(tableID, primID, colID));
            }

            // 1/(Bo*mu_o)
            {
                const auto tableID = std::size_t{0};
                const auto colID   = std::size_t{2};

                std::copy(subTab.begin() + 2*tableRows,
                          subTab.begin() + 3*tableRows,
                          pvto.column(tableID, primID, colID));
            }

            // d[1/Bo]/dPo
            {
                const auto tableID = std::size_t{0};
                const auto colID   = std::size_t{3};

                std::copy(subTab.begin() + 3*tableRows,
                          subTab.begin() + 4*tableRows,
                          pvto.column(tableID, primID, colID));
            }

            // d[1/(Bo*mu_o)]/dPo
            {
                const auto tableID = std::size_t{0};
                const auto colID   = std::size_t{4};

                std::copy(subTab.begin() + 4*tableRows,
                          subTab.begin() + 5*tableRows,
                          pvto.column(tableID, primID, colID));
            }

            primID += 1;
        }
    }

    const auto expect = makeTable(5, {
        5.000000000000000e+01,    9.040365230755323e-01,    7.661326466741799e-01,    2.000000000000000e+20,    2.000000000000000e+20,
        7.500000000000000e+01,    9.077375549181221e-01,    7.279370929575959e-01,    1.480412737035941e-04,   -1.527822148663360e-03,
        1.000000000000000e+02,    9.112115468727220e-01,    6.929365375457962e-01,    1.389596781839941e-04,   -1.400022216471988e-03,
        1.250000000000000e+02,    9.144863787253888e-01,    6.607560539923331e-01,    1.309932741066744e-04,   -1.287219342138526e-03,
        1.500000000000000e+02,    9.175658812302724e-01,    6.314975094496025e-01,    1.231801001953414e-04,   -1.170341781709223e-03,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        7.000000000000000e+01,    8.887150957146157e-01,    8.336914593945738e-01,   -2.015977284331128e-03,    8.889317463209720e-03,
        9.500000000000000e+01,    8.924826189009969e-01,    7.940236822962605e-01,    1.507009274552473e-04,   -1.586711083932530e-03,
        1.200000000000000e+02,    8.960252320705352e-01,    7.580585719716880e-01,    1.417045267815320e-04,   -1.438604412982900e-03,
        1.450000000000000e+02,    8.993533649306149e-01,    7.247005358022682e-01,    1.331253144031885e-04,   -1.334321446776792e-03,
        1.700000000000000e+02,    9.024944947835819e-01,    6.942265344489091e-01,    1.256451941186798e-04,   -1.218960054134364e-03,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        9.000000000000000e+01,    8.736829229935872e-01,    9.063100860929328e-01,   -1.922272726474237e-03,    9.286269398767148e-03,
        1.150000000000000e+02,    8.775085776463464e-01,    8.653930746019195e-01,    1.530261861103677e-04,   -1.636680459640534e-03,
        1.400000000000000e+02,    8.811038468993955e-01,    8.281051192663491e-01,    1.438107701219638e-04,   -1.491518213422815e-03,
        1.650000000000000e+02,    8.844861135680170e-01,    7.932610884018090e-01,    1.352906667448606e-04,   -1.393761234581605e-03,
        1.900000000000000e+02,    8.876816418559648e-01,    7.613050101680658e-01,    1.278211315179112e-04,   -1.278243129349725e-03,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
            });

    check_is_close(pvto.getData(), expect);
}

BOOST_AUTO_TEST_SUITE_END ()

// ---------------------------------------------------------------------
// Derivatives of tabulated, piecewise linear functions

BOOST_AUTO_TEST_SUITE (Calculated_Slopes)

BOOST_AUTO_TEST_CASE (No_Derivatives)
{
    auto descr = ::Opm::DifferentiateOutputTable::Descriptor{};

    descr.tableID    = 0;
    descr.primID     = 0;
    descr.numActRows = 1;

    auto linTable = ::Opm::LinearisedOutputTable {
        1, 1, 3, 3  // Single table, one prim. key, 3 declared rows, 3 cols.
    };

    {
        auto x = 0.0;
        auto y = x;

        *linTable.column(descr.tableID, descr.primID, 0) = x;
        *linTable.column(descr.tableID, descr.primID, 1) = y;
    }

    // Argument dependent symbol lookup.
    calcSlopes(1, descr, linTable); // One dependent column.

    // Too few active rows (< 2).  Leave derivatives alone
    const auto expect = makeTable(3, {
            0,          0,          1.e+20,
            1.e+20,     1.e+20,     1.e+20,
            1.e+20,     1.e+20,     1.e+20,
        });

    check_is_close(linTable.getData(), expect);
}

BOOST_AUTO_TEST_CASE (Constant_Function)
{
    auto descr = ::Opm::DifferentiateOutputTable::Descriptor{};

    descr.tableID    = 0;
    descr.primID     = 0;
    descr.numActRows = 11;

    auto linTable = ::Opm::LinearisedOutputTable {
        1, 1, 15, 3  // Single table, one prim. key, 15 declared rows, 3 cols.
    };

    {
        auto x = std::vector<double> {
            0.0, 0.1, 0.2, 0.3, 0.4,
            0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };

        auto y = std::vector<double>(x.size(), 1.25);

        std::copy(x.begin(), x.end(),
                  linTable.column(descr.tableID, descr.primID, 0));

        std::copy(y.begin(), y.end(),
                  linTable.column(descr.tableID, descr.primID, 1));
    }

    // Argument dependent symbol lookup.
    calcSlopes(1, descr, linTable); // One dependent column.

    // Compute slopes for all intervals, store in left end point.
    // Non-active rows left at defaulted values.
    const auto expect = makeTable(3, {
            0,         1.25e+00,   1.0e+20,
            1.0e-01,   1.25e+00,         0,
            2.0e-01,   1.25e+00,         0,
            3.0e-01,   1.25e+00,         0,
            4.0e-01,   1.25e+00,         0,
            5.0e-01,   1.25e+00,         0,
            6.0e-01,   1.25e+00,         0,
            7.0e-01,   1.25e+00,         0,
            8.0e-01,   1.25e+00,         0,
            9.0e-01,   1.25e+00,         0,
            1.0e+00,   1.25e+00,         0,
            1.0e+20,   1.00e+20,   1.0e+20,
            1.0e+20,   1.00e+20,   1.0e+20,
            1.0e+20,   1.00e+20,   1.0e+20,
            1.0e+20,   1.00e+20,   1.0e+20,
                });

    check_is_close(linTable.getData(), expect);
}

BOOST_AUTO_TEST_CASE (Linear_Function)
{
    auto descr = ::Opm::DifferentiateOutputTable::Descriptor{};

    descr.tableID    = 0;
    descr.primID     = 0;
    descr.numActRows = 11;

    auto linTable = ::Opm::LinearisedOutputTable {
        1, 1, 11, 3  // Single table, one prim. key, 11 declared rows, 3 cols.
    };

    {
        auto x = std::vector<double> {
            0.0, 0.1, 0.2, 0.3, 0.4,
            0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };

        auto y = x;

        std::copy(x.begin(), x.end(),
                  linTable.column(descr.tableID, descr.primID, 0));

        std::copy(y.begin(), y.end(),
                  linTable.column(descr.tableID, descr.primID, 1));
    }

    // Argument dependent symbol lookup.
    calcSlopes(1, descr, linTable); // One dependent column.

    // Compute slopes for all intervals, store in left end point.
    const auto expect = makeTable(3, {
            0,         0,         1.0e+20,
            1.0e-01,   1.0e-01,   1.0,
            2.0e-01,   2.0e-01,   1.0,
            3.0e-01,   3.0e-01,   1.0,
            4.0e-01,   4.0e-01,   1.0,
            5.0e-01,   5.0e-01,   1.0,
            6.0e-01,   6.0e-01,   1.0,
            7.0e-01,   7.0e-01,   1.0,
            8.0e-01,   8.0e-01,   1.0,
            9.0e-01,   9.0e-01,   1.0,
            1.0e+00,   1.0e+00,   1.0,
        });

    check_is_close(linTable.getData(), expect);
}

BOOST_AUTO_TEST_CASE (Nonlinear_Functions)
{
    auto descr = ::Opm::DifferentiateOutputTable::Descriptor{};

    descr.tableID    = 0;
    descr.primID     = 0;
    descr.numActRows = 11;

    auto linTable = ::Opm::LinearisedOutputTable {
        1, 1, 15, 7  // Single table, one prim. key, 15 declared rows, 7 cols.
    };

    {
        const auto x = std::vector<double> {
            0.0, 0.1, 0.2, 0.3, 0.4,
            0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };

        // sin(2*pi * x)
        const auto s = std::vector<double> {
             0,
             5.877852522924731e-01,
             9.510565162951535e-01,
             9.510565162951536e-01,
             5.877852522924732e-01,
             1.224646799147353e-16,
            -5.877852522924730e-01,
            -9.510565162951535e-01,
            -9.510565162951536e-01,
            -5.877852522924734e-01,
            -2.449293598294706e-16,
        };

        // cos(4*pi * x)^2
        const auto c = std::vector<double> {
            1.000000000000000e+00,
            9.549150281252630e-02,
            6.545084971874736e-01,
            6.545084971874737e-01,
            9.549150281252616e-02,
            1.000000000000000e+00,
            9.549150281252648e-02,
            6.545084971874734e-01,
            6.545084971874742e-01,
            9.549150281252602e-02,
            1.000000000000000e+00,
        };

        // exp(-x) / (1 + x)
        const auto f = std::vector<double> {
            1.000000000000000e+00,
            8.225794709417813e-01,
            6.822756275649848e-01,
            5.698601697551676e-01,
            4.788000328825995e-01,
            4.043537731417556e-01,
            3.430072725587666e-01,
            2.921090022302409e-01,
            2.496272022873453e-01,
            2.139840314424206e-01,
            1.839397205857212e-01,
        };

        std::copy(x.begin(), x.end(),
                  linTable.column(descr.tableID, descr.primID, 0));

        std::copy(s.begin(), s.end(),
                  linTable.column(descr.tableID, descr.primID, 1));

        std::copy(c.begin(), c.end(),
                  linTable.column(descr.tableID, descr.primID, 2));

        std::copy(f.begin(), f.end(),
                  linTable.column(descr.tableID, descr.primID, 3));
    }

    // Argument dependent symbol lookup.
    calcSlopes(3, descr, linTable); // Three dependent columns.

    // Compute slopes for all intervals, store in left end point.
    const auto expect = makeTable(7, {
            0,          0,                        1.000000000000000e+00,    1.000000000000000e+00,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
            1.0e-01,    5.877852522924731e-01,    9.549150281252630e-02,    8.225794709417813e-01,    5.877852522924731e+00,   -9.045084971874736e+00,   -1.774205290582187e+00,
            2.0e-01,    9.510565162951535e-01,    6.545084971874736e-01,    6.822756275649848e-01,    3.632712640026804e+00,    5.590169943749473e+00,   -1.403038433767965e+00,
            3.0e-01,    9.510565162951536e-01,    6.545084971874737e-01,    5.698601697551676e-01,    1.110223024625157e-15,    1.110223024625157e-15,   -1.124154578098172e+00,
            4.0e-01,    5.877852522924732e-01,    9.549150281252616e-02,    4.788000328825995e-01,   -3.632712640026803e+00,   -5.590169943749474e+00,   -9.106013687256805e-01,
            5.0e-01,    1.224646799147353e-16,    1.000000000000000e+00,    4.043537731417556e-01,   -5.877852522924733e+00,    9.045084971874740e+00,   -7.444625974084391e-01,
            6.0e-01,   -5.877852522924730e-01,    9.549150281252648e-02,    3.430072725587666e-01,   -5.877852522924733e+00,   -9.045084971874736e+00,   -6.134650058298908e-01,
            7.0e-01,   -9.510565162951535e-01,    6.545084971874734e-01,    2.921090022302409e-01,   -3.632712640026806e+00,    5.590169943749470e+00,   -5.089827032852569e-01,
            8.0e-01,   -9.510565162951536e-01,    6.545084971874742e-01,    2.496272022873453e-01,   -1.110223024625156e-15,    7.771561172376089e-15,   -4.248179994289554e-01,
            9.0e-01,   -5.877852522924734e-01,    9.549150281252602e-02,    2.139840314424206e-01,    3.632712640026804e+00,   -5.590169943749483e+00,   -3.564317084492472e-01,
            1.0e+00,   -2.449293598294706e-16,    1.000000000000000e+00,    1.839397205857212e-01,    5.877852522924733e+00,    9.045084971874742e+00,   -3.004431085669943e-01,
            1.0e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
            1.0e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
            1.0e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
            1.0e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,    1.000000000000000e+20,
        });

    check_is_close(linTable.getData(), expect);
}

BOOST_AUTO_TEST_SUITE_END ()
