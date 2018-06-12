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

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_ECLENDPOINTSCALING

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/utility/ECLEndPointScaling.hpp>

#include <opm/utility/ECLPropTable.hpp>
#include <opm/utility/ECLPvtCommon.hpp>
#include <opm/utility/ECLUnitHandling.hpp>

#include <opm/utility/imported/Units.hpp>

#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>
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

    ::Opm::SatFunc::EPSEvalInterface::SaturationPoints
    associate(const std::vector<double>& s)
    {
        using SatAssoc = ::Opm::SatFunc::
            EPSEvalInterface::SaturationAssoc;

        auto sp = ::Opm::SatFunc::
            EPSEvalInterface::SaturationPoints{};

        sp.reserve(s.size());

        for (const auto& si : s) {
            sp.push_back(SatAssoc{ 0, si });
        }

        return sp;
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

    std::vector<double> sw_core1d_example()
    {
        return {
            1.700000000000000e-01,
            1.861700000000000e-01,
            2.023400000000000e-01,
            2.185110000000000e-01,
            2.346810000000000e-01,
            2.508510000000000e-01,
            2.670210000000000e-01,
            2.831910000000000e-01,
            2.993620000000000e-01,
            3.155320000000000e-01,
            3.317020000000000e-01,
            3.478720000000000e-01,
            3.640430000000000e-01,
            3.802130000000000e-01,
            3.963830000000000e-01,
            4.125530000000000e-01,
            4.287230000000000e-01,
            4.448940000000000e-01,
            4.610640000000000e-01,
            4.772340000000000e-01,
            4.934040000000000e-01,
            5.095740000000000e-01,
            5.257450000000000e-01,
            5.419150000000000e-01,
            5.580850000000001e-01,
            5.742550000000000e-01,
            5.904260000000000e-01,
            6.065960000000000e-01,
            6.227660000000000e-01,
            6.389359999999999e-01,
            6.551060000000000e-01,
            6.712770000000000e-01,
            6.874470000000000e-01,
            7.036170000000000e-01,
            7.197870000000000e-01,
            7.359570000000000e-01,
            7.521280000000000e-01,
            7.682980000000000e-01,
            7.844680000000001e-01,
            8.006380000000000e-01,
            8.168090000000000e-01,
            8.329790000000000e-01,
            8.491490000000000e-01,
            8.653189999999999e-01,
            8.814890000000000e-01,
            8.976600000000000e-01,
            9.138300000000000e-01,
            9.300000000000000e-01,
            1.000000000000000e+00,
        };
    }

    std::vector<double> krw_core1d_example()
    {
        return {
            0,
            4.526940000000000e-04,
            1.810770000000000e-03,
            4.074240000000000e-03,
            7.243100000000000e-03,
            1.131730000000000e-02,
            1.629700000000000e-02,
            2.218200000000000e-02,
            2.897240000000000e-02,
            3.666820000000000e-02,
            4.526940000000000e-02,
            5.477590000000000e-02,
            6.518789999999999e-02,
            7.650520000000000e-02,
            8.872790000000000e-02,
            1.018560000000000e-01,
            1.158900000000000e-01,
            1.308280000000000e-01,
            1.466730000000000e-01,
            1.634220000000000e-01,
            1.810770000000000e-01,
            1.996380000000000e-01,
            2.191040000000000e-01,
            2.394750000000000e-01,
            2.607510000000000e-01,
            2.829330000000000e-01,
            3.060210000000000e-01,
            3.300140000000000e-01,
            3.549120000000000e-01,
            3.807150000000000e-01,
            4.074240000000000e-01,
            4.350380000000000e-01,
            4.635580000000000e-01,
            4.929830000000000e-01,
            5.233139999999999e-01,
            5.545500000000000e-01,
            5.866910000000000e-01,
            6.197370000000000e-01,
            6.536890000000000e-01,
            6.885470000000000e-01,
            7.243100000000000e-01,
            7.609780000000000e-01,
            7.985510000000000e-01,
            8.370300000000001e-01,
            8.764150000000001e-01,
            9.167040000000000e-01,
            9.579000000000000e-01,
            1.000000000000000e+00,
            1.000000000000000e+00,
        };
    }

    std::vector<double> sgfn_raw()
    {
        return makeTable(5, {
                0,          0,                      0,                      0,  0,
                5.000e-02,  1.655000000000000e-03,  0,  3.310000000000000e-02,  0,
                1.000e-01,  6.913000000000000e-03,  0,  1.051600000000000e-01,  0,
                1.500e-01,  1.621300000000000e-02,  0,  1.860000000000001e-01,  0,
                2.000e-01,  2.999000000000000e-02,  0,  2.755399999999998e-01,  0,
                2.500e-01,  4.865500000000000e-02,  0,  3.733000000000000e-01,  0,
                3.000e-01,  7.257300000000000e-02,  0,  4.783600000000001e-01,  0,
                3.500e-01,  1.020460000000000e-01,  0,  5.894600000000001e-01,  0,
                4.000e-01,  1.372870000000000e-01,  0,  7.048199999999992e-01,  0,
                4.500e-01,  1.784020000000000e-01,  0,  8.223000000000005e-01,  0,
                5.000e-01,  2.253680000000000e-01,  0,  9.393200000000004e-01,  0,
                5.500e-01,  2.780300000000000e-01,  0,  1.053239999999999e+00,  0,
                6.000e-01,  3.360930000000000e-01,  0,  1.161260000000001e+00,  0,
                6.500e-01,  3.991350000000000e-01,  0,  1.260840000000000e+00,  0,
                7.000e-01,  4.666310000000000e-01,  0,  1.349920000000002e+00,  0,
                7.500e-01,  5.380000000000000e-01,  0,  1.427379999999999e+00,  0,
                8.000e-01,  6.126650000000000e-01,  0,  1.493299999999998e+00,  0,
                8.500e-01,  6.901690000000000e-01,  0,  1.550080000000002e+00,  0,
                9.000e-01,  7.703950000000001e-01,  0,  1.604519999999999e+00,  0,
                9.500e-01,  8.542180000000000e-01,  0,  1.676460000000002e+00,  0,
                9.999e-01,  9.499000000000000e-01,  0,  1.917474949899796e+00,  0,
                1.000e+00,  9.500000000000000e-01,  0,  1.000000000000000e+00,  0,
        });
    }

    std::vector<double> sofn_raw()
    {
        return makeTable(5, {
                0,                        0,                        0,                        0,                        0,
                9.999999999998899e-05,    0,                        0,                        0,                        0,
                5.000000000000004e-02,    1.000000000000000e-05,    7.000000000000000e-06,    1.999999999999998e-04,    1.402805611222443e-04,
                9.999999999999998e-02,    1.200000000000000e-04,    8.800000000000000e-05,    2.200000000000003e-03,    1.620000000000002e-03,
                1.500000000000000e-01,    5.100000000000000e-04,    3.840000000000000e-04,    7.799999999999994e-03,    5.919999999999996e-03,
                2.000000000000000e-01,    1.490000000000000e-03,    1.117000000000000e-03,    1.960000000000003e-02,    1.466000000000002e-02,
                2.500000000000000e-01,    3.460000000000000e-03,    2.597000000000000e-03,    3.939999999999996e-02,    2.959999999999997e-02,
                3.000000000000000e-01,    6.990000000000000e-03,    5.254000000000000e-03,    7.059999999999993e-02,    5.313999999999995e-02,
                3.500000000000000e-01,    1.284000000000000e-02,    9.662000000000000e-03,    1.170000000000002e-01,    8.816000000000013e-02,
                4.000000000000000e-01,    2.199000000000000e-02,    1.658600000000000e-02,    1.829999999999998e-01,    1.384799999999999e-01,
                4.500000000000000e-01,    3.572000000000000e-02,    2.703500000000000e-02,    2.746000000000004e-01,    2.089800000000003e-01,
                5.000000000000000e-01,    5.565000000000000e-02,    4.232400000000000e-02,    3.985999999999996e-01,    3.057799999999997e-01,
                5.500000000000000e-01,    8.373999999999999e-02,    6.415100000000000e-02,    5.617999999999994e-01,    4.365399999999996e-01,
                6.000000000000000e-01,    1.223700000000000e-01,    9.467100000000001e-02,    7.726000000000013e-01,    6.104000000000009e-01,
                6.500000000000000e-01,    1.741500000000000e-01,    1.365540000000000e-01,    1.035599999999999e+00,    8.376599999999993e-01,
                7.000000000000000e-01,    2.417700000000000e-01,    1.929920000000000e-01,    1.352400000000002e+00,    1.128760000000001e+00,
                7.500000000000000e-01,    3.275700000000000e-01,    2.675890000000000e-01,    1.715999999999998e+00,    1.491939999999999e+00,
                8.000000000000000e-01,    4.328600000000000e-01,    3.640430000000000e-01,    2.105799999999999e+00,    1.929079999999998e+00,
                8.500000000000000e-01,    5.571700000000001e-01,    4.855060000000000e-01,    2.486200000000004e+00,    2.429260000000003e+00,
                9.000000000000000e-01,    6.974600000000000e-01,    6.335620000000000e-01,    2.805799999999996e+00,    2.961119999999997e+00,
                9.500000000000000e-01,    8.478200000000000e-01,    8.068880000000000e-01,    3.007200000000005e+00,    3.466520000000006e+00,
                9.999000000000000e-01,    9.990000000000000e-01,    9.996137760000001e-01,    3.029659318637271e+00,    3.862239999999997e+00,
                1.000000000000000e+00,    1.000000000000000e+00,    1.000000000000000e+00,    1.000000000000111e+01,    3.862239999999221e+00,
        });
    }

    std::vector<double> swfn_raw()
    {
        return makeTable(5, {
                0,           0,             3.756330000000000e+00,    0,                        0,
                1.00e-04,    0,             3.756320000000000e+00,    0,                       -1.000000000006551e-01,
                5.00e-02,    8.6000e-04,    1.869810000000000e+00,    1.723446893787575e-02,   -3.780581162324650e+01,
                1.00e-01,    2.6300e-03,    1.237310000000000e+00,    3.539999999999999e-02,   -1.265000000000000e+01,
                1.50e-01,    5.2400e-03,    9.182100000000000e-01,    5.220000000000001e-02,   -6.382000000000001e+00,
                2.00e-01,    8.7700e-03,    7.245100000000000e-01,    7.059999999999998e-02,   -3.873999999999998e+00,
                2.50e-01,    1.3380e-02,    5.934100000000000e-01,    9.220000000000000e-02,   -2.622000000000000e+00,
                3.00e-01,    1.9270e-02,    4.981100000000000e-01,    1.178000000000000e-01,   -1.906000000000000e+00,
                3.50e-01,    2.6720e-02,    4.251100000000000e-01,    1.490000000000001e-01,   -1.460000000000000e+00,
                4.00e-01,    3.6080e-02,    3.669100000000000e-01,    1.871999999999998e-01,   -1.163999999999998e+00,
                4.50e-01,    4.7810e-02,    3.191100000000000e-01,    2.346000000000000e-01,   -9.560000000000004e-01,
                5.00e-01,    6.2500e-02,    2.788100000000000e-01,    2.938000000000001e-01,   -8.060000000000003e-01,
                5.50e-01,    8.0900e-02,    2.440100000000000e-01,    3.679999999999997e-01,   -6.959999999999993e-01,
                6.00e-01,    1.0394e-01,    2.135100000000000e-01,    4.608000000000007e-01,   -6.100000000000008e-01,
                6.50e-01,    1.3277e-01,    1.863100000000000e-01,    5.765999999999993e-01,   -5.439999999999996e-01,
                7.00e-01,    1.6869e-01,    1.616100000000000e-01,    7.184000000000011e-01,   -4.940000000000007e-01,
                7.50e-01,    2.1302e-01,    1.390100000000000e-01,    8.865999999999988e-01,   -4.519999999999998e-01,
                8.00e-01,    2.6667e-01,    1.180100000000000e-01,    1.073000000000000e+00,   -4.199999999999994e-01,
                8.50e-01,    3.2918e-01,    9.830999999999999e-02,    1.250200000000001e+00,   -3.940000000000007e-01,
                9.00e-01,    3.9706e-01,    7.961000000000000e-02,    1.357600000000000e+00,   -3.739999999999996e-01,
                9.50e-01,    4.6103e-01,    6.161000000000000e-02,    1.279400000000001e+00,   -3.600000000000005e-01,
                1.00e+00,    5.0000e-01,    4.408000000000000e-02,    7.793999999999994e-01,   -3.505999999999996e-01,
        });
    }

    Opm::ECLPropTableRawData makeSatFunc(std::vector<double> data)
    {
        auto table = Opm::ECLPropTableRawData{};

        table.data = std::move(data);

        table.numTables  = 1;
        table.numPrimary = 1;
        table.numCols    = 5;
        table.numRows    = table.data.size() / table.numCols;

        return table;
    }

    Opm::SatFuncInterpolant sgfn()
    {
        // Input tables follow METRIC unit conventions.
        const auto usys = Opm::ECLUnits::createUnitSystem(1);

        auto convert = Opm::SatFuncInterpolant::ConvertUnits{};

        convert.indep = [](const double s) { return s; };

        // Krg(Sg)
        convert.column.emplace_back(
            [](const double kr) { return kr; });

        // Pcgo(Sg)
        convert.column.push_back(
            Opm::ECLPVT::CreateUnitConverter::ToSI::pressure(*usys));

        // dKrg/dSg
        convert.column.emplace_back(
            [](const double dkr) { return dkr; });

        // dPcgo/dSg
        convert.column.push_back(
            Opm::ECLPVT::CreateUnitConverter::ToSI::pressure(*usys));

        return {
            makeSatFunc(sgfn_raw()), convert
        };
    }

    Opm::SatFuncInterpolant sofn()
    {
        auto convert = Opm::SatFuncInterpolant::ConvertUnits{};

        convert.indep = [](const double s) { return s; };

        // Krow(So)
        convert.column.emplace_back(
            [](const double kr) { return kr; });

        // Krog(So)
        convert.column.emplace_back(
            [](const double kr) { return kr; });

        // dKrow/dSo
        convert.column.emplace_back(
            [](const double dkr) { return dkr; });

        // dKrow/dSo
        convert.column.emplace_back(
            [](const double dkr) { return dkr; });

        return {
            makeSatFunc(sofn_raw()), convert
        };
    }

    Opm::SatFuncInterpolant swfn()
    {
        // Input tables follow METRIC unit conventions.
        const auto usys = Opm::ECLUnits::createUnitSystem(1);

        auto convert = Opm::SatFuncInterpolant::ConvertUnits{};

        convert.indep = [](const double s) { return s; };

        // Krw(Sw)
        convert.column.emplace_back(
            [](const double kr) { return kr; });

        // Pcow(Sw)
        convert.column.push_back(
            Opm::ECLPVT::CreateUnitConverter::ToSI::pressure(*usys));

        // dKrw/dSw
        convert.column.emplace_back(
            [](const double dkr) { return dkr; });

        // dPcow/dSw
        convert.column.push_back(
            Opm::ECLPVT::CreateUnitConverter::ToSI::pressure(*usys));

        return {
            makeSatFunc(swfn_raw()), convert
        };
    }
} // Namespace Anonymous

// =====================================================================
// Two-point scaling
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (TwoPointScaling_FullRange)

BOOST_AUTO_TEST_CASE (NoScaling)
{
    namespace SF = ::Opm::SatFunc;

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.0, 0.0, 1.0 };

    const auto smin = std::vector<double>{ 0.0 };
    const auto smax = std::vector<double>{ 1.0 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        check_is_close(s_inp, s);
    }
}

BOOST_AUTO_TEST_CASE (ScaledConnate)
{
    namespace SF = ::Opm::SatFunc;

    // Mobile Range: [0.2, 1.0] maps to [ 0.0, 1.0 ]
    const auto smin = std::vector<double>{ 0.2 };
    const auto smax = std::vector<double>{ 1.0 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.0, 0.0, 1.0 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0,
        0,
        0.25,
        0.5,
        0.75,
        1.0,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        const auto s_inp_expect = std::vector<double> {
            0.2,                // t.s <= smin => smin
            0.2,                // t.s <= smin => smin
            0.4,
            0.6,
            0.8,
            1.0,
        };

        check_is_close(s_inp, s_inp_expect);
    }
}

BOOST_AUTO_TEST_CASE (ScaledMax)
{
    namespace SF = ::Opm::SatFunc;

    // Mobile Range: [0.0, 0.8] maps to [ 0.0, 1.0 ]
    const auto smin = std::vector<double>{ 0.0 };
    const auto smax = std::vector<double>{ 0.8 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.0, 0.0, 1.0 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0,
        0.25,
        0.5,
        0.75,
        1.0,
        1.0,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        const auto s_inp_expect = std::vector<double> {
            0.0,
            0.2,
            0.4,
            0.6,
            0.8,
            0.8,                // t.s >= smax => smax
        };

        check_is_close(s_inp, s_inp_expect);
    }
}

BOOST_AUTO_TEST_CASE (ScaledBoth)
{
    namespace SF = ::Opm::SatFunc;

    // Mobile Range: [0.2, 0.8] maps to [ 0.0, 1.0 ]
    const auto smin = std::vector<double>{ 0.2 };
    const auto smax = std::vector<double>{ 0.8 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.0, 0.0, 1.0 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0,
        0.0,
        1.0 / 3.0,
        2.0 / 3.0,
        1.0,
        1.0,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        const auto s_inp_expect = std::vector<double> {
            0.2,                // t.s <= smin => smin
            0.2,
            0.4,
            0.6,
            0.8,
            0.8,                // t.s >= smax => smax
        };

        check_is_close(s_inp, s_inp_expect);
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (TwoPointScaling_ReducedRange)

BOOST_AUTO_TEST_CASE (NoScaling)
{
    namespace SF = ::Opm::SatFunc;

    const auto smin = std::vector<double>{ 0.2 };
    const auto smax = std::vector<double>{ 0.8 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.2, 0.0, 0.8 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0.2,
        0.2,
        0.4,
        0.6,
        0.8,
        0.8,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        check_is_close(s_inp, expect);
    }
}

BOOST_AUTO_TEST_CASE (ScaledConnate)
{
    namespace SF = ::Opm::SatFunc;

    // Mobile Range: [0.0, 1.0] maps to [ 0.2, 0.8 ]
    // s_eff = 0.6*s + 0.2
    const auto smin = std::vector<double>{ 0.0 };
    const auto smax = std::vector<double>{ 1.0 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.2, 0.0, 0.8 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0.20,
        0.32,
        0.44,
        0.56,
        0.68,
        0.80,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        check_is_close(s_inp, s);
    }
}

BOOST_AUTO_TEST_CASE (ScaledMax)
{
    namespace SF = ::Opm::SatFunc;

    // Mobile Range: [0.2, 0.8] maps to [ 0.0, 1.0 ]
    // s_eff = max(0.75*s + 0.05, 0.2)
    const auto smin = std::vector<double>{ 0.2 };
    const auto smax = std::vector<double>{ 1.0 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.2, 0.0, 0.8 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0.20,
        0.20,
        0.35,
        0.50,
        0.65,
        0.80,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        const auto s_inp_expect = std::vector<double> {
            0.2,                // t.s <= smin -> smin
            0.2,
            0.4,
            0.6,
            0.8,
            1.0,
        };

        check_is_close(s_inp, s_inp_expect);
    }
}

BOOST_AUTO_TEST_CASE (ScaledBoth)
{
    namespace SF = ::Opm::SatFunc;

    // Mobile Range: [0.2, 0.8] maps to [ 0.5, 0.7 ]
    // s_eff = min(max(0.2, 3*s - 13/10), 0.8)
    const auto smin = std::vector<double>{ 0.5 };
    const auto smax = std::vector<double>{ 0.7 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.2, 0.0, 0.8 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0.2,
        0.2,
        0.2,
        0.5,
        0.8,
        0.8,
    };

    const auto eps = SF::TwoPointScaling{ smin, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp     = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        const auto s_inp_expect = std::vector<double> {
            0.5,                // t.s <= smin -> smin
            0.5,                // t.s <= smin -> smin
            0.5,                // t.s <= smin -> smin
            0.6,
            0.7,                // t.s >= smax -> smax
            0.7,                // t.s >= smax -> smax
        };

        check_is_close(s_inp, s_inp_expect);
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (HorizontalScaling_SatFuncCurves)

BOOST_AUTO_TEST_CASE (KrGas_2Pt)
{
    const auto interp = sgfn();

    using Interp = std::remove_cv<
        std::remove_reference<decltype(interp)>::type
        >::type;

    const auto SG = std::vector<double>{
        0.0, 0.025, 0.05, 0.075, 0.1    , 0.125, 0.15 , 0.175,
        0.2, 0.225, 0.25, 0.275, 0.3    , 0.325, 0.35 , 0.375,
        0.4, 0.425, 0.45, 0.475, 0.49995, 0.5,
    };

    // Live Sg in [0, 0.5].  Map to [0, 1] for table lookup
    const auto eps = ::Opm::SatFunc::
        TwoPointScaling{ { 0.0 }, { 0.5 } };

    const auto tep = ::Opm::SatFunc::EPSEvalInterface::TableEndPoints {
        interp.connateSat()[0] , // low
        interp.connateSat()[0] , // disp (ignored)
        interp.maximumSat()[0]   // max
    };

    // Forward: Scaled SG -> Lookup/Table sg
    {
        const auto expect = interp.saturationPoints(Interp::InTable{0});
        const auto sg     = eps.eval(tep, associate(SG));

        check_is_close(sg, expect);
    }

    // Reverse: Lookup/Table sg -> Scaled SG
    {
        const auto sg     = interp.saturationPoints(Interp::InTable{0});
        const auto scaled = eps.reverse(tep, associate(sg));

        check_is_close(scaled, SG);
    }
}

BOOST_AUTO_TEST_CASE (KrOW_3pt)
{
    const auto interp = sofn();

    using Interp = std::remove_cv<
        std::remove_reference<decltype(interp)>::type
        >::type;

    const auto swl  = 0.1;
    const auto swcr = 0.1;      // Swcr == Swco in table.
    const auto sgl  = 0.0;

    // Scaled end-points:
    //   SOWCR = 0.2,  SWCR = 0.25,  SWL = 0.15,  SGL = 0
    //     => Mobile So in [ 0.2, 0.85 ], Sr = 0.75.

    const auto eps = ::Opm::SatFunc::
        ThreePointScaling{ { 0.2 }, { 0.75 }, { 0.85 } };

    const auto tep = ::Opm::SatFunc::EPSEvalInterface::TableEndPoints {
        interp.criticalSat(Interp::ResultColumn{0})[0], // low
        1.0 - (swcr + sgl),                             // disp (Sr)
        1.0 - (swl  + sgl),                             // max
    };

    // Forward: Scaled SO -> Lookup/Table so.
    {
        const auto SO = std::vector<double> {
            0.0, 0.05, 0.1, 0.15,

            // ------------ Sowcr -----------------

            0.2, 0.25, 0.3, 0.35, 0.4, 0.45,
            0.5, 0.55, 0.6, 0.65, 0.7, 0.725, 0.749,

            // ------------ Sr --------------------

            0.75, 0.7501, 0.8, 0.825, 0.85,
        };

        const auto so = eps.eval(tep, associate(SO));

        const auto expect = std::vector<double>{
            // so = Sowcr.  (SO < SLO)

            9.9999999999988987e-05, // SO = 0.0
            9.9999999999988987e-05, // SO = 0.05
            9.9999999999988987e-05, // SO = 0.1
            9.9999999999988987e-05, // SO = 0.15

            // ------------ SLO ------------------

            // so = tep.low + t*(tep.disp - tep.low)
            //    = 1.0e-4  + t*(0.9      - 1.0e-4)
            //
            // t = (SO - SLO) / (SR   - SLO)
            //   = (SO - 0.2) / (0.75 - 0.2)

            9.9999999999988987e-05, // SO = 0.2
            0.081909090909090876,   // SO = 0.25
            0.16371818181818176,    // SO = 0.3
            0.24552727272727265,    // SO = 0.35
            0.32733636363636365,    // SO = 0.4
            0.40914545454545453,    // SO = 0.45
            0.49095454545454542,    // SO = 0.5
            0.57276363636363636,    // SO = 0.55
            0.6545727272727272,     // SO = 0.6
            0.73638181818181814,    // SO = 0.65
            0.81819090909090897,    // SO = 0.7
            0.85909545454545433,    // SO = 0.725
            0.89836381818181799,    // SO = 0.749

            // ------------ Sr --------------------

            // Everything below here maps to Somax (=1-Swco) because the
            // 'tep' data says that Sdisp == Smax in the (purported) input
            // table.  The actual SOFN data (sofn_raw()) does not account
            // for Swco = 0.1 (> 0) and therefore provides satfunc values
            // for So > 0.9.

            0.90000000000000002, // SO = 0.75
            0.90000000000000002, // SO = 0.751
            0.90000000000000002, // SO = 0.8
            0.90000000000000002, // SO = 0.825
            0.90000000000000002, // SO = 0.85
        };

        check_is_close(so, expect);
    }

    // Reverse: Lookup/Table so -> Scaled SO.
    {
        const auto SO = std::vector<double> {
            0.20000000000000001, // so = 0
            0.20000000000000001, // so = 9.9999999999988987e-05
            0.23049783309256588, // so = 0.050000000000000037
            0.2610567840871208,  // so = 0.099999999999999978
            0.29161573508167576, // so = 0.14999999999999999
            0.32217468607623073, // so = 0.20000000000000001
            0.35273363707078564, // so = 0.25
            0.38329258806534061, // so = 0.29999999999999999
            0.41385153905989558, // so = 0.34999999999999998
            0.44441049005445055, // so = 0.40000000000000002
            0.47496944104900546, // so = 0.45000000000000001
            0.50552839204356048, // so = 0.5
            0.53608734303811545, // so = 0.55000000000000004
            0.56664629403267042, // so = 0.59999999999999998
            0.59720524502722527, // so = 0.65000000000000002
            0.62776419602178024, // so = 0.69999999999999996
            0.6583231470163351,  // so = 0.75
            0.68888209801089018, // so = 0.80000000000000004
            0.71944104900544503, // so = 0.84999999999999998
            0.84999999999999998, // so = 0.90000000000000002
            0.84999999999999998, // so = 0.94999999999999996
            0.84999999999999998, // so = 0.99990000000000001
            0.84999999999999998, // so = 1
        };

        const auto so     = interp.saturationPoints(Interp::InTable{0});
        const auto scaled = eps.reverse(tep, associate(so));

        check_is_close(scaled, SO);
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Three-point (alternative) scaling, applicable to relperm only.
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (ThreePointScaling_FullRange)

BOOST_AUTO_TEST_CASE (NoScaling)
{
    namespace SF = ::Opm::SatFunc;

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.0, 0.2, 1.0 };

    const auto smin  = std::vector<double>{ 0.0 };
    const auto sdisp = std::vector<double>{ 0.2 };
    const auto smax  = std::vector<double>{ 1.0 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto eps = SF::ThreePointScaling{ smin, sdisp, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        check_is_close(s_inp, s);
    }
}

BOOST_AUTO_TEST_CASE (ScaledConnate)
{
    namespace SF = ::Opm::SatFunc;

    // Mobile Range: [0.4, 1.0] maps to [ 0.0, 1.0 ]
    const auto smin  = std::vector<double>{ 0.1 };
    const auto sdisp = std::vector<double>{ 0.4 };
    const auto smax  = std::vector<double>{ 1.0 };

    const auto tep = SF::EPSEvalInterface::
        TableEndPoints { 0.0, 0.2, 1.0 };

    const auto s = std::vector<double> {
        0.0,
        0.2,
        0.4,
        0.6,
        0.8,
        1.0,
    };

    const auto expect = std::vector<double> {
        0,
        1.0  / 15,
        0.2,
        7.0  / 15,
        11.0 / 15,
        1.0,
    };

    const auto eps = SF::ThreePointScaling{ smin, sdisp, smax };

    // Input saturation -> Scaled saturation
    {
        const auto sp    = associate(s);
        const auto s_eff = eps.eval(tep, sp);

        check_is_close(s_eff, expect);
    }

    // Tabulated saturation -> Input saturation
    {
        const auto sp    = associate(expect);
        const auto s_inp = eps.reverse(tep, sp);

        const auto s_inp_expect = std::vector<double> {
            0.1,                // t.s <= smin -> smin
            0.2,
            0.4,
            0.6,
            0.8,
            1.0,
        };

        check_is_close(s_inp, s_inp_expect);
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Pure vertical scaling of SF values.
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (PureVerticalScaling_SFValues)

BOOST_AUTO_TEST_CASE (Parabola_ScaledCell)
{
    using SFPt  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues::Point;
    using SFVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    // val = linspace(0, 1, 11) .^ 2
    const auto val = std::vector<double> {
        0,
        1.0e-02,
        4.0e-02,
        9.0e-02,
        1.6e-01,
        2.5e-01,
        3.6e-01,
        4.9e-01,
        6.4e-01,
        8.1e-01,
        1.0e+00,
    };

    // Maximum value in cell is 0.5.
    const auto scaler = Opm::SatFunc::PureVerticalScaling({ 0.5 });

    // This is a lie.  We do however not actually use the sp[i].sat values
    // in *this particular application (pure v-scaling)* though--we only
    // care about sp[i].cell--so we can get away with pretending that the
    // function values (val) are saturations.
    const auto sp = associate(val);

    const auto f = SFVal{
        SFPt{ 0.0, 0.0 }, // Displacement
        SFPt{ 1.0, 1.0 }, // Maximum
    };

    const auto y = scaler.vertScale(f, sp, val);

    // expect = 0.5 * val
    const auto expect = std::vector<double> {
        0,
        5.00e-03,
        2.00e-02,
        4.50e-02,
        8.00e-02,
        1.25e-01,
        1.80e-01,
        2.45e-01,
        3.20e-01,
        4.05e-01,
        5.00e-01,
    };

    check_is_close(y, expect);
}

BOOST_AUTO_TEST_CASE (Parabola_ScaledFunc)
{
    using SFPt  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues::Point;
    using SFVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    // val = linspace(0, 1, 11) .^ 2
    const auto val = std::vector<double> {
        0,
        1.0e-02,
        4.0e-02,
        9.0e-02,
        1.6e-01,
        2.5e-01,
        3.6e-01,
        4.9e-01,
        6.4e-01,
        8.1e-01,
        1.0e+00,
    };

    // Maximum value in cell is 1.
    const auto scaler = Opm::SatFunc::PureVerticalScaling({ 1.0 });

    // This is a lie.  We do however not actually use the sp[i].sat values
    // in *this particular application (pure v-scaling)* though--we only
    // care about sp[i].cell--so we can get away with pretending that the
    // function values (val) are saturations.
    const auto sp = associate(val);

    const auto f = SFVal{
        SFPt{ 0.0, 0.0 }, // Displacement
        SFPt{ 1.0, 2.0 }, // Maximum
    };

    const auto y = scaler.vertScale(f, sp, val);

    // expect = val / 2
    const auto expect = std::vector<double> {
        0,
        5.00e-03,
        2.00e-02,
        4.50e-02,
        8.00e-02,
        1.25e-01,
        1.80e-01,
        2.45e-01,
        3.20e-01,
        4.05e-01,
        5.00e-01,
    };

    check_is_close(y, expect);
}

BOOST_AUTO_TEST_CASE (Parabola_ScaledBoth)
{
    using SFPt  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues::Point;
    using SFVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    // val = linspace(0, 1, 11) .^ 2
    const auto val = std::vector<double> {
        0,
        1.0e-02,
        4.0e-02,
        9.0e-02,
        1.6e-01,
        2.5e-01,
        3.6e-01,
        4.9e-01,
        6.4e-01,
        8.1e-01,
        1.0e+00,
    };

    // Maximum value in cell is 1.5.
    const auto scaler = Opm::SatFunc::PureVerticalScaling({ 1.5 });

    // This is a lie.  We do however not actually use the sp[i].sat values
    // in *this particular application (pure v-scaling)* though--we only
    // care about sp[i].cell--so we can get away with pretending that the
    // function values (val) are saturations.
    const auto sp = associate(val);

    const auto f = SFVal{
        SFPt{ 0.0, 0.0 }, // Displacement
        SFPt{ 1.0, 2.0 }, // Maximum
    };

    const auto y = scaler.vertScale(f, sp, val);

    // expect = val * 0.75
    const auto expect = std::vector<double> {
        0,
        7.500e-03,
        3.000e-02,
        6.750e-02,
        1.200e-01,
        1.875e-01,
        2.700e-01,
        3.675e-01,
        4.800e-01,
        6.075e-01,
        7.500e-01,
    };

    check_is_close(y, expect);
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Critical saturation vertical scaling of SF values.
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (CritSatVScale_SFValues)

/*
  sw = 0.1 : 0.05 : 0.9;
  kr = sw .^ 2;

  [sdisp, fdisp, fmax] = deal(0.7, 0.6, 0.7);

  i = ~ (sw > sdisp);
  y = 0 * kr;
  y( i) = kr(i) .* fdisp./sdisp^2;
  y(~i) = fdisp + (kr(~i) - sdisp^2)./(kr(end) - sdisp^2) .* (fmax - fdisp);

  figure, plot(sw, [kr; y], '.-')
  legend('Unscaled', 'Vert. Scaled', 'Location', 'Best')
*/
BOOST_AUTO_TEST_CASE (Sw2_Regular)
{
    using SFPt  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues::Point;
    using SFVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    const auto sw = associate({
        1.0e-01, 1.5e-01, 2.0e-01, 2.5e-01,
        3.0e-01, 3.5e-01, 4.0e-01, 4.5e-01,
        5.0e-01, 5.5e-01, 6.0e-01, 6.5e-01,
        7.0e-01, 7.5e-01, 8.0e-01, 8.5e-01,
        9.0e-01,
    });

    const auto kr = std::vector<double> { // sw^2
        1.000e-02, 2.250e-02, 4.000e-02, 6.250e-02, //  0 ..  3
        9.000e-02, 1.225e-01, 1.600e-01, 2.025e-01, //  4 ..  7
        2.500e-01, 3.025e-01, 3.600e-01, 4.225e-01, //  8 .. 11
        4.900e-01, 5.625e-01, 6.400e-01, 7.225e-01, // 12 .. 15
        8.100e-01,                                  // 16
    };

    const auto f = SFVal{
        SFPt{ 0.7, 0.49 }, // Displacement
        SFPt{ 0.9, 0.81 }, // Maximum
    };

    // Scaled residual displacement sat: 0.7 (unchanged)
    // Scaled Kr at Scaled Sr (KRxR):    0.6
    // Scaled maximum saturation:        0.9 (unchanged)
    // Scaled Kr at Smax:     (KRx)      0.7
    const auto scaler = Opm::SatFunc::
        CritSatVerticalScaling({ 0.71 }, { 0.6 }, { 0.9 }, { 0.7 });

    const auto y = scaler.vertScale(f, sw, kr);

    // expect = kr .* (KRxR/0.49), S \le Sr
    //          KRxR + (kr - 0.49)/(0.81 - 0.49)*(KRx - KRxR), Sr < S
    const auto expect = std::vector<double> {
        1.224489795918368e-02,  //  0
        2.755102040816328e-02,  //  1
        4.897959183673471e-02,  //  2
        7.653061224489796e-02,  //  3
        1.102040816326531e-01,  //  4
        1.500000000000000e-01,  //  5
        1.959183673469388e-01,  //  6
        2.479591836734695e-01,  //  7
        3.061224489795918e-01,  //  8
        3.704081632653062e-01,  //  9
        4.408163265306123e-01,  // 10
        5.173469387755103e-01,  // 11

        // ------- Sr -------

        6.000000000000000e-01,  // 12
        6.226562500000000e-01,  // 13
        6.468750000000000e-01,  // 14
        6.726562500000000e-01,  // 15
        7.000000000000000e-01,  // 16
    };

    check_is_close(y, expect);
}

BOOST_AUTO_TEST_CASE (Core1D_Coincident_FVal)
{
    using SFPt  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues::Point;
    using SFVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    const auto s  = associate(sw_core1d_example());
    const auto kr = krw_core1d_example();

    const auto f = SFVal{
        SFPt{ 0.93, 1.0 }, // Displacement
        SFPt{ 1.0 , 1.0 }, // Maximum
    };

    const auto scaler = Opm::SatFunc::
        CritSatVerticalScaling({ 0.93 }, { 0.9 }, { 1.0 }, { 1.0 });

    const auto y = scaler.vertScale(f, s, kr);

    const auto expect = std::vector<double> {
        0,
        4.074246000000000e-04,
        1.629693000000000e-03,
        3.666816000000000e-03,
        6.518790000000000e-03,
        1.018557000000000e-02,
        1.466730000000000e-02,
        1.996380000000000e-02,
        2.607516000000000e-02,
        3.300138000000000e-02,
        4.074246000000000e-02,
        4.929831000000000e-02,
        5.866911000000000e-02,
        6.885468000000000e-02,
        7.985511000000001e-02,
        9.167040000000000e-02,
        1.043010000000000e-01,
        1.177452000000000e-01,
        1.320057000000000e-01,
        1.470798000000000e-01,
        1.629693000000000e-01,
        1.796742000000000e-01,
        1.971936000000000e-01,
        2.155275000000000e-01,
        2.346759000000000e-01,
        2.546397000000000e-01,
        2.754189000000000e-01,
        2.970126000000000e-01,
        3.194208000000000e-01,
        3.426435000000000e-01,
        3.666816000000000e-01,
        3.915342000000000e-01,
        4.172022000000000e-01,
        4.436847000000000e-01,
        4.709826000000000e-01,
        4.990950000000000e-01,
        5.280218999999999e-01,
        5.577633000000000e-01,
        5.883201000000000e-01,
        6.196923000000001e-01,
        6.518790000000000e-01,
        6.848802000000001e-01,
        7.186959000000001e-01,
        7.533270000000001e-01,
        7.887735000000000e-01,
        8.250336000000000e-01,
        8.621100000000000e-01,
        9.000000000000000e-01,
        1.000000000000000e+00,
    };

    check_is_close(y, expect);
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (VerticalScaling_SatFuncCurves)

BOOST_AUTO_TEST_CASE (Pcow_2Pt)
{
    using SFPt  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues::Point;
    using SFVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    const auto interp = swfn();

    using Interp = std::remove_cv<
        std::remove_reference<decltype(interp)>::type
        >::type;

    using ImportedOpm::unit::barsa;

    // Maximum Pcow reset to 10 barsa.
    const auto vscale = ::Opm::SatFunc::PureVerticalScaling {
        { 10.0*barsa }
    };

    const auto sw = interp.saturationPoints(Interp::InTable{0});
    const auto pc = interp           //  Pc_ow = ResultColumn{1}
        .interpolate(Interp::InTable{0}, Interp::ResultColumn{1}, sw);

    const auto f = SFVal{
        SFPt{ 1.0, 0.04408*barsa }, // Displacement
        SFPt{ 0.0, 3.75633*barsa }, // Maximum
    };

    const auto PC = vscale.vertScale(f, associate(sw), pc);

    const auto expect = std::vector<double> { // pc * (10.0 / 3.75633)
        1000000,
        999997.33782708121,
        497775.75452635949,
        329393.31741353922,
        244443.37957527695,
        192877.09013851287,
        157976.00317331014,
        132605.49525733895,
        113171.63295024665,
        97677.786562948415,
        84952.60001118113,
        74224.043148498662,
        64959.681391145081,
        56840.053988866792,
        49598.943649785826,
        43023.376540399804,
        37006.865743957533,
        31416.302614520024,
        26171.821964523886,
        21193.558606405721,
        16401.64735260214,
        11734.85822598121,
    };
}

BOOST_AUTO_TEST_CASE (KrOG_3Pt)
{
    using SFPt  = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues::Point;
    using SFVal = ::Opm::SatFunc::VerticalScalingInterface::FunctionValues;

    const auto interp = sofn();

    using Interp = std::remove_cv<
        std::remove_reference<decltype(interp)>::type
        >::type;

    const auto so   = interp.saturationPoints(Interp::InTable{0});
    const auto krog = interp           // krog = ResultColumn{1}
        .interpolate(Interp::InTable{0}, Interp::ResultColumn{1}, so);

    const auto f = SFVal{
        SFPt{ 0.6, 0.094671 }, // Displacement
        SFPt{ 1.0, 1.0 },      // Maximum
    };

    const auto vscale = ::Opm::SatFunc::CritSatVerticalScaling {
        // s   ,   Kr
        { 0.6 }, { 0.25 },
        { 1.0 }, { 0.6  }
    };

    const auto KROG = vscale.vertScale(f, associate(so), krog);

    const auto expect = std::vector<double> {
        // Left interval (So <= Sr = f.disp.sat = 0.6)
        //
        // Pure vertical scaling (multiply by a constant factor).
        //
        //   Kr = Kr(So) * (KRORG / Kr(Sr; table))
        //      = Kr(So) * (KRORG / f.disp.val)
        //      = Kr(So) * (0.25 / 0.094671)
        //
        0,                      // So = 0,      Kr(So) = 0,
        0,                      // So = 1.0e-4  Kr(So) = 0,
        1.8485069345417287e-05, // So = 0.05    Kr(So) = 7.0000e-06
        0.00023238372891381731, // So = 0.1     Kr(So) = 8.8000e-05
        0.0010140380898057484,  // So = 0.15    Kr(So) = 3.8400e-04
        0.0029496889226901584,  // So = 0.2     Kr(So) = 1.1170e-03
        0.0068579607271498132,  // So = 0.25    Kr(So) = 2.5970e-03
        0.013874364905831774,   // So = 0.3     Kr(So) = 5.2540e-03
        0.025514677145060262,   // So = 0.35    Kr(So) = 9.6620e-03
        0.043799051451870158,   // So = 0.4     Kr(So) = 1.6586e-02
        0.071391978536193765,   // So = 0.45    Kr(So) = 2.7035e-02
        0.11176601071077732,    // So = 0.5     Kr(So) = 4.2324e-02
        0.16940509765398062,    // So = 0.55    Kr(So) = 6.4151e-02
        0.25,                   // So = 0.6     Kr(So) = 9.4671e-02

        // ------------------------------------------
        //
        // Right interval (So >= Sr)
        //
        // Scaled Kr values defined by linear function between relperm
        // points (Kr(Sr),KRORG) and (Krmax,KRO)
        //
        //                  Kr(So) - Kr(Sr)
        //    Kr = KRORG + ----------------- (KRO - KRORG)
        //                  Krmax  - Kr(Sr)
        //
        //                    Kr(So)  - f.disp.val
        //       = KRORG + ------------------------ (KRO - KRORG)
        //                  f.max.val - f.disp.val
        //
        //                 Kr(So) - 0.094671
        //       = 0.25 + ------------------- (0.6 - 0.25)
        //                 1      - 0.094671
        //
        0.26619195894531161,    // So = 0.65    Kr(So) = 0.136554
        0.28801087781347995,    // So = 0.7     Kr(So) = 0.192992
        0.31685006224256596,    // So = 0.75    Kr(So) = 0.267589
        0.35413915825075742,    // So = 0.8     Kr(So) = 0.364043
        0.40109672837167476,    // So = 0.85    Kr(So) = 0.485506
        0.45833514667043695,    // So = 0.9     Kr(So) = 0.633562
        0.52534294162674566,    // So = 0.95    Kr(So) = 0.806888
        0.59985068588325352,    // So = 0.9999  Kr(So) = 0.999613776
        0.59999999999999998,    // So = 1.0     Kr(So) = 1.0
    };

    check_is_close(KROG, expect);
}

BOOST_AUTO_TEST_SUITE_END ()
