/*
  Copyright (C) 2020 SINTEF Digital

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

#ifndef OPM_NUMERICALAQUIFERCONNECTION_HPP
#define OPM_NUMERICALAQUIFERCONNECTION_HPP

#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <map>
#include <array>
#include <vector>

namespace Opm {

    class EclipseGrid;
    class Deck;
    class DeckRecord;

    struct NumericalAquiferConnection {
        // TODO: I do not think we need all the values here
        size_t aquifer_id;
        size_t I, J, K;
        size_t global_index;
        FaceDir::DirEnum face_dir;
        double trans_multipler;
        int trans_option;
        bool connect_active_cell;

        // The following are options related to VE simulation
        double ve_frac_relperm;
        double ve_frac_cappress;

        NumericalAquiferConnection(size_t i, size_t j, size_t k, size_t global_index, bool allow_connect_active, const DeckRecord& record);
        NumericalAquiferConnection() = default;

        bool operator==(const NumericalAquiferConnection& other) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer) {
            serializer(this->aquifer_id);
            serializer(this->I);
            serializer(this->J);
            serializer(this->K);
            serializer(this->global_index);
            serializer(this->face_dir);
            serializer(this->trans_multipler);
            serializer(this->trans_option);
            serializer(this->connect_active_cell);
            serializer(this->ve_frac_relperm);
            serializer(this->ve_frac_cappress);
        }

        static std::map<size_t, std::map<size_t, NumericalAquiferConnection>>
                generateConnections(const Deck& deck, const EclipseGrid& grid);
    private:
        static std::vector<NumericalAquiferConnection>
                connectionsFromSingleRecord(const EclipseGrid& grid, const DeckRecord& record);
    };
}

#endif //OPM_NUMERICALAQUIFERCONNECTION_HPP
