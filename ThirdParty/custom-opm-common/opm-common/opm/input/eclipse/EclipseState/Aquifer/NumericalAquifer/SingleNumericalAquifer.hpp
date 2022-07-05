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

#ifndef OPM_SINGLENUMERICALAQUIFER_HPP
#define OPM_SINGLENUMERICALAQUIFER_HPP

#include <vector>
#include <set>

#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/NumericalAquiferConnection.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/NumericalAquiferCell.hpp>

namespace Opm {
    class NNC;
    struct NNCdata;
    class FieldPropsManager;

    struct AquiferCellProps {
        double volume;
        double pore_volume;
        double depth;
        double porosity;
        int satnum;
        int pvtnum;
    };

    class SingleNumericalAquifer {
    public:
        explicit SingleNumericalAquifer(const size_t aqu_id);
        SingleNumericalAquifer() = default;

        void addAquiferCell(const NumericalAquiferCell& aqu_cell);
        void addAquiferConnection(const NumericalAquiferConnection& aqu_con);

        void postProcessConnections(const EclipseGrid& grid, const std::vector<int>& actnum);

        // TODO: the following two can be made one function. Let us see
        // how we use them at the end
        size_t numCells() const;
        size_t id() const;
        size_t numConnections() const;
        const NumericalAquiferCell* getCellPrt(size_t index) const;

        std::unordered_map<size_t, AquiferCellProps> aquiferCellProps() const;

        std::vector<NNCdata> aquiferCellNNCs() const;
        std::vector<NNCdata> aquiferConnectionNNCs(const EclipseGrid &grid, const FieldPropsManager &fp) const;

        const std::vector<NumericalAquiferConnection>& connections() const;

        bool operator==(const SingleNumericalAquifer& other) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer) {
            serializer(this->id_);
            serializer.vector(this->cells_);
            serializer.vector(this->connections_);
        }

        private:
            // Maybe this id_ is not necessary
            // Because if it is a map, the id will be there
            // Then adding aquifer cells will be much easier with the
            // default constructor
            size_t id_;
            std::vector<NumericalAquiferCell> cells_;
            std::vector<NumericalAquiferConnection> connections_;
        };
}


#endif //OPM_SINGLENUMERICALAQUIFER_HPP
