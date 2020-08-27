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
#include <vector>
#include <memory>

#include <opm/parser/eclipse/Units/Dimension.hpp>

namespace Opm {

    class UnitSystem {
    public:
        enum class UnitType {
          UNIT_TYPE_METRIC = 0,
          UNIT_TYPE_FIELD  = 1,
          UNIT_TYPE_LAB    = 2,
          UNIT_TYPE_PVT_M  = 3,
          UNIT_TYPE_INPUT  = 4
        };

        enum class measure : int {
            identity,
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
            geometric_volume,
            liquid_surface_rate,
            gas_surface_rate,
            rate,
            geometric_volume_rate,
            transmissibility,
            effective_Kh,
            mass,
            mass_rate,
            gas_oil_ratio,
            oil_gas_ratio,
            water_cut,
            gas_formation_volume_factor,
            oil_formation_volume_factor,
            water_formation_volume_factor,
            gas_inverse_formation_volume_factor,
            oil_inverse_formation_volume_factor,
            water_inverse_formation_volume_factor,
            liquid_productivity_index,
            gas_productivity_index,
            energy,
            icd_strength,
            polymer_density,
            _count // New entries must be added *before* this
        };

        explicit UnitSystem(int ecl_id);
        explicit UnitSystem(UnitType unit = UnitType::UNIT_TYPE_METRIC);
        explicit UnitSystem(const std::string& deck_name);

        static UnitSystem serializeObject();

        const std::string& getName() const;
        UnitType getType() const;
        int ecl_id() const;

        void addDimension(const std::string& dimension , const Dimension& dim);
        void addDimension(const std::string& dimension, double SIfactor, double SIoffset = 0.0);
        const Dimension& getNewDimension(const std::string& dimension);
        const Dimension& getDimension(const std::string& dimension) const;
        Dimension getDimension(measure m) const;


        bool hasDimension(const std::string& dimension) const;
        bool equal(const UnitSystem& other) const;

        bool operator==( const UnitSystem& ) const;
        bool operator!=( const UnitSystem& ) const;

        Dimension parse(const std::string& dimension) const;

        double from_si( measure, double ) const;
        double to_si( measure, double ) const;
        void from_si( measure, std::vector<double>& ) const;
        void to_si( measure, std::vector<double>& ) const;
        const char* name( measure ) const;
        std::string deck_name() const;
        std::size_t use_count() const;

        static bool valid_name(const std::string& deck_name);
        static UnitSystem newMETRIC();
        static UnitSystem newFIELD();
        static UnitSystem newLAB();
        static UnitSystem newPVT_M();
        static UnitSystem newINPUT();

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_name);
            serializer(m_unittype);
            serializer.map(m_dimensions);
            serializer(m_use_count);
            if (!serializer.isSerializing())
                init();
        }

    private:
        Dimension parseFactor( const std::string& ) const;
        void init();
        void initINPUT();
        void initMETRIC();
        void initFIELD();
        void initPVT_M();
        void initLAB();

        std::string m_name;
        UnitType m_unittype;
        std::map< std::string , Dimension > m_dimensions;
        const double* measure_table_to_si_offset;
        const double* measure_table_from_si;
        const double* measure_table_to_si;
        const char* const*  unit_name_table;

        /*
          The active unit system is determined runtime, to be certain that we do
          not end up in a situation where we first use the default unit system,
          and then subsequently change it.

          The Deck::selectActiveUnitSystem() method has this code:

             const auto& current = this->getActiveUnitSystem();
             if (current.use_count() > 0)
                  throw std::logic_error("Sorry - can not change unit system halways");


        */
        mutable std::size_t m_use_count = 0;
    };
}


#endif

