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

#define BOOST_TEST_MODULE TEST_ECLENDPOINTSCALING

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/utility/ECLEndPointScaling.hpp>

#include <exception>
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
