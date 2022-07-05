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

#ifndef OPM_AQUIFERFETP_HPP
#define OPM_AQUIFERFETP_HPP

/*
  The Aquiferfetp which stands for AquiferFetkovich is a data container object meant to hold the data for the fetkovich aquifer model.
  This includes the logic for parsing as well as the associated tables. It is meant to be used by opm-grid and opm-simulators in order to
  implement the Fetkovich analytical aquifer model in OPM Flow.
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

class Aquifetp {
    public:

    struct AQUFETP_data
    {
        friend class Aquifetp;

        AQUFETP_data() = default;
        AQUFETP_data(const DeckRecord& record, const TableManager& tables);
        AQUFETP_data(const int aquiferID_,
                     const int pvttableID_,
                     const double J_,
                     const double C_t_,
                     const double V0_,
                     const double d0_,
                     const double p0_);

        int aquiferID{};
        int pvttableID{};

        double prod_index{};
        double total_compr{};
        double initial_watvolume{};
        double datum_depth{};

        std::optional<double> initial_pressure{};

        static AQUFETP_data serializeObject();

        double timeConstant() const { return this->time_constant_; }
        double waterDensity() const { return this->water_density_; }
        double waterViscosity() const { return this->water_viscosity_; }

        bool operator==(const AQUFETP_data& other) const;

        void finishInitialisation(const TableManager& tables);

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(this->aquiferID);
            serializer(this->pvttableID);
            serializer(this->prod_index);
            serializer(this->total_compr);
            serializer(this->initial_watvolume);
            serializer(this->datum_depth);
            serializer(this->initial_pressure);
            serializer(this->time_constant_);
            serializer(this->water_density_);
            serializer(this->water_viscosity_);
        }

    private:
        double time_constant_{};
        double water_density_{};
        double water_viscosity_{};
    };

    Aquifetp() = default;
    Aquifetp(const TableManager& tables, const Deck& deck);
    explicit Aquifetp(const std::vector<Aquifetp::AQUFETP_data>& data);

    void loadFromRestart(const RestartIO::RstAquifer& rst,
                         const TableManager&          tables);

    static Aquifetp serializeObject();

    const std::vector<Aquifetp::AQUFETP_data>& data() const;

    std::size_t size() const;
    std::vector<Aquifetp::AQUFETP_data>::const_iterator begin() const;
    std::vector<Aquifetp::AQUFETP_data>::const_iterator end() const;
    bool operator==(const Aquifetp& other) const;

    bool hasAquifer(const int aquID) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(m_aqufetp);
    }

private:
    std::vector<Aquifetp::AQUFETP_data> m_aqufetp;
};
}


#endif
