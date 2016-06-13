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

#ifndef UNITSYSTEM_H
#define UNITSYSTEM_H

#include <string>
#include <map>
#include <memory>

namespace Opm {

    class Dimension;

    class UnitSystem {
    public:
        enum UnitType {
          UNIT_TYPE_METRIC = 0,
          UNIT_TYPE_FIELD  = 1,
          UNIT_TYPE_LAB    = 2
        };

        enum class measure : int {
            length,
            time,
            density,
            pressure,
            temperature_absolute,
            temperature,
            viscosity,
            permeability,
            liquid_surface_volume,
            gas_surface_volume,
            volume,
            liquid_surface_rate,
            gas_surface_rate,
            rate,
            transmissibility,
            mass,
        };

        UnitSystem(UnitType unit);

        const std::string& getName() const;
        UnitType getType() const;

        void addDimension(const std::string& dimension, double SIfactor, double SIoffset = 0.0);
        void addDimension(std::shared_ptr<const Dimension> dimension);
        std::shared_ptr<const Dimension> getNewDimension(const std::string& dimension);
        std::shared_ptr<const Dimension> getDimension(const std::string& dimension) const;
        bool hasDimension(const std::string& dimension) const;
        bool equal(const UnitSystem& other) const;

        std::shared_ptr<const Dimension> parse(const std::string& dimension) const;

        double from_si( measure, double ) const;
        double to_si( measure, double ) const;

        static UnitSystem * newMETRIC();
        static UnitSystem * newFIELD();
    private:
        std::shared_ptr<const Dimension> parseFactor(const std::string& dimension) const;

        std::string m_name;
        UnitType m_unittype;
        std::map<std::string , std::shared_ptr<const Dimension> > m_dimensions;
        const double* measure_table_from_si;
        const double* measure_table_to_si;
    };
}


#endif

