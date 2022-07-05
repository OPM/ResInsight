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

#ifndef OPM_NUMERICALAQUIFERS_HPP
#define OPM_NUMERICALAQUIFERS_HPP

#include <map>
#include <unordered_map>

#include <stddef.h>

#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/SingleNumericalAquifer.hpp>

namespace Opm {
    class Deck;
    class EclipseGrid;
    class FieldPropsManager;
    struct NumericalAquiferCell;

    class NumericalAquifers {
    public:
        NumericalAquifers() = default;
        NumericalAquifers(const Deck& deck, const EclipseGrid& grid, const FieldPropsManager& field_props);

        int numRecords() const { return static_cast<int>(this->m_num_records); }
        size_t size() const;
        bool hasAquifer(size_t aquifer_id) const;
        const SingleNumericalAquifer& getAquifer(size_t aquifer_id) const;
        const std::map<size_t, SingleNumericalAquifer>& aquifers() const;
        bool operator==(const NumericalAquifers& other) const;

        std::unordered_map<size_t, const NumericalAquiferCell*> allAquiferCells() const;

        std::unordered_map<size_t, double> aquiferCellVolumes() const;

        std::vector<NNCdata> aquiferCellNNCs() const;
        std::vector<NNCdata> aquiferConnectionNNCs(const EclipseGrid& grid, const FieldPropsManager& fp) const;

        std::unordered_map<size_t, AquiferCellProps> aquiferCellProps() const;

        void initConnections(const Deck& deck, const EclipseGrid& grid);
        void postProcessConnections(const EclipseGrid& grid, const std::vector<int>& actnum);

        static NumericalAquifers serializeObject();
        template <class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer.map(this->m_aquifers);
            serializer(this->m_num_records);
        }

    private:
        std::map<size_t, SingleNumericalAquifer> m_aquifers{};
        size_t m_num_records{0};

        void addAquiferCell(const NumericalAquiferCell& aqu_cell);
    };
}

#endif //OPM_NUMERICALAQUIFERS_HPP
