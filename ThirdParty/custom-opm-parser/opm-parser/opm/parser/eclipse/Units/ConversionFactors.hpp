/*
  Copyright 2013 Statoil ASA.

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

#ifndef CONVERSION_FACTORS_HPP
#define CONVERSION_FACTORS_HPP


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

    namespace details {
        // Note: the following has been lifted from opm/core/utility/Units.hpp.

        namespace prefix
        /// Conversion prefix for units.
        {
            const double micro = 1.0e-6;  /**< Unit prefix [\f$\mu\f$] */
            const double milli = 1.0e-3;  /**< Unit prefix [m] */
            const double centi = 1.0e-2;  /**< Non-standard unit prefix [c] */
            const double deci  = 1.0e-1;  /**< Non-standard unit prefix [d] */
            const double kilo  = 1.0e3;   /**< Unit prefix [k] */
            const double mega  = 1.0e6;   /**< Unit prefix [M] */
            const double giga  = 1.0e9;   /**< Unit prefix [G] */
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
            inline double square(double v) { return v * v;     }
            inline double cubic (double v) { return v * v * v; }
            /// @}

            // --------------------------------------------------------------
            // Basic (fundamental) units and conversions
            // --------------------------------------------------------------

            /// \name Length
            /// @{
            const double meter =  1;
            const double inch  =  2.54 * prefix::centi*meter;
            const double feet  = 12    * inch;
            /// @}

            /// \name Time
            /// @{
            const double second =   1;
            const double minute =  60 * second;
            const double hour   =  60 * minute;
            const double day    =  24 * hour;
            const double year   = 365 * day;
            /// @}

            /// \name Volume
            /// @{
            const double gallon = 231 * cubic(inch);
            const double stb    =  42 * gallon;
            const double liter  =   1 * cubic(prefix::deci*meter);
            /// @}

            /// \name Mass
            /// @{
            const double kilogram = 1;
            // http://en.wikipedia.org/wiki/Pound_(mass)#Avoirdupois_pound
            const double pound    = 0.45359237 * kilogram;
            /// @}

            // --------------------------------------------------------------
            // Standardised constants
            // --------------------------------------------------------------

            /// \name Standardised constant
            /// @{
            const double gravity = 9.80665 * meter/square(second);
            /// @}

            // --------------------------------------------------------------
            // Derived units and conversions
            // --------------------------------------------------------------

            /// \name Force
            /// @{
            const double Newton = kilogram*meter / square(second); // == 1
            const double lbf    = pound * gravity; // Pound-force
            /// @}

            /// \name Pressure
            /// @{
            const double Pascal = Newton / square(meter); // == 1
            const double barsa  = 100000 * Pascal;
            const double atm    = 101325 * Pascal;
            const double psia   = lbf / square(inch);
            /// @}

            /// \name Temperature. This one is more complicated
            /// because the unit systems used by Eclipse (i.e. degrees
            /// Celsius and degrees Fahrenheit require to add or
            /// subtract an offset for the conversion between from/to
            /// Kelvin
            /// @{
            const double degCelsius = 1.0; // scaling factor °C -> K
            const double degCelsiusOffset = 273.15; // offset for the °C -> K conversion

            const double degFahrenheit = 5.0/9; // scaling factor °F -> K
            const double degFahrenheitOffset = 255.37; // offset for the °C -> K conversion
            /// @}

            /// \name Viscosity
            /// @{
            const double Pas   = Pascal * second; // == 1
            const double Poise = prefix::deci*Pas;
            /// @}

            namespace perm_details {
                const double p_grad   = atm / (prefix::centi*meter);
                const double area     = square(prefix::centi*meter);
                const double flux     = cubic (prefix::centi*meter) / second;
                const double velocity = flux / area;
                const double visc     = prefix::centi*Poise;
                const double darcy    = (velocity * visc) / p_grad;
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
            const double darcy = perm_details::darcy;
            /// @}
        }

    } // namespace details


    namespace Metric {
        using namespace details::prefix;
        using namespace details::unit;
        const double Pressure             = barsa;
        const double Temperature          = degCelsius;
        const double TemperatureOffset    = degCelsiusOffset;
        const double AbsoluteTemperature  = degCelsius; // actually [K], but the these two are identical
        const double Length               = meter;
        const double Time                 = day;
        const double Mass                 = kilogram;
        const double Permeability         = milli*darcy;
        const double Transmissibility     = centi*Poise*cubic(meter)/(day*barsa);
        const double LiquidSurfaceVolume  = cubic(meter);
        const double GasSurfaceVolume     = cubic(meter);
        const double ReservoirVolume      = cubic(meter);
        const double GasDissolutionFactor = GasSurfaceVolume/LiquidSurfaceVolume;
        const double OilDissolutionFactor = LiquidSurfaceVolume/GasSurfaceVolume;
        const double Density              = kilogram/cubic(meter);
        const double PolymerDensity       = kilogram/cubic(meter);
        const double Salinity             = kilogram/cubic(meter);
        const double Viscosity            = centi*Poise;
        const double Timestep             = day;
    }


    namespace Field {
        using namespace details::prefix;
        using namespace details::unit;
        const double Pressure             = psia;
        const double Temperature          = degFahrenheit;
        const double TemperatureOffset    = degFahrenheitOffset;
        const double AbsoluteTemperature  = degFahrenheit; // actually [°R], but the these two are identical
        const double Length               = feet;
        const double Time                 = day;
        const double Mass                 = pound;
        const double Permeability         = milli*darcy;
        const double Transmissibility     = centi*Poise*stb/(day*psia);
        const double LiquidSurfaceVolume  = stb;
        const double GasSurfaceVolume     = 1000*cubic(feet);
        const double ReservoirVolume      = stb;
        const double GasDissolutionFactor = GasSurfaceVolume/LiquidSurfaceVolume;
        const double OilDissolutionFactor = LiquidSurfaceVolume/GasSurfaceVolume;
        const double Density              = pound/cubic(feet);
        const double PolymerDensity       = pound/stb;
        const double Salinity             = pound/stb;
        const double Viscosity            = centi*Poise;
        const double Timestep             = day;
    }

}

#endif
