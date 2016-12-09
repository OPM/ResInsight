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
 * \file
 * Constants and routines to assist in handling units of measurement.  These are
 * geared towards handling common units in reservoir descriptions.
 */

namespace Opm
{
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
            inline double from(const double q, const double unit)
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
            inline double to(const double q, const double unit)
            {
                return q / unit;
            }
        } // namespace convert


#ifndef HAS_ATTRIBUTE_UNUSED
        namespace detail {
            // Some units are sometimes unused, and generate a (potentially) large number of warnings
            // Adding them here silences these warnings, and should have no side-effects
            //JJS double __attribute__((unused)) unused_units = stb + liter + barsa + psia + darcy;
        } // namespace detail
#endif

    } // namespace unit
} // namespace Opm
#endif // OPM_UNITS_HEADER