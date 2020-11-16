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
#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/parser/eclipse/EclipseState/Aquancon.hpp>

#include <unordered_map>
#include <utility>
#include <algorithm>
#include <iterator>
#include <iostream>

namespace Opm {

    namespace{

        void add_cell(std::unordered_map<std::size_t, Aquancon::AquancCell>& work, Aquancon::AquancCell cell) {
            auto cell_iter = work.find(cell.global_index);
            if (cell_iter == work.end())
                work.insert( std::make_pair(cell.global_index, std::move(cell)));
            else {
                auto& prev_cell = cell_iter->second;
                if (prev_cell.aquiferID == cell.aquiferID) {
                    if (prev_cell.influx_coeff.first != cell.influx_coeff.first)
                        throw std::invalid_argument("Can not combine defaulted and not defaulted influx coefficient");

                    if (prev_cell.influx_coeff.first) {
                        if (cell.influx_coeff.second == 0)
                            prev_cell.influx_coeff.second = 0;
                        else
                            prev_cell.influx_coeff.second += cell.influx_coeff.second;
                    }
                } else {
                    std::string msg = "Cell with global index: " + std::to_string(cell.global_index) + " is already connected to Aquifer: " + std::to_string(prev_cell.aquiferID);
                    throw std::invalid_argument( msg );
                }
            }
        }
    }


    Aquancon::Aquancon(const EclipseGrid& grid, const Deck& deck)
    {
        std::unordered_map<std::size_t, Aquancon::AquancCell> work;
        for (std::size_t iaq = 0; iaq < deck.count("AQUANCON"); iaq++) {
            const auto& aquanconKeyword = deck.getKeyword("AQUANCON", iaq);
            for (const auto& aquanconRecord : aquanconKeyword) {
                const int aquiferID = aquanconRecord.getItem("AQUIFER_ID").get<int>(0);
                const int i1 = aquanconRecord.getItem("I1").get<int>(0) - 1;
                const int i2 = aquanconRecord.getItem("I2").get<int>(0) - 1;
                const int j1 = aquanconRecord.getItem("J1").get<int>(0) - 1;
                const int j2 = aquanconRecord.getItem("J2").get<int>(0) - 1;
                const int k1 = aquanconRecord.getItem("K1").get<int>(0) - 1;
                const int k2 = aquanconRecord.getItem("K2").get<int>(0) - 1;
                const double influx_mult = aquanconRecord.getItem("INFLUX_MULT").getSIDouble(0);
                const FaceDir::DirEnum faceDir
                    = FaceDir::FromString(aquanconRecord.getItem("FACE").getTrimmedString(0));

                const std::string& str_inside_reservoir
                    = aquanconRecord.getItem("CONNECT_ADJOINING_ACTIVE_CELL").getTrimmedString(0);
                const bool allow_aquifer_inside_reservoir = DeckItem::to_bool(str_inside_reservoir);

                // Loop over the cartesian indices to convert to the global grid index
                for (int k = k1; k <= k2; k++) {
                    for (int j = j1; j <= j2; j++) {
                        for (int i = i1; i <= i2; i++) {
                            if (grid.cellActive(i, j, k)) { // the cell itself needs to be active
                                if (allow_aquifer_inside_reservoir
                                    || !neighborCellInsideReservoirAndActive(grid, i, j, k, faceDir)) {
                                    std::pair<bool, double> influx_coeff = std::make_pair(false, 0);
                                    auto global_index = grid.getGlobalIndex(i, j, k);
                                    if (aquanconRecord.getItem("INFLUX_COEFF").hasValue(0))
                                        influx_coeff = std::make_pair(
                                            true, aquanconRecord.getItem("INFLUX_COEFF").getSIDouble(0));

                                    AquancCell cell(aquiferID, global_index, influx_coeff, influx_mult, faceDir);
                                    add_cell(work, cell);
                                }
                            }
                        }
                    }
                }
            }
        }

        for (const auto& gi_cell : work) {
            const auto& cell = gi_cell.second;

            this->cells[cell.aquiferID].emplace_back(std::move(cell));
        }
    }


    Aquancon Aquancon::serializeObject()
    {
        Aquancon result;
        result.cells = {{1, {{2, 3, {true, 4.0}, 5.0, FaceDir::XPlus}}}};

        return result;
    }


    const std::vector<Aquancon::AquancCell> Aquancon::operator[](int aquiferID) const {
        return this->cells.at( aquiferID );
    }



    bool Aquancon::cellInsideReservoirAndActive(const Opm::EclipseGrid& grid, const int i, const int j, const int k)
    {
        if ( i < 0 || j < 0 || k < 0
            || size_t(i) > grid.getNX() - 1
            || size_t(j) > grid.getNY() - 1
            || size_t(k) > grid.getNZ() - 1 )
        {
            return false;
        }

        return grid.cellActive(i, j, k );
    }

    bool Aquancon::neighborCellInsideReservoirAndActive(const Opm::EclipseGrid& grid,
           const int i, const int j, const int k, const Opm::FaceDir::DirEnum faceDir)
    {
        switch(faceDir) {
        case FaceDir::XMinus:
            return cellInsideReservoirAndActive(grid, i - 1, j, k);
        case FaceDir::XPlus:
            return cellInsideReservoirAndActive(grid, i + 1, j, k);
        case FaceDir::YMinus:
            return cellInsideReservoirAndActive(grid, i, j - 1, k);
        case FaceDir::YPlus:
            return cellInsideReservoirAndActive(grid, i, j + 1, k);
        case FaceDir::ZMinus:
            return cellInsideReservoirAndActive(grid, i, j, k - 1);
        case FaceDir::ZPlus:
            return cellInsideReservoirAndActive(grid, i, j, k + 1);
        default:
            throw std::runtime_error("Unknown FaceDir enum " + std::to_string(faceDir));
        }
    }

    Aquancon::Aquancon(const std::unordered_map<int, std::vector<Aquancon::AquancCell>>& data) :
        cells(data)
    {}

    const std::unordered_map<int, std::vector<Aquancon::AquancCell>>& Aquancon::data() const {
        return this->cells;
    }

    bool Aquancon::operator==(const Aquancon& other) const {
        return this->cells == other.cells;
    }


    bool Aquancon::active() const {
        return !this->cells.empty();
    }

}
