/*
  Copyright 2014 Andreas Lauser

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
#ifndef ECLIPSE_SATFUNCPROPERTY_INITIALIZERS_HPP
#define ECLIPSE_SATFUNCPROPERTY_INITIALIZERS_HPP

#include <memory>
#include <string>
#include <vector>

namespace Opm {
    class Phases;
    class TableManager;
}

namespace Opm { namespace satfunc {

    /// Collection of unscaled/raw saturation range endpoints extracted
    /// directly from tables of tabulated saturation functions.
    struct RawTableEndPoints
    {
        /// Connate saturation endpoints
        struct {
            /// Connate gas saturation.  One value for each saturation region.
            /// All zero values unless gas is an active phase.
            std::vector<double> gas;

            /// Connate water saturation.  One value for each saturation region.
            /// All zero values unless water is an active phase.
            std::vector<double> water;
        } connate;

        /// Critical saturation endpoints
        struct {
            /// Critical saturation of oil in oil/gas two-phase system.  One
            /// value for each saturation region.  All zero values unless oil
            /// is an active phase.
            std::vector<double> oil_in_gas;

            /// Critical saturation of oil in oil/water two-phase system.  One
            /// value for each saturation region.  All zero values unless oil
            /// is an active phase.
            std::vector<double> oil_in_water;

            /// Critical saturation of gas.  One value for each saturation
            /// region.  All zero values unless oil is an active phase.
            std::vector<double> gas;

            /// Critical saturation of water.  One value for each saturation
            /// region.  All zero values unless oil is an active phase.
            std::vector<double> water;
        } critical;

        /// Maximum saturation endpoints
        struct {
            /// Maximum gas saturation value.  One value for each saturation
            /// region  All zero values unless gas is an active phase.
            std::vector<double> gas;

            /// Maximum water saturation value.  One value for each saturation
            /// region  All zero values unless gas is an active phase.
            std::vector<double> water;
        } maximum;

        bool operator==(const RawTableEndPoints& other) const {
            return this->connate.gas == other.connate.gas &&
                   this->connate.water == other.connate.water &&
                   this->critical.gas == other.critical.gas &&
                   this->critical.water == other.critical.water &&
                   this->maximum.gas == other.maximum.gas &&
                   this->maximum.water == other.maximum.water;
        }
    };

    /// Collection of unscaled/raw saturation function value range endpoints
    /// extracted directly from tables of tabulated saturation functions.
    struct RawFunctionValues
    {
        /// Function values for relative permeability of oil.
        struct {
            /// Maximum relative permeability value of oil in both oil/gas and
            /// oil/water two-phase systems.  One value for each saturation
            /// region.  All zero values unless oil is an active phase.
            std::vector<double> max;

            /// Relative permeability of oil at critical saturation of
            /// displacing phase in oil/gas two-phase system.  One value for
            /// each saturation region.  All zero values unless oil is
            /// an active phase.
            std::vector<double> rg;

            /// Relative permeability of oil at critical saturation of
            /// displacing phase in oil/water two-phase system.  One value
            /// for each saturation region.  All zero values unless oil is
            /// an active phase.
            std::vector<double> rw;
        } kro;

        /// Function values for relative permeability of gas.
        struct {
            /// Maximum relative permeability value of gas.  One value for
            /// each saturation region.  All zero values unless gas is
            /// an active phase.
            std::vector<double> max;

            /// Relative permeability of gas at critical saturation of
            /// displacing phase.  One value for each saturation region.
            /// All zero values unless gas is an active phase.
            std::vector<double> r;
        } krg;

        /// Function values for relative permeability of gas.
        struct {
            /// Maximum relative permeability value of water.  One value for
            /// each saturation region.  All zero values unless water is
            /// an active phase.
            std::vector<double> max;

            /// Relative permeability of water at critical saturation of
            /// displacing phase.  One value for each saturation region.
            /// All zero values unless water is an active phase.
            std::vector<double> r;
        } krw;

        /// Maximum capillary function values.
        struct {
            /// Maximum gas/oil capillary pressure value (Pg - Po).  One
            /// value for each saturation region.  All zero values unless
            /// both gas and oil are active phase.
            std::vector<double> g;

            /// Maximum oil/eater capillary pressure value (Po - Pw).  One
            /// value for each saturation region.  All zero values unless
            /// both oil and water are active phase.
            std::vector<double> w;
        } pc;
    };

    RawTableEndPoints
    getRawTableEndpoints(const Opm::TableManager& tm,
                         const Opm::Phases&       phases,
                         const double             tolcrit);

    RawFunctionValues
    getRawFunctionValues(const Opm::TableManager& tm,
                         const Opm::Phases&       phases,
                         const RawTableEndPoints& ep);

    std::vector<double> init(const std::string& kewyord,
                             const TableManager& tables,
                             const Phases& phases,
                             const RawTableEndPoints& ep,
                             const std::vector<double>& cell_depth,
                             const std::vector<int>& num,
                             const std::vector<int>& endnum);

}} // namespace Opm::satfunc

#endif // ECLIPSE_SATFUNCPROPERTY_INITIALIZERS_HPP
