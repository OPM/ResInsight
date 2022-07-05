/*
  Copyright (C) 2017 TNO

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

#ifndef OPM_AQUIFERCT_HPP
#define OPM_AQUIFERCT_HPP

/*
  The AquiferCT which stands for AquiferCarterTracy is a data container object meant to hold the data for the aquifer carter tracy model.
  This includes the logic for parsing as well as the associated tables. It is meant to be used by opm-grid and opm-simulators in order to
  implement the Carter Tracy analytical aquifer model in OPM Flow.
*/

#include <cstddef>
#include <optional>
#include <vector>

namespace Opm {
    class Deck;
    class DeckRecord;
    class TableManager;
}

namespace Opm { namespace RestartIO {
    class RstAquifer;
}} // Opm::RestartIO

namespace Opm {

    class AquiferCT {
        public:

        struct AQUCT_data
        {
            friend class AquiferCT;

            AQUCT_data() = default;
            AQUCT_data(const DeckRecord& record, const TableManager& tables);
            AQUCT_data(const int aqID,
                       const int infID,
                       const int pvtID,
                       const double phi_aq_,
                       const double d0_,
                       const double C_t_,
                       const double r_o_,
                       const double k_a_,
                       const double h_,
                       const double theta_,
                       const double p0_,
                       const double T0_);

            int aquiferID{};
            int inftableID{};
            int pvttableID{};

            double porosity{};
            double datum_depth{};
            double total_compr{};
            double inner_radius{};
            double permeability{};
            double thickness{};
            double angle_fraction{};

            std::optional<double> initial_pressure{};
            std::optional<double> initial_temperature{};
            std::vector<double> dimensionless_time{};
            std::vector<double> dimensionless_pressure{};

            static AQUCT_data serializeObject();

            double timeConstant() const { return this->time_constant_; }
            double influxConstant() const { return this->influx_constant_; }
            double waterDensity() const { return this->water_density_; }
            double waterViscosity() const { return this->water_viscosity_; }

            bool operator==(const AQUCT_data& other) const;

            void finishInitialisation(const TableManager& tables);

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(this->aquiferID);
                serializer(this->inftableID);
                serializer(this->pvttableID);
                serializer(this->porosity);
                serializer(this->datum_depth);
                serializer(this->total_compr);
                serializer(this->inner_radius);
                serializer(this->permeability);
                serializer(this->thickness);
                serializer(this->angle_fraction);
                serializer(this->initial_pressure);
                serializer(this->initial_temperature);
                serializer(this->dimensionless_time);
                serializer(this->dimensionless_pressure);
                serializer(this->time_constant_);
                serializer(this->influx_constant_);
                serializer(this->water_density_);
                serializer(this->water_viscosity_);
            }

        private:
            double time_constant_{};
            double influx_constant_{};
            double water_density_{};
            double water_viscosity_{};
        };

        AquiferCT() = default;
        AquiferCT(const TableManager& tables, const Deck& deck);
        AquiferCT(const std::vector<AquiferCT::AQUCT_data>& data);

        void loadFromRestart(const RestartIO::RstAquifer& rst,
                             const TableManager&          tables);

        static AquiferCT serializeObject();

        std::size_t size() const;
        std::vector<AquiferCT::AQUCT_data>::const_iterator begin() const;
        std::vector<AquiferCT::AQUCT_data>::const_iterator end() const;
        const std::vector<AquiferCT::AQUCT_data>& data() const;
        bool operator==(const AquiferCT& other) const;

        bool hasAquifer(const int aquID) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer.vector(m_aquct);
        }

    private:
        std::vector<AquiferCT::AQUCT_data> m_aquct;
    };
}


#endif
