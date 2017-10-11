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

#define BOOST_TEST_MODULE TEST_ECLPVTCOMMON_UNITCONV

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/utility/ECLPvtCommon.hpp>

#include <opm/utility/ECLUnitHandling.hpp>

#include <exception>
#include <stdexcept>

struct ConvertToSI
{
    explicit ConvertToSI(const ::Opm::ECLUnits::UnitSystem& usys);

    double dens   { 0.0 };
    double press  { 0.0 };
    double compr  { 0.0 };
    double disgas { 0.0 };
    double vapoil { 0.0 };

    double recipFvf            { 0.0 };
    double recipFvfDerivPress  { 0.0 };
    double recipFvfDerivVapOil { 0.0 };

    double recipFvfVisc            { 0.0 };
    double recipFvfViscDerivPress  { 0.0 };
    double recipFvfViscDerivVapOil { 0.0 };

    double recipFvfGas            { 0.0 };
    double recipFvfGasDerivPress  { 0.0 };
    double recipFvfGasDerivVapOil { 0.0 };

    double recipFvfGasVisc            { 0.0 };
    double recipFvfGasViscDerivPress  { 0.0 };
    double recipFvfGasViscDerivVapOil { 0.0 };
};

ConvertToSI::ConvertToSI(const ::Opm::ECLUnits::UnitSystem& usys)
{
    using Cvrt = ::Opm::ECLPVT::CreateUnitConverter::ToSI;

    auto apply = [](const ::Opm::ECLPVT::ConvertUnits::Converter& cnv)
    {
        return cnv(1.0);
    };

    // Mass density
    this->dens = apply(Cvrt::density(usys));

    // Pressure
    this->press = apply(Cvrt::pressure(usys));

    // Compressibility
    this->compr = apply(Cvrt::compressibility(usys));

    // Dissolved gas-oil ratio (Rs)
    this->disgas = apply(Cvrt::disGas(usys));

    // Vaporised oil-gas ratio (Rv)
    this->vapoil = apply(Cvrt::vapOil(usys));

    // Reciprocal formation volume factor (1/B)
    this->recipFvf = apply(Cvrt::recipFvf(usys));

    // Derivative of reciprocal formation volume factor (1/B) with respect
    // to fluid (phase) pressure.
    this->recipFvfDerivPress =
        apply(Cvrt::recipFvfDerivPress(usys));

    // Derivative of reciprocal formation volume factor (1/B) with respect
    // to vaporised oil-gas ratio.
    this->recipFvfDerivVapOil =
        apply(Cvrt::recipFvfDerivVapOil(usys));

    // Reciprocal product of formation volume factor and viscosity
    // (1/(B*mu)).
    this->recipFvfVisc = apply(Cvrt::recipFvfVisc(usys));

    // Derivative of reciprocal product of formation volume factor and
    // viscosity (1/(B*mu)) with respect to fluid (phase) pressure.
    this->recipFvfViscDerivPress =
        apply(Cvrt::recipFvfViscDerivPress(usys));

    // Derivative of reciprocal product of formation volume factor and
    // viscosity (1/(B*mu)) with respect to vaporised oil-gas ratio.
    this->recipFvfViscDerivVapOil =
        apply(Cvrt::recipFvfViscDerivVapOil(usys));

    // Reciprocal formation volume factor for gas (1/Bg)
    this->recipFvfGas = apply(Cvrt::recipFvfGas(usys));

    // Derivative of reciprocal formation volume factor for gas (1/Bg) with
    // respect to fluid (phase) pressure.
    this->recipFvfGasDerivPress =
        apply(Cvrt::recipFvfGasDerivPress(usys));

    // Derivative of reciprocal formation volume factor for gas (1/Bg) with
    // respect to vaporised oil-gas ratio.
    this->recipFvfGasDerivVapOil =
        apply(Cvrt::recipFvfGasDerivVapOil(usys));

    // Reciprocal product of formation volume factor for gas and viscosity
    // (1/(Bg*mu_g)).
    this->recipFvfGasVisc = apply(Cvrt::recipFvfGasVisc(usys));

    // Derivative of reciprocal product of formation volume factor for gas
    // and viscosity (1/(Bg*mu_g)) with respect to fluid (phase) pressure.
    this->recipFvfGasViscDerivPress =
        apply(Cvrt::recipFvfGasViscDerivPress(usys));

    // Derivative of reciprocal product of formation volume factor for gas
    // and viscosity (1/(Bg*mu_g)) with respect to vaporised oil-gas ratio.
    this->recipFvfGasViscDerivVapOil =
        apply(Cvrt::recipFvfGasViscDerivVapOil(usys));
}

template <std::size_t N>
using DVec = ::Opm::ECLPVT::DenseVector<N>;

// =====================================================================

BOOST_AUTO_TEST_SUITE (Basic_Conversion)

BOOST_AUTO_TEST_CASE (Metric)
{
    const auto usys = ::Opm::ECLUnits::createUnitSystem(1);

    const auto scale = ConvertToSI(*usys);

    // Mass density
    BOOST_CHECK_CLOSE(scale.dens, 1.0, 1.0e-10);

    // Pressure
    BOOST_CHECK_CLOSE(scale.press, 1.0e5, 1.0e-10);

    // Compressibility
    BOOST_CHECK_CLOSE(scale.compr, 1.0e-5, 1.0e-10);

    // Dissolved Gas-Oil Ratio (Rs)
    BOOST_CHECK_CLOSE(scale.disgas, 1.0, 1.0e-10);

    // Vaporised Oil-Gas Ratio (Rv)
    BOOST_CHECK_CLOSE(scale.vapoil, 1.0, 1.0e-10);

    // Reciprocal Formation Volume Factor (1 / B)
    BOOST_CHECK_CLOSE(scale.recipFvf, 1.0, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfDerivPress, 1.0e-5, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfDerivVapOil, 1.0, 1.0e-10);

    // Reciprocal Product of FVF and Viscosity (1 / (B*mu)).
    BOOST_CHECK_CLOSE(scale.recipFvfVisc, 1.0e3, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivPress, 1.0e-2, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivVapOil, 1.0e3, 1.0e-10);

    // Reciprocal Formation Volume Factor for Gas (1 / Bg)
    BOOST_CHECK_CLOSE(scale.recipFvfGas, 1.0, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivPress, 1.0e-5, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg ) w.r.t. Vaporised
    // Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivVapOil, 1.0, 1.0e-10);

    // Reciprocal Product of FVF for Gas and Viscosity (1 / (Bg*mu_g)).
    BOOST_CHECK_CLOSE(scale.recipFvfGasVisc, 1.0e3, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivPress, 1.0e-2, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivVapOil, 1.0e3, 1.0e-10);
}

BOOST_AUTO_TEST_CASE (Field)
{
    const auto usys = ::Opm::ECLUnits::createUnitSystem(2);

    const auto scale = ConvertToSI(*usys);

    // Mass density
    BOOST_CHECK_CLOSE(scale.dens, 1.601846337396014e+01, 1.0e-10);

    // Pressure
    BOOST_CHECK_CLOSE(scale.press, 6.894757293168360e+03, 1.0e-10);

    // Compressibility
    BOOST_CHECK_CLOSE(scale.compr, 1.450377377302092e-04, 1.0e-10);

    // Dissolved Gas-Oil Ratio (Rs)
    BOOST_CHECK_CLOSE(scale.disgas, 1.781076066790352e+02, 1.0e-10);

    // Vaporised Oil-Gas Ratio (Rv)
    BOOST_CHECK_CLOSE(scale.vapoil, 5.614583333333335e-03, 1.0e-10);

    // Reciprocal Formation Volume Factor (1 / B)
    BOOST_CHECK_CLOSE(scale.recipFvf, 1.0, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfDerivPress,
                      1.450377377302092e-04, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfDerivVapOil,
                      1.781076066790352e+02, 1.0e-10);

    // Reciprocal Product of FVF and Viscosity (1 / (B*mu)).
    BOOST_CHECK_CLOSE(scale.recipFvfVisc, 1.0e3, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivPress,
                      1.450377377302093e-01, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivVapOil,
                      1.781076066790352e+05, 1.0e-10);

    // Reciprocal Formation Volume Factor for Gas (1 / Bg)
    BOOST_CHECK_CLOSE(scale.recipFvfGas,
                      1.781076066790352e+02, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivPress,
                      2.583232434526917e-02, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg ) w.r.t. Vaporised
    // Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivVapOil,
                      3.172231955693390e+04, 1.0e-10);

    // Reciprocal Product of FVF for Gas and Viscosity (1 / (Bg*mu_g)).
    BOOST_CHECK_CLOSE(scale.recipFvfGasVisc,
                      1.781076066790352e+05, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivPress,
                      2.583232434526917e+01, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivVapOil,
                      3.172231955693390e+07, 1.0e-10);
}

BOOST_AUTO_TEST_CASE (Lab)
{
    const auto usys = ::Opm::ECLUnits::createUnitSystem(3);

    const auto scale = ConvertToSI(*usys);

    // Mass density
    BOOST_CHECK_CLOSE(scale.dens, 1.0e3, 1.0e-10);

    // Pressure
    BOOST_CHECK_CLOSE(scale.press, 101.325e3, 1.0e-10);

    // Compressibility
    BOOST_CHECK_CLOSE(scale.compr, 9.869232667160129e-06, 1.0e-10);

    // Dissolved Gas-Oil Ratio (Rs)
    BOOST_CHECK_CLOSE(scale.disgas, 1.0, 1.0e-10);

    // Vaporised Oil-Gas Ratio (Rv)
    BOOST_CHECK_CLOSE(scale.vapoil, 1.0, 1.0e-10);

    // Reciprocal Formation Volume Factor (1 / B)
    BOOST_CHECK_CLOSE(scale.recipFvf, 1.0, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfDerivPress,
                      9.869232667160129e-06, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfDerivVapOil,
                      1.0, 1.0e-10);

    // Reciprocal Product of FVF and Viscosity (1 / (B*mu)).
    BOOST_CHECK_CLOSE(scale.recipFvfVisc, 1.0e3, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivPress,
                      9.869232667160128e-03, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivVapOil,
                      1.0e3, 1.0e-10);

    // Reciprocal Formation Volume Factor for Gas (1 / Bg)
    BOOST_CHECK_CLOSE(scale.recipFvfGas, 1.0, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivPress,
                      9.869232667160129e-06, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg) w.r.t. Vaporised
    // Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivVapOil, 1.0, 1.0e-10);

    // Reciprocal Product of FVF for Gas and Viscosity (1 / (Bg*mu_g)).
    BOOST_CHECK_CLOSE(scale.recipFvfGasVisc, 1.0e3, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivPress,
                      9.869232667160128e-03, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivVapOil, 1.0e3, 1.0e-10);
}

BOOST_AUTO_TEST_CASE (PVT_M)
{
    const auto usys = ::Opm::ECLUnits::createUnitSystem(4);

    const auto scale = ConvertToSI(*usys);

    // Mass density
    BOOST_CHECK_CLOSE(scale.dens, 1.0, 1.0e-10);

    // Pressure
    BOOST_CHECK_CLOSE(scale.press, 101.325e3, 1.0e-10);

    // Compressibility
    BOOST_CHECK_CLOSE(scale.compr, 9.869232667160129e-06, 1.0e-10);

    // Dissolved Gas-Oil Ratio (Rs)
    BOOST_CHECK_CLOSE(scale.disgas, 1.0, 1.0e-10);

    // Vaporised Oil-Gas Ratio (Rv)
    BOOST_CHECK_CLOSE(scale.vapoil, 1.0, 1.0e-10);

    // Reciprocal Formation Volume Factor (1 / B)
    BOOST_CHECK_CLOSE(scale.recipFvf, 1.0, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfDerivPress,
                      9.869232667160129e-06, 1.0e-10);

    // Derivative of Reciprocal FVF (1 / B) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfDerivVapOil, 1.0, 1.0e-10);

    // Reciprocal Product of FVF and Viscosity (1 / (B*mu)).
    BOOST_CHECK_CLOSE(scale.recipFvfVisc, 1.0e3, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivPress,
                      9.869232667160128e-03, 1.0e-10);

    // Derivative of Reciprocal Product of FVF and Viscosity (1 / (B*mu))
    // w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfViscDerivVapOil, 1.0e3, 1.0e-10);

    // Reciprocal Formation Volume Factor for Gas (1 / Bg)
    BOOST_CHECK_CLOSE(scale.recipFvfGas, 1.0, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivPress,
                      9.869232667160129e-06, 1.0e-10);

    // Derivative of Reciprocal FVF for Gas (1 / Bg ) w.r.t. Vaporised
    // Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasDerivVapOil, 1.0, 1.0e-10);

    // Reciprocal Product of FVF for Gas and Viscosity (1 / (Bg*mu_g)).
    BOOST_CHECK_CLOSE(scale.recipFvfGasVisc, 1.0e3, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Pressure
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivPress,
                      9.869232667160128e-03, 1.0e-10);

    // Derivative of Reciprocal Product of FVF for Gas and Viscosity (1 /
    // (Bg*mu_g)) w.r.t. Vaporised Oil-Gas Ratio.
    BOOST_CHECK_CLOSE(scale.recipFvfGasViscDerivVapOil, 1.0e3, 1.0e-10);
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (DenseVector)

BOOST_AUTO_TEST_CASE (Construct)
{
    // DenseVector<1>
    {
        const auto x = DVec<1>{ std::array<double, 1>{ { 1.0 } } };

        BOOST_CHECK_CLOSE(x.array()[0], 1.0, 1.0e-10);
    }

    // DenseVector<2>
    {
        const auto x = DVec<2>{ std::array<double, 2>{ { 2.0, -1.0 } } };

        BOOST_CHECK_CLOSE(x.array()[0],  2.0, 1.0e-10);
        BOOST_CHECK_CLOSE(x.array()[1], -1.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE (Addition)
{
    const auto x = DVec<2>{
        std::array<double,2>{ 0.1, 2.3 }
    };

    const auto two_x = x + x;

    BOOST_CHECK_CLOSE(two_x.array()[0], 0.2, 1.0e-10);
    BOOST_CHECK_CLOSE(two_x.array()[1], 4.6, 1.0e-10);
}

BOOST_AUTO_TEST_CASE (Subtraction)
{
    const auto x = DVec<2>{
        std::array<double,2>{ 0.1, 2.3 }
    };

    const auto y = DVec<2>{
        std::array<double,2>{ 10.9, 8.7 }
    };

    const auto x_minus_y = x - y;

    BOOST_CHECK_CLOSE(x_minus_y.array()[0], -10.8, 1.0e-10);
    BOOST_CHECK_CLOSE(x_minus_y.array()[1], - 6.4, 1.0e-10);
}

BOOST_AUTO_TEST_CASE (Mult_By_Scalar)
{
    // x *= a
    {
        auto x = DVec<2> {
            std::array<double,2>{ 0.1, 2.3 }
        };

        x *= 5.0;

        BOOST_CHECK_CLOSE(x.array()[0],  0.5, 1.0e-10);
        BOOST_CHECK_CLOSE(x.array()[1], 11.5, 1.0e-10);
    }

    // y <- x * a
    {
        const auto x = DVec<2> {
            std::array<double,2>{ 0.1, 2.3 }
        };

        {
            const auto y = x * 5.0;

            BOOST_CHECK_CLOSE(y.array()[0],  0.5, 1.0e-10);
            BOOST_CHECK_CLOSE(y.array()[1], 11.5, 1.0e-10);
        }

        {
            const auto y = 2.5 * x;

            BOOST_CHECK_CLOSE(y.array()[0], 0.25, 1.0e-10);
            BOOST_CHECK_CLOSE(y.array()[1], 5.75, 1.0e-10);
        }
    }
}

BOOST_AUTO_TEST_CASE (Divide_By_Scalar)
{
    // x /= a
    {
        auto x = DVec<2> {
            std::array<double,2>{ 0.5, 11.5 }
        };

        x /= 5.0;

        BOOST_CHECK_CLOSE(x.array()[0], 0.1, 1.0e-10);
        BOOST_CHECK_CLOSE(x.array()[1], 2.3, 1.0e-10);
    }

    // y <- x / a
    {
        const auto x = DVec<2> {
            std::array<double,2>{ 0.25, 5.75 }
        };

        const auto y = x / 2.5;

        BOOST_CHECK_CLOSE(y.array()[0], 0.1, 1.0e-10);
        BOOST_CHECK_CLOSE(y.array()[1], 2.3, 1.0e-10);
    }
}

BOOST_AUTO_TEST_SUITE_END ()
