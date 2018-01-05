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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_ECL1DINTERPOLATION

#include <boost/test/unit_test.hpp>

#include <opm/utility/ECLPiecewiseLinearInterpolant.hpp>
#include <opm/utility/ECLTableInterpolation1D.hpp>

#include <exception>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace PP = ::Opm::Interp1D::PiecewisePolynomial;

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

    std::vector<double> spe1_swof()
    {
        return makeTable(4, {
                0.12,    0,                       1,       0,     //  0
                0.18,    4.64876033057851E-008,   1,       0,     //  1
                0.24,    0.000000186,             0.997,   0,     //  2
                0.3 ,    4.18388429752066E-007,   0.98,    0,     //  3
                0.36,    7.43801652892562E-007,   0.7,     0,     //  4
                0.42,    1.16219008264463E-006,   0.35,    0,     //  5
                0.48,    1.67355371900826E-006,   0.2,     0,     //  6
                0.54,    2.27789256198347E-006,   0.09,    0,     //  7
                0.6 ,    2.97520661157025E-006,   0.021,   0,     //  8
                0.66,    3.7654958677686E-006,    0.01,    0,     //  9
                0.72,    4.64876033057851E-006,   0.001,   0,     // 10
                0.78,    0.000005625,             0.0001,  0,     // 11
                0.84,    6.69421487603306E-006,   0,       0,     // 12
                0.91,    8.05914256198347E-006,   0,       0,     // 13
                1   ,    0.00001,                 0,       0, }); // 14
    }

    std::vector<double> spe1_pvdg_incl_der()
    {
        return makeTable(5, {
                // Pg                     1/Bg                      1/(Bg*vg)                 d[1/Bg]/dp                d[1/(Bg*vg)]/dp
                1.470000000000000e+01,    6.000024000096000e-03,    7.500030000120000e-01,    0,                        0,                     // 0
                2.647000000000000e+02,    8.269246671628200e-02,    8.613798616279400e+00,    3.067697708647400e-04,    3.145518246507000e-02, // 1
                5.147000000000000e+02,    1.593879502709600e-01,    1.423106698847900e+01,    3.067819342187100e-04,    2.246907348879700e-02, // 2
                1.014700000000000e+03,    3.127932436659400e-01,    2.234237454756700e+01,    3.068105867899500e-04,    1.622261511817700e-02, // 3
                2.014700000000000e+03,    6.195786864931800e-01,    3.278194108429500e+01,    3.067854428272500e-04,    1.043956653672900e-02, // 4
                2.514700000000000e+03,    7.727975270479100e-01,    3.715372726191900e+01,    3.064376811094600e-04,    8.743572355246899e-03, // 5
                3.014700000000000e+03,    9.259259259259300e-01,    4.061078622482100e+01,    3.062567977560200e-04,    6.914117925804800e-03, // 6
                4.014700000000000e+03,    1.233045622688000e+00,    4.600916502567300e+01,    3.071196967621100e-04,    5.398378800851800e-03, // 7
                5.014700000000000e+03,    1.540832049306600e+00,    4.986511486429200e+01,    3.077864266185900e-04,    3.855949838619000e-03, // 8
                9.014700000000001e+03,    2.590673575129500e+00,    5.512071436445800e+01,    2.624603814557300e-04,    1.313899875041500e-03, // 9
                    });
    }

    std::vector<double> spe1_pvdg_without_der()
    {
        return makeTable(3, {
                // Pg                     1/Bg                      1/(Bg*vg)
                1.470000000000000e+01,    6.000024000096000e-03,    7.500030000120000e-01, // 0
                2.647000000000000e+02,    8.269246671628200e-02,    8.613798616279400e+00, // 1
                5.147000000000000e+02,    1.593879502709600e-01,    1.423106698847900e+01, // 2
                1.014700000000000e+03,    3.127932436659400e-01,    2.234237454756700e+01, // 3
                2.014700000000000e+03,    6.195786864931800e-01,    3.278194108429500e+01, // 4
                2.514700000000000e+03,    7.727975270479100e-01,    3.715372726191900e+01, // 5
                3.014700000000000e+03,    9.259259259259300e-01,    4.061078622482100e+01, // 6
                4.014700000000000e+03,    1.233045622688000e+00,    4.600916502567300e+01, // 7
                5.014700000000000e+03,    1.540832049306600e+00,    4.986511486429200e+01, // 8
                9.014700000000001e+03,    2.590673575129500e+00,    5.512071436445800e+01, // 9
                    });
    }

    std::vector<double> pvtg_subtable_zero_der()
    {
        return makeTable(5, {
                // Rv     1/Bg                   1/(Bg*vg               d[1/Bg]/dRv  d[1/(Bg*vg)]/dRv
                4.97e-06, 4.006731308598445e+01, 2.780521380012800e+03, 0,           0,                     // 0
                2.48e-06, 4.006731308598445e+01, 2.782452297637809e+03, 0,          -7.754689257064208e+05, // 1
                0       , 4.006731308598445e+01, 2.782452297637809e+03, 0,           0,                     // 2
                    });
    }

    std::vector<double> pvtg_subtable_incl_der()
    {
        return makeTable(5, {
                // Rv        1/Bg                      1/(Bg*vg)                 d[1/Bg]/dRv               d[1/(Bg*vg)]/dRv
                5.21e-06,    5.669255626736210e+01,    3.802317657100074e+03,    8.312621590688825e-01,    5.108981385436368e+01, // 0
                2.61e-06,    5.668612890425712e+01,    3.804438181493767e+03,    2.472062732683009e+03,   -8.155863052665014e+05, // 1
                0       ,    5.667970299835629e+01,    3.804006912641362e+03,    2.462032912196776e+03,    1.652371082011811e+05, // 2
                    });
    }

    std::function<double(const double)>
    createDummyTransform()
    {
        return { [](const double x) { return x; } };
    }

    std::vector<std::function<double(const double)>>
    createDummyTransform(const std::size_t ncol)
    {
        return std::vector<std::function<double(const double)>>
            (ncol, createDummyTransform());
    }
} // Namespace Anonymous

// =====================================================================
// Point Binning
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (PointLocationBinning)

BOOST_AUTO_TEST_CASE (NonExistentRange)
{
    const auto xi = std::vector<double>{};

    BOOST_CHECK_THROW(PP::LocalInterpPoint::identify(xi, 0.0),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (SingleAbscissa)
{
    const auto abscissas = std::vector<double>{ 0.0 };

    // Left of range's left (only) end-point.
    {
        const auto x = -1.0;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        // Left of range's interval is always 0 (multiplied by .size() to
        // enforce correct integer type in '==' check within Boost.Test).
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - xmin.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.front(), 1.0e-10);
    }

    // Right of range's right (only) end-point.
    {
        const auto x = 1.0;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        // Right of range's interval is always one less than the number of
        // abscissas.
        BOOST_CHECK_EQUAL(pt.interval, abscissas.size() - 1);

        // t = x - xmax.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.back(), 1.0e-10);
    }

    // Single abscissa point.
    {
        const auto x = abscissas[0];

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        // We should identify this as being in range.
        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

        // Interval should correspond to left (only) end-point.
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - abscissas[interval] (== 0.0).
        BOOST_CHECK_CLOSE(pt.t, 0.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE (SingleInterval)
{
    const auto abscissas = std::vector<double>{ 0.0, 1.0 };

    // Left of range's left end-point.
    {
        const auto x = -1.0;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        // Left of range's interval is always 0 (multiplied by .size() to
        // enforce correct integer type in '==' check within Boost.Test).
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - xmin.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.front(), 1.0e-10);
    }

    // Right of range's right end-point.
    {
        const auto x = 2.0;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        // Right of range's interval is always one less than the number of
        // abscissas.
        BOOST_CHECK_EQUAL(pt.interval, abscissas.size() - 1);

        // t = x - xmax.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.back(), 1.0e-10);
    }

    // Single abscissa point (left end point).
    {
        const auto x = abscissas[0];

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        // We should identify this as being in range.
        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

        // Interval should correspond to left (only) end-point.
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - abscissas[interval] (== 0.0).
        BOOST_CHECK_CLOSE(pt.t, 0.0, 1.0e-10);
    }

    // Single abscissa point (right end point).
    {
        const auto x = abscissas[1];

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        // We should identify this as being in range.
        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

        // Interval should correspond to left end-point.
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - abscissas[interval] (== 1.0).
        BOOST_CHECK_CLOSE(pt.t, 1.0, 1.0e-10);
    }

    // Common case: Points within range
    {
        const auto x = std::vector<double> {
            0.1, 0.2, 0.4, 0.5, 0.75, 0.8,
            1.0 - std::numeric_limits<double>::epsilon(),
        };

        for (const auto& xi : x) {
            const auto pt =
                PP::LocalInterpPoint::identify(abscissas, xi);

            // We should identify this as being in range.
            BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

            // Interval should correspond to left end-point.
            BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

            // t = x - abscissas[interval] (== xi).
            BOOST_CHECK_CLOSE(pt.t, xi, 1.0e-10);
        }
    }
}

BOOST_AUTO_TEST_CASE (MultipleIntervals)
{
    const auto abscissas = []() -> std::vector<double>
    {
        auto       pvdg = spe1_pvdg_incl_der();
        const auto nrow = pvdg.size() / 5;

        return {
            std::make_move_iterator(std::begin(pvdg)),
            std::make_move_iterator(std::begin(pvdg) + nrow)
        };
    }();

    // Left of range's left end-point.
    {
        const auto x = 10.0;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        // Left of range's interval is always 0 (multiplied by .size() to
        // enforce correct integer type in '==' check within Boost.Test).
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - xmin (== -4.7).
        BOOST_CHECK_CLOSE(pt.t, -4.7, 1.0e-10);
    }

    // Right of range's right end-point.
    {
        const auto x = 10.0147e3;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        // Right of range's interval is always one less than the number of
        // abscissas.
        BOOST_CHECK_EQUAL(pt.interval, abscissas.size() - 1);

        // t = x - xmax (== 1000).
        BOOST_CHECK_CLOSE(pt.t, 1.0e3, 1.0e-10);
    }

    // Check that every abscissa is classified as being in range and that it
    // is associated to the point to the left of itself, except for the
    // first.  The first abscissa must be associated with itself.
    //
    // Note: This behaviour is an "emergent" property of the implementation
    // of LocalInterpPoint::identify().
    {
        const auto n = abscissas.size();
        auto i = 0*n;

        for (const auto& xi : abscissas) {
            const auto pt =
                PP::LocalInterpPoint::identify(abscissas, xi);

            const auto interval_expect = (i == 0) ? i : (i - 1);

            BOOST_CHECK_EQUAL(pt.interval, interval_expect);

            BOOST_CHECK_CLOSE(pt.t, xi - abscissas[interval_expect], 1.0e-10);

            i += 1;
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Piecewise Linear 1D Interpolation
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (PiecewiseLinear)

BOOST_AUTO_TEST_CASE (SWOF)
{
    const auto swof = spe1_swof();

    const auto nRows = swof.size() / 4;
    auto xBegin = std::begin(swof);
    auto xEnd   = xBegin + nRows;

    auto colIt = std::vector<decltype(xBegin)>{ xEnd };
    for (auto j = 1; j < 3; ++j) {
        colIt.push_back(colIt.back() + nRows);
    }

    using Extrap = PP::ExtrapolationPolicy::Constant;
    auto interp  = PP::Linear<Extrap>
        { Extrap{}, xBegin, xEnd, colIt,
          createDummyTransform(),
          createDummyTransform(colIt.size()) };

    // Extrapolation to the left of the interval.
    {
        const auto pt = interp.classifyPoint(0.0);

        const auto krw  = interp.evaluate(0, pt);
        const auto krow = interp.evaluate(1, pt);
        const auto pcow = interp.evaluate(2, pt);

        BOOST_CHECK_CLOSE(krw , 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(krow, 1.0, 1.0e-10);
        BOOST_CHECK_CLOSE(pcow, 0.0, 1.0e-10);
    }

    // Extrapolation to the right of the interval.
    {
        const auto pt = interp.classifyPoint(1.1);

        const auto krw  = interp.evaluate(0, pt);
        const auto krow = interp.evaluate(1, pt);
        const auto pcow = interp.evaluate(2, pt);

        BOOST_CHECK_CLOSE(krw , 1.0e-5, 1.0e-10);
        BOOST_CHECK_CLOSE(krow, 0.0   , 1.0e-10);
        BOOST_CHECK_CLOSE(pcow, 0.0   , 1.0e-10);
    }

    // First sub-interval (Sw \in [0.12, 0.18)).
    {
        const auto sw = std::vector<double> {
            0.12, 0.13, 0.14, 0.15, 0.16, 0.17 };

        //             (swof(1,1) - swof(0,1))
        // swof(0,0) + ----------------------- * (sw - swof(0,0))
        //             (swof(1,0) - swof(0,0))
        const auto krw_expect = std::vector<double> {
            0,
            7.747933884297524e-09,
            1.549586776859505e-08,
            2.324380165289255e-08,
            3.099173553719008e-08,
            3.873966942148760e-08,
        };

        const auto krow_expect = std::vector<double> {
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        };

        const auto pcow_expect = std::vector<double> {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        };

        auto krw  = std::vector<double>{}; krw .reserve(sw.size());
        auto krow = std::vector<double>{}; krow.reserve(sw.size());
        auto pcow = std::vector<double>{}; pcow.reserve(sw.size());

        for (const auto& swi : sw) {
            const auto pt = interp.classifyPoint(swi);

            krw .push_back(interp.evaluate(0, pt));
            krow.push_back(interp.evaluate(1, pt));
            pcow.push_back(interp.evaluate(2, pt));
        }

        check_is_close(krw , krw_expect );
        check_is_close(krow, krow_expect);
        check_is_close(pcow, pcow_expect);
    }

    // Second sub-interval (Sw \in [0.18, 0.24)).
    {
        const auto sw = std::vector<double> {
            0.18, 0.19, 0.20, 0.21, 0.22, 0.23, 0.24,
        };

        //             (swof(2,1) - swof(1,1))
        // swof(1,0) + ----------------------- * (sw - swof(1,0))
        //             (swof(2,0) - swof(1,0))
        const auto krw_expect = std::vector<double> {
            4.648760330578510e-08,
            6.973966942148761e-08,
            9.299173553719010e-08,
            1.162438016528925e-07,
            1.394958677685950e-07,
            1.627479338842975e-07,
            1.860000000000000e-07,
        };

        const auto krow_expect = std::vector<double> {
            1.000000000000000e+00,
            9.994999999999999e-01,
            9.990000000000000e-01,
            9.984999999999999e-01,
            9.980000000000000e-01,
            9.974999999999999e-01,
            9.970000000000000e-01,
        };

        const auto pcow_expect = std::vector<double> {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        };

        auto krw  = std::vector<double>{}; krw .reserve(sw.size());
        auto krow = std::vector<double>{}; krow.reserve(sw.size());
        auto pcow = std::vector<double>{}; pcow.reserve(sw.size());

        for (const auto& swi : sw) {
            const auto pt = interp.classifyPoint(swi);

            krw .push_back(interp.evaluate(0, pt));
            krow.push_back(interp.evaluate(1, pt));
            pcow.push_back(interp.evaluate(2, pt));
        }

        check_is_close(krw , krw_expect );
        check_is_close(krow, krow_expect);
        check_is_close(pcow, pcow_expect);
    }

    // Fifth sub-interval (Sw \in [0.36, 0.42)).
    {
        const auto sw = std::vector<double> {
            0.36, 0.37, 0.38, 0.39, 0.40, 0.41, 0.42, 0.39, 0.365,
        };

        //             (swof(5,1) - swof(4,1))
        // swof(4,0) + ----------------------- * (sw - swof(4,0))
        //             (swof(5,0) - swof(4,0))
        const auto krw_expect = std::vector<double> {
            7.438016528925620e-07,
            8.135330578512400e-07,
            8.832644628099181e-07,
            9.529958677685962e-07,
            1.022727272727274e-06,
            1.092458677685952e-06,
            1.162190082644630e-06,
            9.529958677685962e-07,
            7.786673553719010e-07,
        };

        const auto krow_expect = std::vector<double> {
            7.000000000000000e-01,
            6.416666666666666e-01,
            5.833333333333331e-01,
            5.249999999999998e-01,
            4.666666666666665e-01,
            4.083333333333334e-01,
            3.500000000000000e-01,
            5.249999999999998e-01,
            6.708333333333333e-01,
        };

        const auto pcow_expect = std::vector<double>(sw.size(), 0.0);

        auto krw  = std::vector<double>{}; krw .reserve(sw.size());
        auto krow = std::vector<double>{}; krow.reserve(sw.size());
        auto pcow = std::vector<double>{}; pcow.reserve(sw.size());

        for (const auto& swi : sw) {
            const auto pt = interp.classifyPoint(swi);

            krw .push_back(interp.evaluate(0, pt));
            krow.push_back(interp.evaluate(1, pt));
            pcow.push_back(interp.evaluate(2, pt));
        }

        check_is_close(krw , krw_expect );
        check_is_close(krow, krow_expect);
        check_is_close(pcow, pcow_expect);
    }

    // Eleventh sub-interval (Sw \in [0.72, 0.78)).
    {
        const auto sw = std::vector<double> {
            0.72, 0.73, 0.74, 0.75, 0.76, 0.77, 0.78, 0.725,
        };

        //              (swof(11,1) - swof(10,1))
        // swof(10,0) + ------------------------- * (sw - swof(10,0))
        //              (swof(11,0) - swof(10,0))
        const auto krw_expect = std::vector<double> {
            4.648760330578510e-06,
            4.811466942148759e-06,
            4.974173553719007e-06,
            5.136880165289256e-06,
            5.299586776859503e-06,
            5.462293388429752e-06,
            5.625000000000000e-06,
            4.730113636363634e-06,
        };

        const auto krow_expect = std::vector<double> {
            1.000000000000000e-03,
            8.500000000000001e-04,
            7.000000000000001e-04,
            5.500000000000000e-04,
            4.000000000000001e-04,
            2.500000000000001e-04,
            1.000000000000000e-04,
            9.250000000000000e-04,
        };

        const auto pcow_expect = std::vector<double>(sw.size(), 0.0);

        auto krw  = std::vector<double>{}; krw .reserve(sw.size());
        auto krow = std::vector<double>{}; krow.reserve(sw.size());
        auto pcow = std::vector<double>{}; pcow.reserve(sw.size());

        for (const auto& swi : sw) {
            const auto pt = interp.classifyPoint(swi);

            krw .push_back(interp.evaluate(0, pt));
            krow.push_back(interp.evaluate(1, pt));
            pcow.push_back(interp.evaluate(2, pt));
        }

        check_is_close(krw , krw_expect );
        check_is_close(krow, krow_expect);
        check_is_close(pcow, pcow_expect);
    }
}

BOOST_AUTO_TEST_CASE (PVDG_Estimated_Derivatives)
{
    const auto pvdg = spe1_pvdg_without_der();

    const auto nRows = pvdg.size() / 3;
    auto xBegin = std::begin(pvdg);
    auto xEnd   = xBegin + nRows;

    auto colIt = std::vector<decltype(xBegin)>{ xEnd };
    for (auto j = 1; j < 2; ++j) {
        colIt.push_back(colIt.back() + nRows);
    }

    using Extrap = PP::ExtrapolationPolicy::Linearly;
    auto interp  = PP::Linear<Extrap>
        { Extrap{}, xBegin, xEnd, colIt,
          createDummyTransform(),
          createDummyTransform(colIt.size()) };

    // Extrapolation to the left.  d/dp estimated from first interval's size
    // and function values.
    {
        const auto pt = interp.classifyPoint(10.0);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        // PVDG(1,2) + ((PVDG(2,2) - PVDG(1,2))/(PVDG(2,1) - PVDG(1,1))*pt.t
        BOOST_CHECK_CLOSE(bg, 4.558206077031703e-03, 1.0e-10);

        // PVDG(1,3) + ((PVDG(2,3) - PVDG(1,3))/(PVDG(2,1) - PVDG(1,1))*pt.t
        BOOST_CHECK_CLOSE(bg_by_vg, 6.021636424261729e-01, 1.0e-10);
    }

    // Extrapolation to the right.  d/dp estimated from last interval's size
    // and function values.
    {
        const auto pt = interp.classifyPoint(10.0147e+3);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        // PVDG(1,2) + ((PVDG(2,2) - PVDG(1,2))/(PVDG(2,1) - PVDG(1,1))*pt.t
        BOOST_CHECK_CLOSE(bg, 2.853133956585225e+00, 1.0e-10);

        // PVDG(1,3) + ((PVDG(2,3) - PVDG(1,3))/(PVDG(2,1) - PVDG(1,1))*pt.t
        BOOST_CHECK_CLOSE(bg_by_vg, 5.643461423949950e+01, 1.0e-10);
    }

    // First interval.  Pg \in [14.7, 264.7).
    {
        const auto Pg = std::vector<double> {
            14.7, 64.7, 114.7, 164.7, 214.7, 264.7 };

        const auto bg_expect = std::vector<double> {
            6.000024000096000e-03,
            2.133851254333320e-02,
            3.667700108657040e-02,
            5.201548962980759e-02,
            6.735397817304480e-02,
            8.269246671628200e-02,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            7.500030000120000e-01,
            2.322762123265480e+00,
            3.895521246518960e+00,
            5.468280369772439e+00,
            7.041039493025919e+00,
            8.613798616279400e+00,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Pg.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Pg.size());

        for (const auto& pgi : Pg) {
            const auto pt = interp.classifyPoint(pgi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }

    // Second interval.  Pg \in [264.7, 514.7).
    {
        const auto Pg = std::vector<double> {
            264.7, 314.7, 364.7, 414.7, 464.7, 514.7 };

        const auto bg_expect = std::vector<double> {
            8.269246671628200e-02,
            9.803156342721760e-02,
            1.133706601381532e-01,
            1.287097568490888e-01,
            1.440488535600244e-01,
            1.593879502709600e-01,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            8.613798616279400e+00,
            9.737252290719320e+00,
            1.086070596515924e+01,
            1.198415963959916e+01,
            1.310761331403908e+01,
            1.423106698847900e+01,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Pg.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Pg.size());

        for (const auto& pgi : Pg) {
            const auto pt = interp.classifyPoint(pgi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }

    // Last interval.  Pg \in [5014.7, 9014.7).
    {
        const auto Pg = std::vector<double> {
            5014.7, 5514.7, 6014.7, 6514.7, 7014.7, 7514.7, 8014.7, 8514.7, 9014.7 };

        const auto bg_expect = std::vector<double> {
            1.540832049306600e+00,
            1.672062240034462e+00,
            1.803292430762325e+00,
            1.934522621490187e+00,
            2.065752812218050e+00,
            2.196983002945912e+00,
            2.328213193673775e+00,
            2.459443384401637e+00,
            2.590673575129500e+00,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            4.986511486429200e+01,
            5.052206480181275e+01,
            5.117901473933350e+01,
            5.183596467685425e+01,
            5.249291461437500e+01,
            5.314986455189575e+01,
            5.380681448941650e+01,
            5.446376442693725e+01,
            5.512071436445800e+01,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Pg.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Pg.size());

        for (const auto& pgi : Pg) {
            const auto pt = interp.classifyPoint(pgi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }
}

BOOST_AUTO_TEST_CASE (PVDG_Tabulated_Derivatives)
{
    const auto pvdg = spe1_pvdg_incl_der();

    const auto nRows = pvdg.size() / 5;
    auto xBegin = std::begin(pvdg);
    auto xEnd   = xBegin + nRows;

    auto colIt = std::vector<decltype(xBegin)>{ xEnd };
    for (auto j = 1; j < 4; ++j) {
        colIt.push_back(colIt.back() + nRows);
    }

    const auto nResCol = std::size_t{2}; // 1/B, 1/(B*v)
    using Extrap = PP::ExtrapolationPolicy::LinearlyWithDerivatives;
    auto interp  = PP::Linear<Extrap>
        { Extrap{nResCol}, xBegin, xEnd, colIt,
          createDummyTransform(),
          createDummyTransform(colIt.size()) };

    // Extrapolation to the left (d/dp = 0, from table).
    {
        const auto pt = interp.classifyPoint(10.0);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        BOOST_CHECK_CLOSE(bg, 6.000024000096000e-03, 1.0e-10);
        BOOST_CHECK_CLOSE(bg_by_vg, 7.500030000120000e-01, 1.0e-10);
    }

    // Extrapolation to the right (d/dp > 0, from table).
    {
        const auto pt = interp.classifyPoint(10.0147e+3);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        BOOST_CHECK_CLOSE(bg, 2.853133956585230e+00, 1.0e-10);
        BOOST_CHECK_CLOSE(bg_by_vg, 5.643461423949950e+01, 1.0e-10);
    }

    // First interval.  Pg \in [14.7, 264.7).
    {
        const auto Pg = std::vector<double> {
            14.7, 64.7, 114.7, 164.7, 214.7, 264.7 };

        const auto bg_expect = std::vector<double> {
            6.000024000096000e-03,
            2.133851254333320e-02,
            3.667700108657040e-02,
            5.201548962980759e-02,
            6.735397817304480e-02,
            8.269246671628200e-02,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            7.500030000120000e-01,
            2.322762123265480e+00,
            3.895521246518960e+00,
            5.468280369772439e+00,
            7.041039493025919e+00,
            8.613798616279400e+00,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Pg.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Pg.size());

        for (const auto& pgi : Pg) {
            const auto pt = interp.classifyPoint(pgi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }

    // Second interval.  Pg \in [264.7, 514.7).
    {

        const auto Pg = std::vector<double> {
            264.7, 314.7, 364.7, 414.7, 464.7, 514.7 };

        const auto bg_expect = std::vector<double> {
            8.269246671628200e-02,
            9.803156342721760e-02,
            1.133706601381532e-01,
            1.287097568490888e-01,
            1.440488535600244e-01,
            1.593879502709600e-01,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            8.613798616279400e+00,
            9.737252290719320e+00,
            1.086070596515924e+01,
            1.198415963959916e+01,
            1.310761331403908e+01,
            1.423106698847900e+01,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Pg.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Pg.size());

        for (const auto& pgi : Pg) {
            const auto pt = interp.classifyPoint(pgi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }

    // Last interval.  Pg \in [5014.7, 9014.7).
    {

        const auto Pg = std::vector<double> {
            5014.7, 5514.7, 6014.7, 6514.7, 7014.7, 7514.7, 8014.7, 8514.7, 9014.7 };

        const auto bg_expect = std::vector<double> {
            1.540832049306600e+00,
            1.672062240034462e+00,
            1.803292430762325e+00,
            1.934522621490187e+00,
            2.065752812218050e+00,
            2.196983002945912e+00,
            2.328213193673775e+00,
            2.459443384401637e+00,
            2.590673575129500e+00,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            4.986511486429200e+01,
            5.052206480181275e+01,
            5.117901473933350e+01,
            5.183596467685425e+01,
            5.249291461437500e+01,
            5.314986455189575e+01,
            5.380681448941650e+01,
            5.446376442693725e+01,
            5.512071436445800e+01,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Pg.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Pg.size());

        for (const auto& pgi : Pg) {
            const auto pt = interp.classifyPoint(pgi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Descendingly Sorted Input Ranges (B and mu a.f.o. decreasing Rv)
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (PointLocationBinning_Descending)

BOOST_AUTO_TEST_CASE (NonExistentRange)
{
    const auto xi = std::vector<double>{};

    const auto is_ascending = std::false_type{};

    BOOST_CHECK_THROW(PP::LocalInterpPoint::identify(xi, 0.0, is_ascending),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (SingleAbscissa)
{
    const auto abscissas = std::vector<double>{ 0.0 };

    auto identify = [&abscissas](const double x)
    {
        return PP::LocalInterpPoint::
            identify(abscissas, x, std::false_type{});
    };

    // Left of range's left (only) end-point.
    {
        const auto x  = 1.0;
        const auto pt = identify(x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        // Left of range's interval is always 0 (multiplied by .size() to
        // enforce correct integer type in '==' check within Boost.Test).
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - xmin.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.front(), 1.0e-10);
    }

    // Right of range's right (only) end-point.
    {
        const auto x  = -1.0;
        const auto pt = identify(x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        // Right of range's interval is always one less than the number of
        // abscissas.
        BOOST_CHECK_EQUAL(pt.interval, abscissas.size() - 1);

        // t = x - xmax.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.back(), 1.0e-10);
    }

    // Single abscissa point.
    {
        const auto x  = abscissas[0];
        const auto pt = identify(x);

        // We should identify this as being in range.
        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

        // Interval should correspond to left (only) end-point.
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - abscissas[interval] (== 0.0).
        BOOST_CHECK_CLOSE(pt.t, 0.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE (SingleInterval)
{
    const auto abscissas = std::vector<double>{ 1.0, 0.0 };

    auto identify = [&abscissas](const double x)
    {
        return PP::LocalInterpPoint::
            identify(abscissas, x, std::false_type{});
    };

    // Left of range's left end-point.
    {
        const auto x  = 2.0;
        const auto pt = identify(x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        // Left of range's interval is always 0 (multiplied by .size() to
        // enforce correct integer type in '==' check within Boost.Test).
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - xmin.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.front(), 1.0e-10);
    }

    // Right of range's right end-point.
    {
        const auto x  = -1.0;
        const auto pt = identify(x);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        // Right of range's interval is always one less than the number of
        // abscissas.
        BOOST_CHECK_EQUAL(pt.interval, abscissas.size() - 1);

        // t = x - xmax.
        BOOST_CHECK_CLOSE(pt.t, x - abscissas.back(), 1.0e-10);
    }

    // Single abscissa point (left end point).
    {
        const auto x  = abscissas[0];
        const auto pt = identify(x);

        // We should identify this as being in range.
        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

        // Interval should correspond to left (only) end-point.
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - abscissas[interval] (== 0.0).
        BOOST_CHECK_CLOSE(pt.t, 0.0, 1.0e-10);
    }

    // Single abscissa point (right end point).
    {
        const auto x  = abscissas[1];
        const auto pt = identify(x);

        // We should identify this as being in range.
        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

        // Interval should correspond to left end-point.
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - abscissas[interval] (== -1.0).
        BOOST_CHECK_CLOSE(pt.t, -1.0, 1.0e-10);
    }

    // Common case: Points within range
    {
        const auto x = std::vector<double> {
            0.1, 0.2, 0.4, 0.5, 0.75, 0.8,
            1.0 - std::numeric_limits<double>::epsilon(),
        };

        for (const auto& xi : x) {
            const auto pt = identify(xi);

            // We should identify this as being in range.
            BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::InRange);

            // Interval should correspond to left end-point.
            BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

            // t = x - abscissas[interval] (== xi - 1.0).
            BOOST_CHECK_CLOSE(pt.t, xi - 1.0, 1.0e-10);
        }
    }
}

BOOST_AUTO_TEST_CASE (MultipleIntervals)
{
    const auto abscissas = []() -> std::vector<double>
    {
        auto       pvtg = pvtg_subtable_zero_der();
        const auto nrow = pvtg.size() / 5;

        return {
            std::make_move_iterator(std::begin(pvtg)),
            std::make_move_iterator(std::begin(pvtg) + nrow)
        };
    }();

    const auto is_descending = std::false_type{};

    // Left of range's left end-point.
    {
        const auto x = 1.0;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x, is_descending);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        // Left of range's interval is always 0 (multiplied by .size() to
        // enforce correct integer type in '==' check within Boost.Test).
        BOOST_CHECK_EQUAL(pt.interval, 0*abscissas.size());

        // t = x - xmin (== 9.999950300000000e-01).
        BOOST_CHECK_CLOSE(pt.t, 9.999950300000000e-01, 1.0e-10);
    }

    // Right of range's right end-point.
    {
        const auto x = -0.5;

        const auto pt =
            PP::LocalInterpPoint::identify(abscissas, x, is_descending);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        // Right of range's interval is always one less than the number of
        // abscissas.
        BOOST_CHECK_EQUAL(pt.interval, abscissas.size() - 1);

        // t = x - xmax (== -0.5).
        BOOST_CHECK_CLOSE(pt.t, -0.5, 1.0e-10);
    }

    // Check that every abscissa is classified as being in range and that it
    // is associated to the point to the left of itself, except for the
    // first.  The first abscissa must be associated with itself.
    //
    // Note: This behaviour is an "emergent" property of the implementation
    // of LocalInterpPoint::identify().
    {
        const auto n = abscissas.size();
        auto i = 0*n;

        for (const auto& xi : abscissas) {
            const auto pt =
                PP::LocalInterpPoint::identify(abscissas, xi, is_descending);

            const auto interval_expect = (i == 0) ? i : (i - 1);

            BOOST_CHECK_EQUAL(pt.interval, interval_expect);

            BOOST_CHECK_CLOSE(pt.t, xi - abscissas[interval_expect], 1.0e-10);

            i += 1;
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Piecewise Linear 1D Interpolation in Descendingly Sorted Input Range
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (PicewiseLinar_Descending)

BOOST_AUTO_TEST_CASE (PVTGSingleSubTable_ConstantExtrap)
{
    const auto pvtg = pvtg_subtable_zero_der();

    const auto nRows = pvtg.size() / 5;
    auto xBegin = std::begin(pvtg);
    auto xEnd   = xBegin + nRows;

    auto colIt = std::vector<decltype(xBegin)>{ xEnd };
    for (auto j = 1; j < 4; ++j) {
        colIt.push_back(colIt.back() + nRows);
    }

    const auto IsAscendingRange = false;
    using Extrap = PP::ExtrapolationPolicy::Constant;
    auto interp  = PP::Linear<Extrap, IsAscendingRange>
        { Extrap{}, xBegin, xEnd, colIt,
          createDummyTransform(),
          createDummyTransform(colIt.size()) };

    // Extrapolation to the left.
    {
        const auto pt = interp.classifyPoint(5.0e-6);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        BOOST_CHECK_CLOSE(bg, 4.006731308598445e+01, 1.0e-10);
        BOOST_CHECK_CLOSE(bg_by_vg, 2.780521380012800e+03, 1.0e-10);
    }

    // Extrapolation to the right.
    {
        const auto pt = interp.classifyPoint(-1.0e-6);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        BOOST_CHECK_CLOSE(bg, 4.006731308598445e+01, 1.0e-10);
        BOOST_CHECK_CLOSE(bg_by_vg, 2.782452297637809e+03, 1.0e-10);
    }

    // First interval.  Rv \in (2.48e-6, 4.97e-6].
    {
        const auto Rv = std::vector<double> {
            4.97e-6, 4.47e-6, 3.97e-6, 3.47e-6, 2.97e-6, 2.48e-6 };

        const auto bg_expect = std::vector<double> {
            // Unchanging in interval...
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            2.780521380012800e+03,
            2.780909114475653e+03,
            2.781296848938506e+03,
            2.781684583401360e+03,
            2.782072317864213e+03,
            2.782452297637809e+03,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Rv.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Rv.size());

        for (const auto& Rvi : Rv) {
            const auto pt = interp.classifyPoint(Rvi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }

    // Second interval.  Rv \in (0, 2.48e-6].
    {
        const auto Rv = std::vector<double> {
            2.48e-6, 2.0e-6, 1.5e-6, 1.0e-6, 0.75e-6, 0.5e-6, 0.25e-6, 0.0,
        };

        const auto bg_expect = std::vector<double> {
            // Constant in interval
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
            4.006731308598445e+01,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            // Constant in range.
            2.782452297637809e+03,
            2.782452297637809e+03,
            2.782452297637809e+03,
            2.782452297637809e+03,
            2.782452297637809e+03,
            2.782452297637809e+03,
            2.782452297637809e+03,
            2.782452297637809e+03,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Rv.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Rv.size());

        for (const auto& Rvi : Rv) {
            const auto pt = interp.classifyPoint(Rvi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }
}

BOOST_AUTO_TEST_CASE (PVTSingleSubTable_ExtrapWithDerivatives)
{
    const auto pvtg = pvtg_subtable_incl_der();

    const auto nRows = pvtg.size() / 5;
    auto xBegin = std::begin(pvtg);
    auto xEnd   = xBegin + nRows;

    auto colIt = std::vector<decltype(xBegin)>{ xEnd };
    for (auto j = 1; j < 4; ++j) {
        colIt.push_back(colIt.back() + nRows);
    }

    const auto nResCol = std::size_t{2};
    const auto IsAscendingRange = false;
    using Extrap = PP::ExtrapolationPolicy::LinearlyWithDerivatives;
    auto interp  = PP::Linear<Extrap, IsAscendingRange>
        { Extrap{nResCol}, xBegin, xEnd, colIt,
          createDummyTransform(),
          createDummyTransform(colIt.size()) };

    // Extrapolation to the left.
    {
        const auto pt = interp.classifyPoint(6.0e-6);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::LeftOfRange);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        BOOST_CHECK_CLOSE(bg, 5.669255692405920e+01, 1.0e-10);
        BOOST_CHECK_CLOSE(bg_by_vg, 3.802317697461027e+03, 1.0e-10);
    }

    // Extrapolation to the right.
    {
        const auto pt = interp.classifyPoint(-1.0e-6);

        BOOST_CHECK(pt.cat == Opm::Interp1D::PointCategory::RightOfRange);

        const auto bg       = interp.evaluate(0, pt);
        const auto bg_by_vg = interp.evaluate(1, pt);

        BOOST_CHECK_CLOSE(bg, 5.667724096544409e+01, 1.0e-10);
        BOOST_CHECK_CLOSE(bg_by_vg, 3.803841675533160e+03, 1.0e-10);
    }

    // First interval.  Rv \in (2.61e-6, 5.21e-6].
    {
        const auto Rv = std::vector<double> {
            5.21e-6, 4.81e-6, 4.41e-6, 4.01e-6, 3.61e-6, 3.21e-6, 2.81e-6, 2.61e-6, };

        const auto bg_expect = std::vector<double> {
            5.669255626736210e+01,
            5.669156744226903e+01,
            5.669057861717595e+01,
            5.668958979208288e+01,
            5.668860096698980e+01,
            5.668761214189673e+01,
            5.668662331680365e+01,
            5.668612890425712e+01,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            3.802317657100074e+03,
            3.802643891622180e+03,
            3.802970126144287e+03,
            3.803296360666394e+03,
            3.803622595188500e+03,
            3.803948829710607e+03,
            3.804275064232714e+03,
            3.804438181493767e+03,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Rv.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Rv.size());

        for (const auto& Rvi : Rv) {
            const auto pt = interp.classifyPoint(Rvi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }

    // Second interval.  Rv \in (0, 2.61e-6].
    {
        const auto Rv = std::vector<double> {
            2.61e-6, 2.2e-6, 1.8e-6, 1.4e-6, 1.0e-6, 0.6e-6, 0.4e-6, 0.2e-6, 0.0,
        };

        const auto bg_expect = std::vector<double> {
            5.668612890425712e+01,
            5.668511947076312e+01,
            5.668413465759824e+01,
            5.668314984443336e+01,
            5.668216503126848e+01,
            5.668118021810361e+01,
            5.668068781152117e+01,
            5.668019540493873e+01,
            5.667970299835629e+01,
        };

        const auto bg_by_vg_expect = std::vector<double> {
            3.804438181493767e+03,
            3.804370434279404e+03,
            3.804304339436124e+03,
            3.804238244592843e+03,
            3.804172149749563e+03,
            3.804106054906283e+03,
            3.804073007484642e+03,
            3.804039960063002e+03,
            3.804006912641362e+03,
        };

        auto bg       = std::vector<double>{}; bg      .reserve(Rv.size());
        auto bg_by_vg = std::vector<double>{}; bg_by_vg.reserve(Rv.size());

        for (const auto& Rvi : Rv) {
            const auto pt = interp.classifyPoint(Rvi);

            bg      .push_back(interp.evaluate(0, pt));
            bg_by_vg.push_back(interp.evaluate(1, pt));
        }

        check_is_close(bg      , bg_expect      );
        check_is_close(bg_by_vg, bg_by_vg_expect);
    }
}

BOOST_AUTO_TEST_SUITE_END ()
