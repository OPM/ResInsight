/*
  Copyright 2017 SINTEF ICT, Applied Mathematics.
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

#define BOOST_TEST_MODULE TEST_ASSEMBLED_CONNECTIONS

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/utility/ECLUnitHandling.hpp>

#include <exception>
#include <stdexcept>

BOOST_AUTO_TEST_SUITE (Basic_Conversion)

BOOST_AUTO_TEST_CASE (Constructor)
{
    auto M = ::Opm::ECLUnits::createUnitSystem(1); // METRIC
    auto F = ::Opm::ECLUnits::createUnitSystem(2); // FIELD
    auto L = ::Opm::ECLUnits::createUnitSystem(3); // LAB
    auto P = ::Opm::ECLUnits::createUnitSystem(4); // PVT-M

    BOOST_CHECK_THROW(::Opm::ECLUnits::createUnitSystem( 5), std::runtime_error);
    BOOST_CHECK_THROW(::Opm::ECLUnits::createUnitSystem(-1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (Metric)
{
    auto M = ::Opm::ECLUnits::createUnitSystem(1);

    // Pressure (bars)
    {
        const auto scale  = M->pressure();
        const auto expect = 100.0e3;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume Rate (rm^3 / day)
    {
        const auto scale  = M->reservoirRate();
        const auto expect = 1.157407407407407e-05;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume (rm^3)
    {
        const auto scale  = M->reservoirVolume();
        const auto expect = 1.0;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Time (day)
    {
        const auto scale  = M->time();
        const auto expect = 86.400e+03;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Transmissibility ((cP * m^3) / (day * barsa))
    {
        const auto scale  = M->transmissibility();
        const auto expect = 1.157407407407407e-13;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE (Field)
{
    auto F = ::Opm::ECLUnits::createUnitSystem(2);

    // Pressure (psi)
    {
        const auto scale  = F->pressure();
        const auto expect = 6.894757293168360e+03;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume Rate (rb / day)
    {
        const auto scale  = F->reservoirRate();
        const auto expect = 1.840130728333334e-06;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume (rb)
    {
        const auto scale  = F->reservoirVolume();
        const auto expect = 1.589872949280001e-01;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Time (day)
    {
        const auto scale  = F->time();
        const auto expect = 86.400e+03;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Transmissibility ((cP * rb) / (day * psia))
    {
        const auto scale  = F->transmissibility();
        const auto expect = 2.668883979653090e-13;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE (Lab)
{
    auto L = ::Opm::ECLUnits::createUnitSystem(3);

    // Pressure (atm)
    {
        const auto scale  = L->pressure();
        const auto expect = 101.325e+03;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume Rate (r(cm)^3 / h)
    {
        const auto scale  = L->reservoirRate();
        const auto expect = 2.777777777777778e-10;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume (r(cm)^3)
    {
        const auto scale  = L->reservoirVolume();
        const auto expect = 1.0e-06;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Time (hour)
    {
        const auto scale  = L->time();
        const auto expect = 3600.0;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Transmissibility ((cP * (cm)^3) / (h * atm))
    {
        const auto scale  = L->transmissibility();
        const auto expect = 2.741453518655592e-18;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    auto P = ::Opm::ECLUnits::createUnitSystem(4);

    // Pressure (atm)
    {
        const auto scale  = P->pressure();
        const auto expect = 101.325e+03;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume Rate (rm^3 / day)
    {
        const auto scale  = P->reservoirRate();
        const auto expect = 1.157407407407407e-05;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Reservoir Volume (rm^3)
    {
        const auto scale  = P->reservoirVolume();
        const auto expect = 1.0;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Time (day)
    {
        const auto scale  = P->time();
        const auto expect = 86.400e+03;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }

    // Transmissibility ((cP * rm^3 / (day * atm))
    {
        const auto scale  = P->transmissibility();
        const auto expect = 1.142272299439830e-13;

        BOOST_CHECK_CLOSE(scale, expect, 1.0e-10);
    }
}

BOOST_AUTO_TEST_SUITE_END ()
