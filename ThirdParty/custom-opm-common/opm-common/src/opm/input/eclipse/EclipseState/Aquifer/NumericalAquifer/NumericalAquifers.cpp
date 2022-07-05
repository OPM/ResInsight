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

#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/NumericalAquifers.hpp>

#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/NumericalAquiferCell.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/SingleNumericalAquifer.hpp>

#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>

#include <fmt/format.h>

#include <cstddef>
#include <unordered_set>

namespace Opm {

    NumericalAquifers::NumericalAquifers(const Deck& deck, const EclipseGrid& grid,
                                         const FieldPropsManager& field_props)
    {
        using AQUNUM=ParserKeywords::AQUNUM;
        if ( !deck.hasKeyword<AQUNUM>() ) return;

        std::unordered_set<std::size_t> cells;
        // there might be multiple keywords of keyword AQUNUM, it is not totally
        // clear about the rules here. For now, we take care of all the keywords
        const auto& aqunum_keywords = deck.getKeywordList<AQUNUM>();
        for (const auto& keyword : aqunum_keywords) {
            for (const auto& record : *keyword) {
                const NumericalAquiferCell aqu_cell(this->m_num_records++, record, grid, field_props);
                if (cells.count(aqu_cell.global_index) > 0) {
                    auto error = fmt::format("Numerical aquifer cell at ({}, {}, {}) is declared more than once",
                                             aqu_cell.I + 1, aqu_cell.J + 1, aqu_cell.K + 1);
                    throw OpmInputError(error, keyword->location());
                } else {
                    this->addAquiferCell(aqu_cell);
                    cells.insert(aqu_cell.global_index);
                }
            }
        }

        this->initConnections(deck, grid);
    }


    void NumericalAquifers::addAquiferCell(const NumericalAquiferCell& aqu_cell) {
        const size_t id = aqu_cell.aquifer_id;
        if (!this->hasAquifer(id)) {
            this->m_aquifers.insert(std::make_pair(id, SingleNumericalAquifer{id}));
        }

        auto& aquifer = this->m_aquifers.at(id);
        aquifer.addAquiferCell(aqu_cell);
    }

    bool NumericalAquifers::hasAquifer(const size_t aquifer_id) const {
        return (this->m_aquifers.find(aquifer_id) != this->m_aquifers.end());
    }

    void NumericalAquifers::
    initConnections(const Deck& deck, const EclipseGrid& grid) {
        const auto aquifer_connections = NumericalAquiferConnection::generateConnections(deck, grid);

        for (auto& pair : this->m_aquifers) {
            const size_t aqu_id = pair.first;
            const auto& aqu_cons = aquifer_connections.find(aqu_id);
            if (aqu_cons == aquifer_connections.end()) {
                const auto error = fmt::format("Numerical aquifer {} does not have any connections\n", aqu_id);
                throw std::runtime_error(error);
            }
            auto& aquifer = pair.second;
            const auto& cons = aqu_cons->second;

            const auto all_aquifer_cells = this->allAquiferCells();
            // For now, there is no two aquifers can be connected to one cell
            // aquifer can not connect to aquifer cells
            for (const auto& con : cons) {
                const auto& aqu_con = con.second;
                const size_t con_global_index = aqu_con.global_index;
                const auto cell_iter = all_aquifer_cells.find(con_global_index);
                if (cell_iter != all_aquifer_cells.end()) {
                    const size_t cell_aquifer_id = cell_iter->second->aquifer_id;
                    auto msg = fmt::format("Problem with keyword AQUCON \n"
                                           "Aquifer connection declared at grid cell ({}, {}, {}), is a aquifer cell "
                                           "of Aquifer {}, and will be removed",
                                           aqu_con.I + 1, aqu_con.J + 1, aqu_con.K + 1, cell_aquifer_id);
                    OpmLog::warning(msg);
                    continue;
                }

                aquifer.addAquiferConnection(con.second);
            }
        }
    }

    bool NumericalAquifers::operator==(const NumericalAquifers& other) const {
        return (this->m_aquifers == other.m_aquifers)
            && (this->m_num_records == other.m_num_records);
    }

    size_t NumericalAquifers::size() const {
        return this->m_aquifers.size();
    }

    NumericalAquifers NumericalAquifers::serializeObject() {
        NumericalAquifers result;
        result.m_aquifers  = {{1, SingleNumericalAquifer{1}}};
        return result;
    }

    const SingleNumericalAquifer& NumericalAquifers::getAquifer(const size_t aquifer_id) const {
        const auto iter = this->m_aquifers.find(aquifer_id);
        if ( iter != this->m_aquifers.end() ) {
            return iter->second;
        } else {
            const auto msg = fmt::format(" There is no numerical aquifer {}", aquifer_id);
            throw std::runtime_error(msg);
        }
    }

    std::unordered_map<size_t, const NumericalAquiferCell*> NumericalAquifers::allAquiferCells() const {
        std::unordered_map<size_t, const NumericalAquiferCell*> cells;
        for (const auto& [id, aquifer] : this->m_aquifers) {
            for (size_t i = 0; i < aquifer.numCells(); ++i) {
                const NumericalAquiferCell* cell_ptr = aquifer.getCellPrt(i);
                cells.insert(std::make_pair(cell_ptr->global_index, cell_ptr));
            }
        }
        return cells;
    }

    const std::map<size_t, SingleNumericalAquifer>& NumericalAquifers::aquifers() const {
        return this->m_aquifers;
    }

    std::unordered_map<size_t, AquiferCellProps> NumericalAquifers::aquiferCellProps() const {
        std::unordered_map<size_t, AquiferCellProps> cell_props;
        for ([[maybe_unused]]const auto& [id, aquifer] : this->m_aquifers ) {
            auto aqu_cell_props = aquifer.aquiferCellProps();
            cell_props.insert(aqu_cell_props.begin(), aqu_cell_props.end());
        }
        return cell_props;
    }

    std::unordered_map<size_t, double> NumericalAquifers::aquiferCellVolumes() const {
        std::unordered_map<size_t, double> cell_volumes;
        const auto aquifer_cells = this->allAquiferCells();
        for (const auto& [global_index, cell] : aquifer_cells) {
            cell_volumes.insert(std::make_pair(global_index, cell->cellVolume()));
        }
        return cell_volumes;
    }

    std::vector<NNCdata>
    NumericalAquifers::aquiferCellNNCs() const {
        std::vector<NNCdata> nncs;
        for ([[maybe_unused]] const auto& [id, aquifer] : this->m_aquifers) {
            auto aqu_nncs = aquifer.aquiferCellNNCs();
            nncs.insert( nncs.end(), std::make_move_iterator(aqu_nncs.begin()), std::make_move_iterator(aqu_nncs.end()));
        }
        return nncs;
    }

    std::vector<NNCdata>
    NumericalAquifers::aquiferConnectionNNCs(const EclipseGrid& grid, const FieldPropsManager& fp) const {
        std::vector<NNCdata> nncs;
        for ([[maybe_unused]] const auto& [id, aquifer] : this->m_aquifers) {
            auto aqu_nncs = aquifer.aquiferConnectionNNCs(grid, fp);
            nncs.insert( nncs.end(), std::make_move_iterator(aqu_nncs.begin()), std::make_move_iterator(aqu_nncs.end()));
        }
        return nncs;
    }

    void NumericalAquifers::postProcessConnections(const EclipseGrid& grid, const std::vector<int>& actnum) {
        for ([[maybe_unused]] auto& [id, aquifer] : this->m_aquifers) {
            aquifer.postProcessConnections(grid, actnum);
        }
    }
}
