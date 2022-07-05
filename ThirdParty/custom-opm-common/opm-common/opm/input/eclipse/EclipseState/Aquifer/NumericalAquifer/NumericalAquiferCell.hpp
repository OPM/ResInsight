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

#ifndef OPM_NUMERICALAQUIFERCELL_HPP
#define OPM_NUMERICALAQUIFERCELL_HPP

#include <cstddef>
#include <optional>

namespace Opm {
    class DeckRecord;
    class EclipseGrid;
    class FieldPropsManager;

    struct NumericalAquiferCell {
        NumericalAquiferCell(const std::size_t record_id_, const DeckRecord&, const EclipseGrid&, const FieldPropsManager&);
        NumericalAquiferCell() = default;
        std::size_t aquifer_id{};
        std::size_t I{}, J{}, K{};
        double area{};
        double length{};
        double porosity{};
        double permeability{};
        double depth{};
        std::optional<double> init_pressure{};
        int pvttable{};
        int sattable{};
        std::size_t global_index{};
        std::size_t record_id{};

        double cellVolume() const;
        double poreVolume() const;
        double transmissiblity() const;
        bool operator == (const NumericalAquiferCell& other) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer) {
            serializer(this->aquifer_id);
            serializer(this->I);
            serializer(this->J);
            serializer(this->K);
            serializer(this->area);
            serializer(this->length);
            serializer(this->porosity);
            serializer(this->permeability);
            serializer(this->depth);
            serializer(this->init_pressure);
            serializer(this->pvttable);
            serializer(this->sattable);
            serializer(this->global_index);
            serializer(this->record_id);
        }
    };
}

#endif //OPM_NUMERICALAQUIFERCELL_HPP
