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

#include <opm/input/eclipse/EclipseState/Aquifer/Aquancon.hpp>

#include <opm/io/eclipse/rst/aquifer.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <fmt/format.h>

#include "AquiferHelpers.hpp"

namespace {
    Opm::Aquancon::AquancCell
    makeAquiferCell(const int                                            aquiferID,
                    const Opm::RestartIO::RstAquifer::Connections::Cell& rst_cell)
    {
        return {
            aquiferID,
            rst_cell.global_index,
            rst_cell.influx_coeff,
            rst_cell.effective_facearea,
            rst_cell.face_dir
        };
    }

    std::vector<Opm::Aquancon::AquancCell>
    makeAquiferConnections(const int                                      aquiferID,
                           const Opm::RestartIO::RstAquifer::Connections& rst_connections)
    {
        auto connections = std::vector<Opm::Aquancon::AquancCell>{};
        const auto& rst_cells = rst_connections.cells();

        connections.reserve(rst_cells.size());
        for (const auto& rst_cell : rst_cells) {
            connections.push_back(makeAquiferCell(aquiferID, rst_cell));
        }

        return connections;
    }
} // Anonymous

namespace Opm {

    namespace{

        double face_area(FaceDir::DirEnum face_dir, std::size_t global_index, const EclipseGrid& grid) {
            const auto& dims = grid.getCellDims(global_index);
            switch (face_dir) {
            case FaceDir::XPlus:
            case FaceDir::XMinus:
                return dims[1] * dims[2];
            case FaceDir::YPlus:
            case FaceDir::YMinus:
                return dims[0] * dims[2];
            case FaceDir::ZPlus:
            case FaceDir::ZMinus:
                return dims[0] * dims[1];
            default:
                throw std::logic_error("What the f...");
            }
        }

        void add_cell(const KeywordLocation& location,
                      std::map<std::size_t, Aquancon::AquancCell>& work,
                      const EclipseGrid& grid,
                      const int aquiferID,
                      const std::size_t global_index,
                      std::optional<double> influx_coeff,
                      const double influx_mult,
                      const FaceDir::DirEnum face_dir)
        {
            const auto faceArea = face_area(face_dir, global_index, grid);
            auto cell_iter = work.find(global_index);
            if (cell_iter == work.end()) {
                if (!influx_coeff.has_value())
                    influx_coeff = faceArea;
                work.emplace(std::piecewise_construct,
                             std::forward_as_tuple(global_index),
                             std::forward_as_tuple(aquiferID, global_index,
                                                   influx_coeff.value() * influx_mult,
                                                   faceArea * influx_mult, face_dir));
            }
            else {
                auto& prev_cell = cell_iter->second;
                if (prev_cell.aquiferID == aquiferID) {
                    prev_cell.influx_coeff += influx_coeff.value_or(0.0);
                    prev_cell.influx_coeff *= influx_mult;
                    prev_cell.effective_facearea += faceArea;
                    prev_cell.effective_facearea *= influx_mult;
                } else {
                    auto [i,j,k] = grid.getIJK(global_index);
                    auto msg = fmt::format("Problem with AQUANCON keyword\n"
                                           "In {} line {}\n"
                                           "Cell ({}, {}, {}) is already connected to aquifer: {}", location.filename, location.lineno, i + 1, j + 1, k + 1, prev_cell.aquiferID);
                    throw std::invalid_argument( msg );
                }
            }
        }

    }


    Aquancon::Aquancon(const EclipseGrid& grid, const Deck& deck)
    {
        std::map<std::size_t, Aquancon::AquancCell> work;
        const std::vector<int>& actnum = grid.getACTNUM();
        for (std::size_t iaq = 0; iaq < deck.count("AQUANCON"); iaq++) {
            const auto& aquanconKeyword = deck["AQUANCON"][iaq];
            OpmLog::info(OpmInputError::format("Initializing aquifer connections from {keyword} in {file} line {line}", aquanconKeyword.location()));
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
                            if (actnum[grid.getGlobalIndex(i, j, k)]) { // the cell itself needs to be active
                                if (allow_aquifer_inside_reservoir
                                    || !AquiferHelpers::neighborCellInsideReservoirAndActive(grid, i, j, k, faceDir, actnum)) {
                                    std::optional<double> influx_coeff;
                                    if (aquanconRecord.getItem("INFLUX_COEFF").hasValue(0))
                                        influx_coeff = aquanconRecord.getItem("INFLUX_COEFF").getSIDouble(0);

                                    auto global_index = grid.getGlobalIndex(i,j,k);
                                    add_cell(aquanconKeyword.location(), work, grid, aquiferID, global_index, influx_coeff, influx_mult, faceDir);
                                }
                            } else {
                                const auto& location = aquanconKeyword.location();
                                auto msg = fmt::format("Problem with keyword {}\n"
                                                       "In {} line {} \n"
                                                       "Connection to inactive cell ({},{},{}) is ignored", location.keyword, location.filename, location.lineno, i+1, j+1, k+1);
                                OpmLog::warning(msg);
                            }
                        }
                    }
                }
            }
        }

        for (const auto& gi_cell : work) {
            auto cell = gi_cell.second;
            const auto aquiferID = cell.aquiferID;

            this->cells[aquiferID].emplace_back(std::move(cell));
        }
    }


    Aquancon Aquancon::serializeObject()
    {
        Aquancon result;
        result.cells = {{1, {{2, 3, 4.0, 5.0, FaceDir::XPlus}}}};

        return result;
    }


    const std::vector<Aquancon::AquancCell>& Aquancon::operator[](int aquiferID) const {
        const auto search = this->cells.find(aquiferID);
        if (search == this->cells.end()) {
            auto msg = fmt::format("There is no connection associated with analytical aquifer {}\n", aquiferID);
            throw std::runtime_error(msg);
        }
        return search->second;
    }

    Aquancon::Aquancon(const std::unordered_map<int, std::vector<Aquancon::AquancCell>>& data) :
        cells(data)
    {}

    void Aquancon::pruneDeactivatedAquiferConnections(const std::vector<std::size_t>& deactivated_cells)
    {
        const auto removed = std::unordered_set<std::size_t> {
            deactivated_cells.begin(), deactivated_cells.end()
        };

        for (auto& conns : this->cells) {
            auto end = std::remove_if(conns.second.begin(), conns.second.end(),
                [&removed](const AquancCell& cell) -> bool
            {
                return removed.find(cell.global_index) != removed.end();
            });

            conns.second.erase(end, conns.second.end());
        }
    }

    void Aquancon::loadFromRestart(const RestartIO::RstAquifer& rst_aquifers)
    {
        this->cells.clear();

        for (const auto& [aquiferID, rst_connections] : rst_aquifers.connections()) {
            this->cells.insert_or_assign(aquiferID, makeAquiferConnections(aquiferID, rst_connections));
        }
    }

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
