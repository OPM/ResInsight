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

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_ECLPROPERTYUNITCONVERSION

#include <boost/test/unit_test.hpp>

#include <opm/utility/ECLPropertyUnitConversion.hpp>

#include <opm/utility/ECLUnitHandling.hpp>

#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
    std::vector<double> unity(const std::vector<double>::size_type n = 1)
    {
        return std::vector<double>(n, 1.0);
    }

    template <class Coll1, class Coll2>
    void check_is_close(const Coll1& c1, const Coll2& c2)
    {
        BOOST_REQUIRE_EQUAL(c1.size(), c2.size());

        for (auto b1 = std::begin(c1),
                  e1 = std::end  (c1),
                  b2 = std::begin(c2);
             b1 != e1; ++b1, ++b2)
        {
            BOOST_CHECK_CLOSE(*b1, *b2, 1.0e-10);
        }
    }
}

struct UnitCollection
{
    using UPtr = std::unique_ptr<const Opm::ECLUnits::UnitSystem>;

    UPtr metric { ::Opm::ECLUnits::metricUnitConventions() };
    UPtr field  { ::Opm::ECLUnits::fieldUnitConventions()  };
    UPtr lab    { ::Opm::ECLUnits::labUnitConventions()    };
    UPtr pvt_m  { ::Opm::ECLUnits::pvtmUnitConventions()   };
};

// =====================================================================

BOOST_AUTO_TEST_SUITE (Pressure)

BOOST_AUTO_TEST_CASE (Unspecified_Throws_Invalid)
{
    const auto ucoll = UnitCollection{};

    // Case 1: Pressure().to(si) (no .from() -> invalid)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto press = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::
                          Pressure().to(*si).appliedTo(press),
                          std::invalid_argument);
    }

    // Case 2: Pressure().from(field) (no .to() -> invalid)
    {
        auto press = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::Pressure()
                          .from(*ucoll.field).appliedTo(press),
                          std::invalid_argument);

        check_is_close(press, unity());
    }
}

BOOST_AUTO_TEST_CASE (Same_to_same_NoChange)
{
    const auto ucoll = UnitCollection{};

    // SI -> SI (Pascal -> Pascal)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto press = unity();

        ::Opm::ECLUnits::Convert::Pressure()
              .from(*si).to(*si).appliedTo(press);

        check_is_close(press, unity());
    }

    // Metric -> Metric (Bar -> Bar)
    {
        auto press = unity();

        ::Opm::ECLUnits::Convert::Pressure()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(press);

        check_is_close(press, unity());
    }

    // Field -> Field (Psi -> Psi)
    {
        auto press = unity();

        ::Opm::ECLUnits::Convert::Pressure()
              .from(*ucoll.field).to(*ucoll.field).appliedTo(press);

        check_is_close(press, unity());
    }

    // Lab -> Lab (Atm -> Atm)
    {
        auto press = unity();

        ::Opm::ECLUnits::Convert::Pressure()
              .from(*ucoll.lab).to(*ucoll.lab).appliedTo(press);

        check_is_close(press, unity());
    }

    // PVT-M -> PVT-M (Atm -> Atm)
    {
        auto press = unity();

        ::Opm::ECLUnits::Convert::Pressure()
              .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(press);

        check_is_close(press, unity());
    }
}

BOOST_AUTO_TEST_CASE (SI)
{
    const auto ucoll = UnitCollection{};
    const auto si = ::Opm::ECLUnits::internalUnitConventions();

    // SI -> Metric (Pascal -> Bar; /= 1.0e5)
    {
        const auto expect = std::vector<double>{ 1.0e-5 };

        // Case 1: Pressure().from(metric).to(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*si).to(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(si).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.metric).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // SI -> Field (Pascal -> Psi; /= 6.894757293168360e+03)
    {
        const auto expect = std::vector<double>{ 1.450377377302092e-04 };

        // Case 1: Pressure().from(si).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*si).to(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.field).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // SI -> Lab (Pascal -> Atm; /= 101325)
    {
        const auto expect = std::vector<double>{ 9.869232667160128e-06 };

        // Case 1: Pressure().from(si).to(lab)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*si).to(*ucoll.lab).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(lab).from(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.lab).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // SI -> PVT-M (Pascal -> Atm; /= 101325)
    {
        const auto expect = std::vector<double>{ 9.869232667160128e-06 };

        // Case 1: Pressure().from(si).to(pvt_m)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*si).to(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(pvt_m).from(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.pvt_m).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Metric)
{
    const auto ucoll = UnitCollection{};

    // Metric -> SI (Bar -> Pascal; *= 1.0e5)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0e5 };

        // Case 1: Pressure().from(metric).to(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.metric).to(*si).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(si).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*si).from(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Metric -> Field (Bar -> Psi; *= 14.50377377302092)
    {
        const auto expect = std::vector<double>{ 14.50377377302092 };

        // Case 1: Pressure().from(metric).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.metric).to(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.field).from(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Metric -> Lab (Bar -> Atm; /= 1.01325)
    {
        const auto expect = std::vector<double>{ 9.869232667160128e-01 };

        // Case 1: Pressure().from(metric).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.metric).to(*ucoll.lab).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.lab).from(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Metric -> PVT-M (Bar -> Atm; /= 1.01325)
    {
        const auto expect = std::vector<double>{ 9.869232667160128e-01 };

        // Case 1: Pressure().from(metric).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.metric).to(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.pvt_m).from(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Field)
{
    const auto ucoll = UnitCollection{};

    // Field -> SI (Psi -> Pascal; *= 6.894757293168360e+03)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 6.894757293168360e+03 };

        // Case 1: Pressure().from(field).to(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.field).to(*si).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(si).from(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*si).from(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Field -> Metric (Psi -> Bar; /= 14.50377377302092)
    {
        const auto expect = std::vector<double>{ 6.894757293168360e-02 };

        // Case 1: Pressure().from(field).to(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.field).to(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(metric).from(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.metric).from(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Field -> Lab (Psi -> Atm; /= 14.69594877551345)
    {
        const auto expect = std::vector<double>{ 6.804596390987772e-02 };

        // Case 1: Pressure().from(field).to(lab)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.field).to(*ucoll.lab).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(lab).from(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.lab).from(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Field -> PVT-M (Psi -> Atm; /= 14.69594877551345)
    {
        const auto expect = std::vector<double>{ 6.804596390987772e-02 };

        // Case 1: Pressure().from(field).to(pvt_m)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.field).to(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(pvt_m).from(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.pvt_m).from(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Lab)
{
    const auto ucoll = UnitCollection{};

    // Lab -> SI (Atm -> Pascal; *= 1.01325e5)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.01325e5 };

        // Case 1: Pressure().from(lab).to(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.lab).to(*si).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(si).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*si).from(*ucoll.lab).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Lab -> Metric (Atm -> Bar; *= 1.01325)
    {
        const auto expect = std::vector<double>{ 1.01325 };

        // Case 1: Pressure().from(lab).to(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.lab).to(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.metric).from(*ucoll.lab).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Lab -> Field (Atm -> Psi; *= 14.69594877551345)
    {
        const auto expect = std::vector<double>{ 14.69594877551345 };

        // Case 1: Pressure().from(metric).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.lab).to(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(lab)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.field).from(*ucoll.lab).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // Lab -> PVT-M (Atm -> Atm; unchanged)
    {
        // Case 1: Pressure().from(metric).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.lab).to(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, unity());
        }

        // Case 2: Pressure().to(field).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.pvt_m).from(*ucoll.lab).appliedTo(press);

            check_is_close(press, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    const auto ucoll = UnitCollection{};

    // PVT-M -> SI (Atm -> Pascal; *= 1.01325e5)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.01325e5 };

        // Case 1: Pressure().from(pvt_m).to(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.pvt_m).to(*si).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(si).from(pvt_m)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*si).from(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // PVT-M -> Metric (Atm -> Bar; *= 1.01325)
    {
        const auto expect = std::vector<double>{ 1.01325 };

        // Case 1: Pressure().from(pvt_m).to(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.pvt_m).to(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(pvt_m)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.metric).from(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // PVT-M -> Field (Atm -> Psi; *= 14.69594877551345)
    {
        const auto expect = std::vector<double>{ 14.69594877551345 };

        // Case 1: Pressure().from(pvt_m).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.pvt_m).to(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Pressure().to(field).from(pvt_m)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.field).from(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // PVT-M -> Lab (Atm -> Atm; unchanged)
    {
        // Case 1: Pressure().from(pvt_m).to(lab)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, unity());
        }

        // Case 2: Pressure().to(lab).from(pvt_m)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Pressure()
                  .to(*ucoll.pvt_m).from(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, unity());
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (Viscosity)

BOOST_AUTO_TEST_CASE (Unspecified_Throws_Invalid)
{
    const auto ucoll = UnitCollection{};

    // Case 1: Viscosity().to(si) (no .from() -> invalid)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto visc = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::
                          Viscosity().to(*si).appliedTo(visc),
                          std::invalid_argument);
    }

    // Case 2: Viscosity().from(field) (no .to() -> invalid)
    {
        auto visc = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::Viscosity()
                          .from(*ucoll.field).appliedTo(visc),
                          std::invalid_argument);

        check_is_close(visc, unity());
    }
}

BOOST_AUTO_TEST_CASE (Same_to_same_NoChange)
{
    const auto ucoll = UnitCollection{};

    // SI -> SI (Pas -> Pas)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto visc = unity();

        ::Opm::ECLUnits::Convert::Viscosity()
              .from(*si).to(*si).appliedTo(visc);

        check_is_close(visc, unity());
    }

    // Metric -> Metric (cP -> cP)
    {
        auto visc = unity();

        ::Opm::ECLUnits::Convert::Viscosity()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(visc);

        check_is_close(visc, unity());
    }

    // Field -> Field (cP -> cP)
    {
        auto visc = unity();

        ::Opm::ECLUnits::Convert::Viscosity()
              .from(*ucoll.field).to(*ucoll.field).appliedTo(visc);

        check_is_close(visc, unity());
    }

    // Lab -> Lab (cP -> cP)
    {
        auto visc = unity();

        ::Opm::ECLUnits::Convert::Viscosity()
              .from(*ucoll.lab).to(*ucoll.lab).appliedTo(visc);

        check_is_close(visc, unity());
    }

    // PVT-M -> PVT-M (cP -> cP)
    {
        auto visc = unity();

        ::Opm::ECLUnits::Convert::Viscosity()
              .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(visc);

        check_is_close(visc, unity());
    }
}

BOOST_AUTO_TEST_CASE (SI)
{
    const auto ucoll = UnitCollection{};
    const auto si = ::Opm::ECLUnits::internalUnitConventions();

    // SI -> Metric (Pas -> cP; *= 1000)
    {
        const auto expect = std::vector<double>{ 1000.0 };

        // Case 1: Viscosity().from(metric).to(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*si).to(*ucoll.metric).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Viscosity().to(si).from(metric)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.metric).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // SI -> Field (Pas -> cP; *= 1000)
    {
        const auto expect = std::vector<double>{ 1000.0 };

        // Case 1: Viscosity().from(si).to(field)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*si).to(*ucoll.field).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Viscosity().to(field).from(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.field).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // SI -> Lab (Pas -> cP; *= 1000)
    {
        const auto expect = std::vector<double>{ 1000.0 };

        // Case 1: Viscosity().from(si).to(lab)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*si).to(*ucoll.lab).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Viscosity().to(lab).from(si)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.lab).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }

    // SI -> PVT-M (Pas -> cP; *= 1000)
    {
        const auto expect = std::vector<double>{ 1000.0 };

        // Case 1: Viscosity().from(si).to(pvt_m)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*si).to(*ucoll.pvt_m).appliedTo(press);

            check_is_close(press, expect);
        }

        // Case 2: Viscosity().to(pvt_m).from(lab)
        {
            auto press = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.pvt_m).from(*si).appliedTo(press);

            check_is_close(press, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Metric)
{
    const auto ucoll = UnitCollection{};

    // Metric -> SI (cP -> Pas; /= 1000)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0e-3 };

        // Case 1: Viscosity().from(metric).to(si)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.metric).to(*si).appliedTo(visc);

            check_is_close(visc, expect);
        }

        // Case 2: Viscosity().to(si).from(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*si).from(*ucoll.metric).appliedTo(visc);

            check_is_close(visc, expect);
        }
    }

    // Metric -> Field (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(metric).to(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.metric).to(*ucoll.field).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.field).from(*ucoll.metric).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // Metric -> Lab (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(metric).to(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.metric).to(*ucoll.lab).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.lab).from(*ucoll.metric).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // Metric -> PVT-M (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(metric).to(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.metric).to(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.pvt_m).from(*ucoll.metric).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (Field)
{
    const auto ucoll = UnitCollection{};

    // Field -> SI (cP -> Pas; /= 1000)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0e-3 };

        // Case 1: Viscosity().from(field).to(si)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.field).to(*si).appliedTo(visc);

            check_is_close(visc, expect);
        }

        // Case 2: Viscosity().to(si).from(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*si).from(*ucoll.field).appliedTo(visc);

            check_is_close(visc, expect);
        }
    }

    // Field -> Metric (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(field).to(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.field).to(*ucoll.metric).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(metric).from(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.metric).from(*ucoll.field).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // Field -> Lab (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(field).to(lab)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.field).to(*ucoll.lab).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(lab).from(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.lab).from(*ucoll.field).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // Field -> PVT-M (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(field).to(pvt_m)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.field).to(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(pvt_m).from(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.pvt_m).from(*ucoll.field).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (Lab)
{
    const auto ucoll = UnitCollection{};

    // Lab -> SI (cP -> Pas; /= 1000)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0e-3 };

        // Case 1: Viscosity().from(lab).to(si)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.lab).to(*si).appliedTo(visc);

            check_is_close(visc, expect);
        }

        // Case 2: Viscosity().to(si).from(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*si).from(*ucoll.lab).appliedTo(visc);

            check_is_close(visc, expect);
        }
    }

    // Lab -> Metric (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(lab).to(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.lab).to(*ucoll.metric).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.metric).from(*ucoll.lab).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // Lab -> Field (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(metric).to(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.lab).to(*ucoll.field).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(lab)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.field).from(*ucoll.lab).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // Lab -> PVT-M (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(metric).to(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.lab).to(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.pvt_m).from(*ucoll.lab).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    const auto ucoll = UnitCollection{};

    // PVT-M -> SI (cP -> Pas; /= 1000)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0e-3 };

        // Case 1: Viscosity().from(pvt_m).to(si)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.pvt_m).to(*si).appliedTo(visc);

            check_is_close(visc, expect);
        }

        // Case 2: Viscosity().to(si).from(pvt_m)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*si).from(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, expect);
        }
    }

    // PVT-M -> Metric (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(pvt_m).to(metric)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.pvt_m).to(*ucoll.metric).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(pvt_m)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.metric).from(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // PVT-M -> Field (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(pvt_m).to(field)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.pvt_m).to(*ucoll.field).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(field).from(pvt_m)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.field).from(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }

    // PVT-M -> Lab (cP -> cP; Unchanged)
    {
        // Case 1: Viscosity().from(pvt_m).to(lab)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, unity());
        }

        // Case 2: Viscosity().to(lab).from(pvt_m)
        {
            auto visc = unity();

            ::Opm::ECLUnits::Convert::Viscosity()
                  .to(*ucoll.pvt_m).from(*ucoll.pvt_m).appliedTo(visc);

            check_is_close(visc, unity());
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (GasFVF)

BOOST_AUTO_TEST_CASE (Unspecified_Throws_Invalid)
{
    const auto ucoll = UnitCollection{};

    // Case 1: GasFVF().to(si) (no .from() -> invalid)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Bg = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::
                          GasFVF().to(*si).appliedTo(Bg),
                          std::invalid_argument);
    }

    // Case 2: GasFVF().from(field) (no .to() -> invalid)
    {
        auto Bg = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::GasFVF()
                          .from(*ucoll.field).appliedTo(Bg),
                          std::invalid_argument);

        check_is_close(Bg, unity());
    }
}

BOOST_AUTO_TEST_CASE (Same_to_same_NoChange)
{
    const auto ucoll = UnitCollection{};

    // SI -> SI (rm^3/sm^3 -> rm^3/sm^3)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Bg = unity();

        ::Opm::ECLUnits::Convert::GasFVF()
              .from(*si).to(*si).appliedTo(Bg);

        check_is_close(Bg, unity());
    }

    // Metric -> Metric (rm^3/sm^3 -> rm^3/sm^3)
    {
        auto Bg = unity();

        ::Opm::ECLUnits::Convert::GasFVF()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Bg);

        check_is_close(Bg, unity());
    }

    // Field -> Field (rb/Mscf -> rb/Mscf)
    {
        auto Bg = unity();

        ::Opm::ECLUnits::Convert::GasFVF()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Bg);

        check_is_close(Bg, unity());
    }

    // Lab -> Lab (rcm^3/scm^3 -> rcm^3/scm^3)
    {
        auto Bg = unity();

        ::Opm::ECLUnits::Convert::GasFVF()
              .from(*ucoll.lab).to(*ucoll.lab).appliedTo(Bg);

        check_is_close(Bg, unity());
    }

    // PVT-M -> PVT-M (rm^3/sm^3 -> rm^3/sm^3)
    {
        auto Bg = unity();

        ::Opm::ECLUnits::Convert::GasFVF()
              .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Bg);

        check_is_close(Bg, unity());
    }
}

BOOST_AUTO_TEST_CASE (SI)
{
    const auto ucoll = UnitCollection{};
    const auto si = ::Opm::ECLUnits::internalUnitConventions();

    // SI -> Metric (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(si).to(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*si).to(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(metric).from(si)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.metric).from(*si).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // SI -> Field (rm^3/sm^3 -> rb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: GasFVF().from(si).to(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*si).to(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(field).from(si)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.field).from(*si).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // SI -> Lab (rm^3/sm^3 -> rcm^3/scm^3; Unchanged)
    {
        // Case 1: GasFVF().from(si).to(lab)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*si).to(*ucoll.lab).appliedTo(Bg);

            check_is_close(Bg, unity());
        }

        // Case 2: GasFVF().to(lab).from(si)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.lab).from(*si).appliedTo(Bg);

            check_is_close(Bg, unity());
        }
    }

    // SI -> PVT-M (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        // Case 1: GasFVF().from(si).to(pvt_m)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*si).to(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, unity());
        }

        // Case 2: GasFVF().to(pvt_m).from(is)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.pvt_m).from(*si).appliedTo(Bg);

            check_is_close(Bg, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (Metric)
{
    const auto ucoll = UnitCollection{};

    // Metric -> SI (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(metric).to(si)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.metric).to(*si).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(si).from(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*si).from(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Metric -> Field (rm^3/sm^3 -> rb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: GasFVF().from(metric).to(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.metric).to(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(field).from(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.field).from(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Metric -> Lab (rm^3/sm^3 -> rcm^3/scm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(metric).to(lab)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.metric).to(*ucoll.lab).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(lab).from(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.lab).from(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Metric -> PVT-M (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(metric).to(pvt_m)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.metric).to(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(pvt_m).from(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Field)
{
    const auto ucoll = UnitCollection{};

    // Field -> SI (rb/Mscf -> rm^3/sm^3; /= 178.1076066790352)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: GasFVF().from(field).to(si)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.field).to(*si).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(si).from(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*si).from(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Field -> Metric (rb/Mscf -> rm^3/sm^3; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: GasFVF().from(field).to(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.field).to(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(metric).from(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.metric).from(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Field -> Lab (rb/Mscf -> rcm^3/scm^3; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: GasFVF().from(field).to(lab)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.field).to(*ucoll.lab).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(lab).from(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.lab).from(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Field -> PVT-M (rb/Mscf -> rm^3/sm^3; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: GasFVF().from(field).to(pvt_m)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.field).to(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(pvt_m).from(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Lab)
{
    const auto ucoll = UnitCollection{};

    // Lab -> SI (rcm^3/scm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(lab).to(si)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.lab).to(*si).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(si).from(lab)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*si).from(*ucoll.lab).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Lab -> Metric (rcm^3/scm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(lab).to(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.lab).to(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(metric).from(lab)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.metric).from(*ucoll.lab).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Lab -> Field (rcm^3/scm^3 -> rb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: GasFVF().from(lab).to(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.lab).to(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(field).from(lab)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.field).from(*ucoll.lab).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // Lab -> PVT-M (rcm^3/scm^3 -> rm^3/sm^3; unchanged)
    {
        // Case 1: GasFVF().from(metric).to(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.lab).to(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, unity());
        }

        // Case 2: GasFVF().to(field).from(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.lab).appliedTo(Bg);

            check_is_close(Bg, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    const auto ucoll = UnitCollection{};

    // PVT-M -> SI (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(pvt_m).to(si)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.pvt_m).to(*si).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(si).from(pvt_m)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*si).from(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // PVT-M -> Metric (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(pvt_m).to(metric)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.pvt_m).to(*ucoll.metric).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(field).from(pvt_m)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.metric).from(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // PVT-M -> Field (rm^3/sm^3 -> rb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: GasFVF().from(pvt_m).to(field)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.pvt_m).to(*ucoll.field).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(field).from(pvt_m)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.field).from(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }

    // PVT-M -> Lab (rm^3/sm^3 -> rcm^3/scm^3; unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: GasFVF().from(pvt_m).to(lab)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, expect);
        }

        // Case 2: GasFVF().to(lab).from(pvt_m)
        {
            auto Bg = unity();

            ::Opm::ECLUnits::Convert::GasFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.pvt_m).appliedTo(Bg);

            check_is_close(Bg, expect);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (OilFVF)

BOOST_AUTO_TEST_CASE (Unspecified_Throws_Invalid)
{
    const auto ucoll = UnitCollection{};

    // Case 1: OilFVF().to(si) (no .from() -> invalid)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Bo = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::
                          OilFVF().to(*si).appliedTo(Bo),
                          std::invalid_argument);
    }

    // Case 2: OilFVF().from(field) (no .to() -> invalid)
    {
        auto Bo = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::OilFVF()
                          .from(*ucoll.field).appliedTo(Bo),
                          std::invalid_argument);

        check_is_close(Bo, unity());
    }
}

BOOST_AUTO_TEST_CASE (Same_to_same_NoChange)
{
    const auto ucoll = UnitCollection{};

    // SI -> SI (rm^3/sm^3 -> rm^3/sm^3)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Bo = unity();

        ::Opm::ECLUnits::Convert::OilFVF()
              .from(*si).to(*si).appliedTo(Bo);

        check_is_close(Bo, unity());
    }

    // Metric -> Metric (rm^3/sm^3 -> rm^3/sm^3)
    {
        auto Bo = unity();

        ::Opm::ECLUnits::Convert::OilFVF()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Bo);

        check_is_close(Bo, unity());
    }

    // Field -> Field (rb/stb -> rb/stb)
    {
        auto Bo = unity();

        ::Opm::ECLUnits::Convert::OilFVF()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Bo);

        check_is_close(Bo, unity());
    }

    // Lab -> Lab (rcm^3/scm^3 -> rcm^3/scm^3)
    {
        auto Bo = unity();

        ::Opm::ECLUnits::Convert::OilFVF()
              .from(*ucoll.lab).to(*ucoll.lab).appliedTo(Bo);

        check_is_close(Bo, unity());
    }

    // PVT-M -> PVT-M (rm^3/sm^3 -> rm^3/sm^3)
    {
        auto Bo = unity();

        ::Opm::ECLUnits::Convert::OilFVF()
              .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Bo);

        check_is_close(Bo, unity());
    }
}

BOOST_AUTO_TEST_CASE (SI)
{
    const auto ucoll = UnitCollection{};
    const auto si = ::Opm::ECLUnits::internalUnitConventions();

    // SI -> Metric (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(si).to(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*si).to(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(metric).from(si)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.metric).from(*si).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // SI -> Field (rm^3/sm^3 -> rb/stb; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(si).to(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*si).to(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(field).from(si)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.field).from(*si).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // SI -> Lab (rm^3/sm^3 -> rcm^3/scm^3; Unchanged)
    {
        // Case 1: OilFVF().from(si).to(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*si).to(*ucoll.lab).appliedTo(Bo);

            check_is_close(Bo, unity());
        }

        // Case 2: OilFVF().to(lab).from(si)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.lab).from(*si).appliedTo(Bo);

            check_is_close(Bo, unity());
        }
    }

    // SI -> PVT-M (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        // Case 1: OilFVF().from(si).to(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*si).to(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, unity());
        }

        // Case 2: OilFVF().to(pvt_m).from(is)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.pvt_m).from(*si).appliedTo(Bo);

            check_is_close(Bo, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (Metric)
{
    const auto ucoll = UnitCollection{};

    // Metric -> SI (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(metric).to(si)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.metric).to(*si).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(si).from(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*si).from(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Metric -> Field (rm^3/sm^3 -> rb/stb; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(metric).to(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.metric).to(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(field).from(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.field).from(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Metric -> Lab (rm^3/sm^3 -> rcm^3/scm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(metric).to(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.metric).to(*ucoll.lab).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(lab).from(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.lab).from(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Metric -> PVT-M (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(metric).to(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.metric).to(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(pvt_m).from(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Field)
{
    const auto ucoll = UnitCollection{};

    // Field -> SI (rb/stb -> rm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(field).to(si)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.field).to(*si).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(si).from(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*si).from(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Field -> Metric (rb/stb -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(field).to(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.field).to(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(metric).from(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.metric).from(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Field -> Lab (rb/stb -> rcm^3/scm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(field).to(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.field).to(*ucoll.lab).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(lab).from(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.lab).from(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Field -> PVT-M (rb/stb -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(field).to(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.field).to(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(pvt_m).from(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Lab)
{
    const auto ucoll = UnitCollection{};

    // Lab -> SI (rcm^3/scm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(lab).to(si)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.lab).to(*si).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(si).from(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*si).from(*ucoll.lab).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Lab -> Metric (rcm^3/scm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(lab).to(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.lab).to(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(metric).from(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.metric).from(*ucoll.lab).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Lab -> Field (rcm^3/scm^3 -> rb/stb; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(lab).to(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.lab).to(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(field).from(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.field).from(*ucoll.lab).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // Lab -> PVT-M (rcm^3/scm^3 -> rm^3/sm^3; unchanged)
    {
        // Case 1: OilFVF().from(lab).to(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.lab).to(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, unity());
        }

        // Case 2: OilFVF().to(pvt_m).from(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.lab).appliedTo(Bo);

            check_is_close(Bo, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    const auto ucoll = UnitCollection{};

    // PVT-M -> SI (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(pvt_m).to(si)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.pvt_m).to(*si).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(si).from(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*si).from(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // PVT-M -> Metric (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(pvt_m).to(metric)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.pvt_m).to(*ucoll.metric).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(field).from(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.metric).from(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // PVT-M -> Field (rm^3/sm^3 -> rb/stb; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(pvt_m).to(field)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.pvt_m).to(*ucoll.field).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(field).from(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.field).from(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }

    // PVT-M -> Lab (rm^3/sm^3 -> rcm^3/scm^3; unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: OilFVF().from(pvt_m).to(lab)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, expect);
        }

        // Case 2: OilFVF().to(lab).from(pvt_m)
        {
            auto Bo = unity();

            ::Opm::ECLUnits::Convert::OilFVF()
                  .to(*ucoll.pvt_m).from(*ucoll.pvt_m).appliedTo(Bo);

            check_is_close(Bo, expect);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (DissolvedGasOilRatio)

BOOST_AUTO_TEST_CASE (Unspecified_Throws_Invalid)
{
    const auto ucoll = UnitCollection{};

    // Case 1: DissolvedGasOilRatio().to(si) (no .from() -> invalid)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Rs = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::
                          DissolvedGasOilRatio().to(*si).appliedTo(Rs),
                          std::invalid_argument);
    }

    // Case 2: DissolvedGasOilRatio().from(field) (no .to() -> invalid)
    {
        auto Rs = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                          .from(*ucoll.field).appliedTo(Rs),
                          std::invalid_argument);

        check_is_close(Rs, unity());
    }
}

BOOST_AUTO_TEST_CASE (Same_to_same_NoChange)
{
    const auto ucoll = UnitCollection{};

    // SI -> SI (sm^3/sm^3 -> sm^3/sm^3)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Rs = unity();

        ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
              .from(*si).to(*si).appliedTo(Rs);

        check_is_close(Rs, unity());
    }

    // Metric -> Metric (sm^3/sm^3 -> sm^3/sm^3)
    {
        auto Rs = unity();

        ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Rs);

        check_is_close(Rs, unity());
    }

    // Field -> Field (Mscf/stb -> Mscf/stb)
    {
        auto Rs = unity();

        ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Rs);

        check_is_close(Rs, unity());
    }

    // Lab -> Lab (scm^3/scm^3 -> scm^3/scm^3)
    {
        auto Rs = unity();

        ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
              .from(*ucoll.lab).to(*ucoll.lab).appliedTo(Rs);

        check_is_close(Rs, unity());
    }

    // PVT-M -> PVT-M (sm^3/sm^3 -> sm^3/sm^3)
    {
        auto Rs = unity();

        ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
              .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Rs);

        check_is_close(Rs, unity());
    }
}

BOOST_AUTO_TEST_CASE (SI)
{
    const auto ucoll = UnitCollection{};
    const auto si = ::Opm::ECLUnits::internalUnitConventions();

    // SI -> Metric (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(si).to(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*si).to(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(metric).from(si)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.metric).from(*si).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // SI -> Field (sm^3/sm^3 -> Mscf/stb; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: DissolvedGasOilRatio().from(si).to(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*si).to(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(field).from(si)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.field).from(*si).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // SI -> Lab (sm^3/sm^3 -> scm^3/scm^3; Unchanged)
    {
        // Case 1: DissolvedGasOilRatio().from(si).to(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*si).to(*ucoll.lab).appliedTo(Rs);

            check_is_close(Rs, unity());
        }

        // Case 2: DissolvedGasOilRatio().to(lab).from(si)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.lab).from(*si).appliedTo(Rs);

            check_is_close(Rs, unity());
        }
    }

    // SI -> PVT-M (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        // Case 1: DissolvedGasOilRatio().from(si).to(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*si).to(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, unity());
        }

        // Case 2: DissolvedGasOilRatio().to(pvt_m).from(is)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.pvt_m).from(*si).appliedTo(Rs);

            check_is_close(Rs, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (Metric)
{
    const auto ucoll = UnitCollection{};

    // Metric -> SI (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(metric).to(si)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.metric).to(*si).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(si).from(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*si).from(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Metric -> Field (sm^3/sm^3 -> Mscf/stb; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: DissolvedGasOilRatio().from(metric).to(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.metric).to(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(field).from(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.field).from(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Metric -> Lab (sm^3/sm^3 -> scm^3/scm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(metric).to(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.metric).to(*ucoll.lab).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(lab).from(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.lab).from(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Metric -> PVT-M (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(metric).to(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.metric).to(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(pvt_m).from(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Field)
{
    const auto ucoll = UnitCollection{};

    // Field -> SI (Mscf/stb -> sm^3/sm^3; *= 178.1076066790352)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: DissolvedGasOilRatio().from(field).to(si)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.field).to(*si).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(si).from(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*si).from(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Field -> Metric (Mscf/stb -> sm^3/sm^3; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: DissolvedGasOilRatio().from(field).to(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.field).to(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(metric).from(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.metric).from(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Field -> Lab (Mscf/stb -> scm^3/scm^3; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: DissolvedGasOilRatio().from(field).to(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.field).to(*ucoll.lab).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(lab).from(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.lab).from(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Field -> PVT-M (Mscf/stb -> sm^3/sm^3; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: DissolvedGasOilRatio().from(field).to(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.field).to(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(pvt_m).from(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Lab)
{
    const auto ucoll = UnitCollection{};

    // Lab -> SI (scm^3/scm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(lab).to(si)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.lab).to(*si).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(si).from(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*si).from(*ucoll.lab).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Lab -> Metric (scm^3/scm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(lab).to(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.lab).to(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(metric).from(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.metric).from(*ucoll.lab).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Lab -> Field (scm^3/scm^3 -> Mscf/stb; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: DissolvedGasOilRatio().from(lab).to(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.lab).to(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(field).from(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.field).from(*ucoll.lab).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // Lab -> PVT-M (scm^3/scm^3 -> sm^3/sm^3; Unchanged)
    {
        // Case 1: DissolvedGasOilRatio().from(lab).to(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.lab).to(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, unity());
        }

        // Case 2: DissolvedGasOilRatio().to(pvt_m).from(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.lab).appliedTo(Rs);

            check_is_close(Rs, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    const auto ucoll = UnitCollection{};

    // PVT-M -> SI (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(pvt_m).to(si)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.pvt_m).to(*si).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(si).from(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*si).from(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // PVT-M -> Metric (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(pvt_m).to(metric)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.pvt_m).to(*ucoll.metric).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(field).from(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.metric).from(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // PVT-M -> Field (sm^3/sm^3 -> Mscf/stb; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: DissolvedGasOilRatio().from(pvt_m).to(field)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.pvt_m).to(*ucoll.field).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(field).from(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.field).from(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }

    // PVT-M -> Lab (sm^3/sm^3 -> scm^3/scm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: DissolvedGasOilRatio().from(pvt_m).to(lab)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, expect);
        }

        // Case 2: DissolvedGasOilRatio().to(lab).from(pvt_m)
        {
            auto Rs = unity();

            ::Opm::ECLUnits::Convert::DissolvedGasOilRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.pvt_m).appliedTo(Rs);

            check_is_close(Rs, expect);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (VaporisedOilGasRatio)

BOOST_AUTO_TEST_CASE (Unspecified_Throws_Invalid)
{
    const auto ucoll = UnitCollection{};

    // Case 1: VaporisedOilGasRatio().to(si) (no .from() -> invalid)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Rv = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::
                          VaporisedOilGasRatio().to(*si).appliedTo(Rv),
                          std::invalid_argument);
    }

    // Case 2: VaporisedOilGasRatio().from(field) (no .to() -> invalid)
    {
        auto Rv = unity();

        BOOST_CHECK_THROW(::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                          .from(*ucoll.field).appliedTo(Rv),
                          std::invalid_argument);

        check_is_close(Rv, unity());
    }
}

BOOST_AUTO_TEST_CASE (Same_to_same_NoChange)
{
    const auto ucoll = UnitCollection{};

    // SI -> SI (sm^3/sm^3 -> sm^3/sm^3)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        auto Rv = unity();

        ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
              .from(*si).to(*si).appliedTo(Rv);

        check_is_close(Rv, unity());
    }

    // Metric -> Metric (sm^3/sm^3 -> sm^3/sm^3)
    {
        auto Rv = unity();

        ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Rv);

        check_is_close(Rv, unity());
    }

    // Field -> Field (stb/Mscf -> stb/Mscf)
    {
        auto Rv = unity();

        ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
              .from(*ucoll.metric).to(*ucoll.metric).appliedTo(Rv);

        check_is_close(Rv, unity());
    }

    // Lab -> Lab (scm^3/scm^3 -> scm^3/scm^3)
    {
        auto Rv = unity();

        ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
              .from(*ucoll.lab).to(*ucoll.lab).appliedTo(Rv);

        check_is_close(Rv, unity());
    }

    // PVT-M -> PVT-M (sm^3/sm^3 -> sm^3/sm^3)
    {
        auto Rv = unity();

        ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
              .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Rv);

        check_is_close(Rv, unity());
    }
}

BOOST_AUTO_TEST_CASE (SI)
{
    const auto ucoll = UnitCollection{};
    const auto si = ::Opm::ECLUnits::internalUnitConventions();

    // SI -> Metric (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(si).to(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*si).to(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(metric).from(si)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.metric).from(*si).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // SI -> Field (sm^3/sm^3 -> stb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: VaporisedOilGasRatio().from(si).to(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*si).to(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(field).from(si)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.field).from(*si).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // SI -> Lab (sm^3/sm^3 -> scm^3/scm^3; Unchanged)
    {
        // Case 1: VaporisedOilGasRatio().from(si).to(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*si).to(*ucoll.lab).appliedTo(Rv);

            check_is_close(Rv, unity());
        }

        // Case 2: VaporisedOilGasRatio().to(lab).from(si)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.lab).from(*si).appliedTo(Rv);

            check_is_close(Rv, unity());
        }
    }

    // SI -> PVT-M (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        // Case 1: VaporisedOilGasRatio().from(si).to(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*si).to(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, unity());
        }

        // Case 2: VaporisedOilGasRatio().to(pvt_m).from(is)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.pvt_m).from(*si).appliedTo(Rv);

            check_is_close(Rv, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (Metric)
{
    const auto ucoll = UnitCollection{};

    // Metric -> SI (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(metric).to(si)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.metric).to(*si).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(si).from(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*si).from(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Metric -> Field (sm^3/sm^3 -> stb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: VaporisedOilGasRatio().from(metric).to(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.metric).to(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(field).from(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.field).from(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Metric -> Lab (sm^3/sm^3 -> scm^3/scm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(metric).to(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.metric).to(*ucoll.lab).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(lab).from(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.lab).from(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Metric -> PVT-M (rm^3/sm^3 -> rm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(metric).to(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.metric).to(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(pvt_m).from(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Field)
{
    const auto ucoll = UnitCollection{};

    // Field -> SI (stb/Mscf -> sm^3/sm^3; /= 178.1076066790352)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: VaporisedOilGasRatio().from(field).to(si)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.field).to(*si).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(si).from(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*si).from(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Field -> Metric (stb/Mscf -> sm^3/sm^3; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: VaporisedOilGasRatio().from(field).to(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.field).to(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(metric).from(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.metric).from(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Field -> Lab (stb/Mscf -> scm^3/scm^3; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: VaporisedOilGasRatio().from(field).to(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.field).to(*ucoll.lab).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(lab).from(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.lab).from(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Field -> PVT-M (stb/Mscf -> sm^3/sm^3; /= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 5.614583333333335e-03 };

        // Case 1: VaporisedOilGasRatio().from(field).to(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.field).to(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(pvt_m).from(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }
}

BOOST_AUTO_TEST_CASE (Lab)
{
    const auto ucoll = UnitCollection{};

    // Lab -> SI (scm^3/scm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(lab).to(si)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.lab).to(*si).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(si).from(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*si).from(*ucoll.lab).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Lab -> Metric (scm^3/scm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(lab).to(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.lab).to(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(metric).from(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.metric).from(*ucoll.lab).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Lab -> Field (scm^3/scm^3 -> stb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: VaporisedOilGasRatio().from(lab).to(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.lab).to(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(field).from(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.field).from(*ucoll.lab).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // Lab -> PVT-M (scm^3/scm^3 -> sm^3/sm^3; Unchanged)
    {
        // Case 1: VaporisedOilGasRatio().from(lab).to(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.lab).to(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, unity());
        }

        // Case 2: VaporisedOilGasRatio().to(pvt_m).from(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.lab).appliedTo(Rv);

            check_is_close(Rv, unity());
        }
    }
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    const auto ucoll = UnitCollection{};

    // PVT-M -> SI (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto si = ::Opm::ECLUnits::internalUnitConventions();

        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(pvt_m).to(si)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.pvt_m).to(*si).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(si).from(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*si).from(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // PVT-M -> Metric (sm^3/sm^3 -> sm^3/sm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(pvt_m).to(metric)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.pvt_m).to(*ucoll.metric).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(field).from(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.metric).from(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // PVT-M -> Field (sm^3/sm^3 -> stb/Mscf; *= 178.1076066790352)
    {
        const auto expect = std::vector<double>{ 178.1076066790352 };

        // Case 1: VaporisedOilGasRatio().from(pvt_m).to(field)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.pvt_m).to(*ucoll.field).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(field).from(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.field).from(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }

    // PVT-M -> Lab (sm^3/sm^3 -> scm^3/scm^3; Unchanged)
    {
        const auto expect = std::vector<double>{ 1.0 };

        // Case 1: VaporisedOilGasRatio().from(pvt_m).to(lab)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .from(*ucoll.pvt_m).to(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, expect);
        }

        // Case 2: VaporisedOilGasRatio().to(lab).from(pvt_m)
        {
            auto Rv = unity();

            ::Opm::ECLUnits::Convert::VaporisedOilGasRatio()
                  .to(*ucoll.pvt_m).from(*ucoll.pvt_m).appliedTo(Rv);

            check_is_close(Rv, expect);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END ()
