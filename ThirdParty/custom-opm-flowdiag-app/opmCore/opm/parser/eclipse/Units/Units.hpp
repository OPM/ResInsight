//===========================================================================
//
// File: Units.hpp
//
// Created: Thu Jul  2 09:19:08 2009
//
// Author(s): Halvor M Nilsen <hnil@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
Copyright 2009, 2010, 2011, 2012 SINTEF ICT, Applied Mathematics.
Copyright 2009, 2010, 2011, 2012 Statoil ASA.

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

#ifndef OPM_UNITS_HEADER
#define OPM_UNITS_HEADER

/**
The unit sets emplyed in ECLIPSE, in particular the FIELD units,
are quite inconsistent. Ideally one should choose units for a set
of base quantities like Mass,Time and Length and then derive the
units for e.g. pressure and flowrate in a consisten manner. However
that is not the case; for instance in the metric system we have:

[Length] = meters
[time] = days
[mass] = kg

This should give:

[Pressure] = [mass] / ([length] * [time]^2) = kg / (m * days * days)

Instead pressure is given in Bars. When it comes to FIELD units the
number of such examples is long.
*/
namespace Opm {
namespace prefix
    /// Conversion prefix for units.
{
constexpr const double micro = 1.0e-6;  /**< Unit prefix [\f$\mu\f$] */
constexpr const double milli = 1.0e-3;  /**< Unit prefix [m] */
constexpr const double centi = 1.0e-2;  /**< Non-standard unit prefix [c] */
constexpr const double deci  = 1.0e-1;  /**< Non-standard unit prefix [d] */
constexpr const double kilo  = 1.0e3;   /**< Unit prefix [k] */
constexpr const double mega  = 1.0e6;   /**< Unit prefix [M] */
constexpr const double giga  = 1.0e9;   /**< Unit prefix [G] */
} // namespace prefix

namespace unit
    /// Definition of various units.
    /// All the units are defined in terms of international standard
    /// units (SI).  Example of use: We define a variable \c k which
    /// gives a permeability. We want to set \c k to \f$1\,mD\f$.
    /// \code
    /// using namespace Opm::unit
    /// double k = 0.001*darcy;
    /// \endcode
    /// We can also use one of the prefixes defined in Opm::prefix
    /// \code
    /// using namespace Opm::unit
    /// using namespace Opm::prefix
    /// double k = 1.0*milli*darcy;
    /// \endcode
{
///\name Common powers
/// @{
constexpr double square(double v) { return v * v;     }
constexpr double cubic (double v) { return v * v * v; }
/// @}

// --------------------------------------------------------------
// Basic (fundamental) units and conversions
// --------------------------------------------------------------

/// \name Length
/// @{
constexpr const double meter =  1;
constexpr const double inch  =  2.54 * prefix::centi*meter;
constexpr const double feet  = 12    * inch;
/// @}

/// \name Time
/// @{
constexpr const double second =   1;
constexpr const double minute =  60 * second;
constexpr const double hour   =  60 * minute;
constexpr const double day    =  24 * hour;
constexpr const double year   = 365 * day;
/// @}

/// \name Volume
/// @{
constexpr const double gallon = 231 * cubic(inch);
constexpr const double stb    =  42 * gallon;
constexpr const double liter  =   1 * cubic(prefix::deci*meter);
/// @}

/// \name Mass
/// @{
constexpr const double kilogram = 1;
constexpr const double gram     = 1.0e-3 * kilogram;
// http://en.wikipedia.org/wiki/Pound_(mass)#Avoirdupois_pound
constexpr const double pound    = 0.45359237 * kilogram;
/// @}

// --------------------------------------------------------------
// Standardised constants
// --------------------------------------------------------------

/// \name Standardised constant
/// @{
constexpr const double gravity = 9.80665 * meter/square(second);
/// @}

// --------------------------------------------------------------
// Derived units and conversions
// --------------------------------------------------------------

/// \name Force
/// @{
constexpr const double Newton = kilogram*meter / square(second); // == 1
constexpr const double dyne   = 1e-5*Newton;
constexpr const double lbf    = pound * gravity; // Pound-force
                                                 /// @}

                                                 /// \name Pressure
                                                 /// @{
constexpr const double Pascal = Newton / square(meter); // == 1
constexpr const double barsa  = 100000 * Pascal;
constexpr const double atm    = 101325 * Pascal;
constexpr const double psia   = lbf / square(inch);
/// @}

/// \name Temperature. This one is more complicated
/// because the unit systems used by Eclipse (i.e. degrees
/// Celsius and degrees Fahrenheit require to add or
/// subtract an offset for the conversion between from/to
/// Kelvin
/// @{
constexpr const double degCelsius = 1.0; // scaling factor °C -> K
constexpr const double degCelsiusOffset = 273.15; // offset for the °C -> K conversion

constexpr const double degFahrenheit = 5.0/9; // scaling factor °F -> K
constexpr const double degFahrenheitOffset = 255.37; // offset for the °C -> K conversion
                                                     /// @}

                                                     /// \name Viscosity
                                                     /// @{
constexpr const double Pas   = Pascal * second; // == 1
constexpr const double Poise = prefix::deci*Pas;
/// @}

namespace perm_details {
constexpr const double p_grad   = atm / (prefix::centi*meter);
constexpr const double area     = square(prefix::centi*meter);
constexpr const double flux     = cubic (prefix::centi*meter) / second;
constexpr const double velocity = flux / area;
constexpr const double visc     = prefix::centi*Poise;
constexpr const double darcy    = (velocity * visc) / p_grad;
//                    == 1e-7 [m^2] / 101325
//                    == 9.869232667160130e-13 [m^2]
}
/// \name Permeability
/// @{
///
/// A porous medium with a permeability of 1 darcy permits a flow (flux)
/// of \f$1\,\mathit{cm}^3/s\f$ of a fluid with viscosity
/// \f$1\,\mathit{cP}\f$ (\f$1\,mPa\cdot s\f$) under a pressure gradient
/// of \f$1\,\mathit{atm}/\mathit{cm}\f$ acting across an area of
/// \f$1\,\mathit{cm}^2\f$.
///
constexpr const double darcy = perm_details::darcy;
/// @}

/**
* Unit conversion routines.
*/
namespace convert {
/**
* Convert from external units of measurements to equivalent
* internal units of measurements.  Note: The internal units of
* measurements are *ALWAYS*, and exclusively, SI.
*
* Example: Convert a double @c kx, containing a permeability value
* in units of milli-darcy (mD) to the equivalent value in SI units
* (i.e., \f$m^2\f$).
* \code
*    using namespace Opm::unit;
*    using namespace Opm::prefix;
*    convert::from(kx, milli*darcy);
* \endcode
*
* @param[in] q    Physical quantity.
* @param[in] unit Physical unit of measurement.
* @return Value of @c q in equivalent SI units of measurements.
*/
constexpr double from(const double q, const double unit)
{
    return q * unit;
}

/**
* Convert from internal units of measurements to equivalent
* external units of measurements.  Note: The internal units of
* measurements are *ALWAYS*, and exclusively, SI.
*
* Example: Convert a <CODE>std::vector<double> p</CODE>, containing
* pressure values in the SI unit Pascal (i.e., unit::Pascal) to the
* equivalent values in Psi (unit::psia).
* \code
*    using namespace Opm::unit;
*    std::transform(p.begin(), p.end(), p.begin(),
*                   boost::bind(convert::to, _1, psia));
* \endcode
*
* @param[in] q    Physical quantity, measured in SI units.
* @param[in] unit Physical unit of measurement.
* @return Value of @c q in unit <CODE>unit</CODE>.
*/
constexpr double to(const double q, const double unit)
{
    return q / unit;
}
} // namespace convert
}

namespace Metric {
using namespace prefix;
using namespace unit;
constexpr const double Pressure             = barsa;
constexpr const double Temperature          = degCelsius;
constexpr const double TemperatureOffset    = degCelsiusOffset;
constexpr const double AbsoluteTemperature  = degCelsius; // actually [K], but the these two are identical
constexpr const double Length               = meter;
constexpr const double Time                 = day;
constexpr const double Mass                 = kilogram;
constexpr const double Permeability         = milli*darcy;
constexpr const double Transmissibility     = centi*Poise*cubic(meter)/(day*barsa);
constexpr const double LiquidSurfaceVolume  = cubic(meter);
constexpr const double GasSurfaceVolume     = cubic(meter);
constexpr const double ReservoirVolume      = cubic(meter);
constexpr const double GasDissolutionFactor = GasSurfaceVolume/LiquidSurfaceVolume;
constexpr const double OilDissolutionFactor = LiquidSurfaceVolume/GasSurfaceVolume;
constexpr const double Density              = kilogram/cubic(meter);
constexpr const double PolymerDensity       = kilogram/cubic(meter);
constexpr const double Salinity             = kilogram/cubic(meter);
constexpr const double Viscosity            = centi*Poise;
constexpr const double Timestep             = day;
constexpr const double SurfaceTension       = dyne/(centi*meter);
}


namespace Field {
using namespace prefix;
using namespace unit;
constexpr const double Pressure             = psia;
constexpr const double Temperature          = degFahrenheit;
constexpr const double TemperatureOffset    = degFahrenheitOffset;
constexpr const double AbsoluteTemperature  = degFahrenheit; // actually [°R], but the these two are identical
constexpr const double Length               = feet;
constexpr const double Time                 = day;
constexpr const double Mass                 = pound;
constexpr const double Permeability = milli*darcy;
constexpr const double Transmissibility = centi*Poise*stb/(day*psia);
constexpr const double LiquidSurfaceVolume  = stb;
constexpr const double GasSurfaceVolume     = 1000*cubic(feet);
constexpr const double ReservoirVolume      = stb;
constexpr const double GasDissolutionFactor = GasSurfaceVolume/LiquidSurfaceVolume;
constexpr const double OilDissolutionFactor = LiquidSurfaceVolume/GasSurfaceVolume;
constexpr const double Density              = pound/cubic(feet);
constexpr const double PolymerDensity       = pound/stb;
constexpr const double Salinity             = pound/stb;
constexpr const double Viscosity            = centi*Poise;
constexpr const double Timestep             = day;
constexpr const double SurfaceTension       = dyne/(centi*meter);
}


namespace Lab {
using namespace prefix;
using namespace unit;
constexpr const double Pressure             = atm;
constexpr const double Temperature          = degCelsius;
constexpr const double TemperatureOffset    = degCelsiusOffset;
constexpr const double AbsoluteTemperature  = degCelsius; // actually [K], but the these two are identical
constexpr const double Length               = centi*meter;
constexpr const double Time                 = hour;
constexpr const double Mass                 = gram;
constexpr const double Permeability         = milli*darcy;
constexpr const double Transmissibility     = centi*Poise*cubic(centi*meter)/(hour*atm);
constexpr const double LiquidSurfaceVolume  = cubic(centi*meter);
constexpr const double GasSurfaceVolume     = cubic(centi*meter);
constexpr const double ReservoirVolume      = cubic(centi*meter);
constexpr const double GasDissolutionFactor = GasSurfaceVolume/LiquidSurfaceVolume;
constexpr const double OilDissolutionFactor = LiquidSurfaceVolume/GasSurfaceVolume;
constexpr const double Density              = gram/cubic(centi*meter);
constexpr const double PolymerDensity       = gram/cubic(centi*meter);
constexpr const double Salinity             = gram/cubic(centi*meter);
constexpr const double Viscosity            = centi*Poise;
constexpr const double Timestep             = hour;
constexpr const double SurfaceTension       = dyne/(centi*meter);
}

}

#endif // OPM_UNITS_HEADER
