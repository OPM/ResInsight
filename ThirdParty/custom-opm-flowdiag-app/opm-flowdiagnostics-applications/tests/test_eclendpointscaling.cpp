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

#include <exception>
#include <initializer_list>
#include <stdexcept>
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

    // Scaled residual displacement sat: 0.7
    // Scaled Kr at Scaled Sr (KRxR):    0.6
    // Scaled Kr at Smax:     (KRx)      0.7
    const auto scaler = Opm::SatFunc::
        CritSatVerticalScaling({ 0.71 }, { 0.6 }, { 0.7 });

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
        CritSatVerticalScaling({ 0.93 }, { 0.9 }, { 1.0 });

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
