/*
  Copyright 2019 Equinor
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

#define BOOST_TEST_MODULE LogiHEAD_Vector

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/LogiHEAD.hpp>

#include <opm/output/eclipse/VectorItems/logihead.hpp>

BOOST_AUTO_TEST_SUITE(Member_Functions)

BOOST_AUTO_TEST_CASE(Radial_Settings_and_Init)
{
    const auto e300_radial = false;
    const auto e100_radial = true;
    const auto enableHyster = true;

    const auto lh = Opm::RestartIO::LogiHEAD{}
        .variousParam(e300_radial, e100_radial, 4, enableHyster);

    const auto& v = lh.data();

    BOOST_CHECK_EQUAL(v[  3], false); // E300 Radial
    BOOST_CHECK_EQUAL(v[  4], true);  // E100 Radial
    BOOST_CHECK_EQUAL(v[  6], true);  // enableHyster
    BOOST_CHECK_EQUAL(v[ 75], true);  // MS Well Simulation Case
}

BOOST_AUTO_TEST_CASE(PVTModel)
{
    namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

    // Defaulted => dead oil, dry gas, non-constant Co.
    {
        auto pvt = ::Opm::RestartIO::LogiHEAD::PVTModel{};

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.pvtModel(pvt);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::IsLiveOil ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::IsWetGas  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::ConstCo   ], false);
    }

    // Live oil, others defaulted
    {
        auto pvt = ::Opm::RestartIO::LogiHEAD::PVTModel{};

        pvt.isLiveOil = true;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.pvtModel(pvt);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::IsLiveOil ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::IsWetGas  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::ConstCo   ], false);
    }

    // Wet gas, others defaulted
    {
        auto pvt = ::Opm::RestartIO::LogiHEAD::PVTModel{};

        pvt.isWetGas = true;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.pvtModel(pvt);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::IsLiveOil ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::IsWetGas  ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::ConstCo   ], false);
    }

    // Constant oil compressibility, others defaulted
    {
        auto pvt = ::Opm::RestartIO::LogiHEAD::PVTModel{};

        pvt.constComprOil = true;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.pvtModel(pvt);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::IsLiveOil ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::IsWetGas  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::ConstCo   ], true);
    }
}

BOOST_AUTO_TEST_CASE(SaturationFunction)
{
    namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

    // Defaulted => non-directional and reversible Kr, no EPS but
    // reversible if actually enabled.
    {
        auto sfunc = ::Opm::RestartIO::LogiHEAD::SatfuncFlags{};

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.saturationFunction(sfunc);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::DirKr     ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::E100RevKr ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::EndScale  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::DirEPS    ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::RevEPS    ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::AltEPS    ], false);
    }

    // Directionally dependent relative permeability, all other defaulted.
    {
        auto sfunc = ::Opm::RestartIO::LogiHEAD::SatfuncFlags {};

        sfunc.useDirectionalRelPerm = true;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.saturationFunction(sfunc);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::DirKr     ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::E100RevKr ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::EndScale  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::DirEPS    ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::RevEPS    ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::AltEPS    ], false);
    }

    // Irreversible relative permeability, all other defaulted.
    {
        auto sfunc = ::Opm::RestartIO::LogiHEAD::SatfuncFlags {};

        sfunc.useReversibleRelPerm = false;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.saturationFunction(sfunc);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::DirKr     ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::E100RevKr ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::EndScale  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::DirEPS    ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::RevEPS    ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::AltEPS    ], false);
    }

    // End-point scaling activated, all other defaulted.
    {
        auto sfunc = ::Opm::RestartIO::LogiHEAD::SatfuncFlags {};

        sfunc.useEndScale = true;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.saturationFunction(sfunc);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::DirKr     ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::E100RevKr ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::EndScale  ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::DirEPS    ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::RevEPS    ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::AltEPS    ], false);
    }

    // Directionally dependent end-point scaling, all other defaulted.
    {
        auto sfunc = ::Opm::RestartIO::LogiHEAD::SatfuncFlags {};

        sfunc.useDirectionalEPS = true;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.saturationFunction(sfunc);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::DirKr     ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::E100RevKr ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::EndScale  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::DirEPS    ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::RevEPS    ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::AltEPS    ], false);
    }

    // Irreversible end-point scaling, all other defaulted.
    {
        auto sfunc = ::Opm::RestartIO::LogiHEAD::SatfuncFlags {};

        sfunc.useReversibleEPS = false;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.saturationFunction(sfunc);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::DirKr     ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::E100RevKr ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::EndScale  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::DirEPS    ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::RevEPS    ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::AltEPS    ], false);
    }

    // Alternative end-point scaling option (three-point method), all other
    // defaulted.
    {
        auto sfunc = ::Opm::RestartIO::LogiHEAD::SatfuncFlags {};

        sfunc.useAlternateEPS = true;

        const auto lh =
            ::Opm::RestartIO::LogiHEAD{}.saturationFunction(sfunc);

        const auto& v = lh.data();

        BOOST_CHECK_EQUAL(v[ VI::logihead::DirKr     ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::E100RevKr ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::EndScale  ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::DirEPS    ], false);
        BOOST_CHECK_EQUAL(v[ VI::logihead::RevEPS    ], true);
        BOOST_CHECK_EQUAL(v[ VI::logihead::AltEPS    ], true);
    }
}

BOOST_AUTO_TEST_SUITE_END()
