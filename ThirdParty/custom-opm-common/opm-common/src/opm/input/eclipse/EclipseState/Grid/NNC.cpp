/*
  Copyright 2015 IRIS

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
#include <array>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/N.hpp>

namespace Opm
{

namespace {

std::optional<std::size_t> global_index(const EclipseGrid& grid, const DeckRecord& record, std::size_t item_offset) {
    std::size_t i = static_cast<size_t>(record.getItem(0 + item_offset).get< int >(0)-1);
    std::size_t j = static_cast<size_t>(record.getItem(1 + item_offset).get< int >(0)-1);
    std::size_t k = static_cast<size_t>(record.getItem(2 + item_offset).get< int >(0)-1);

    if (i >= grid.getNX())
        return {};

    if (j >= grid.getNY())
        return {};

    if (k >= grid.getNZ())
        return {};

    if (!grid.cellActive(i,j,k))
        return {};

    return grid.getGlobalIndex(i,j,k);
}


std::optional<std::pair<std::size_t, std::size_t>> make_index_pair(const EclipseGrid& grid, const DeckRecord& record) {
    auto g1 = global_index(grid, record, 0);
    auto g2 = global_index(grid, record, 3);
    if (!g1)
        return {};

    if (!g2)
        return {};

    if (*g1 < *g2)
        return std::make_pair(*g1, *g2);
    else
        return std::make_pair(*g2, *g1);
}

bool is_neighbor(const EclipseGrid& grid, std::size_t g1, std::size_t g2) {
    auto diff = g2 - g1;
    if (diff == 0)
        return true;

    if (diff == 1)
        return true;

    if (diff == grid.getNX())
        return true;

    if (diff == grid.getNY())
        return true;

    return false;
}

}

    NNC::NNC(const EclipseGrid& grid, const Deck& deck) {
        this->load_input(grid, deck);
        this->load_edit(grid, deck);
    }


    void NNC::load_input(const EclipseGrid& grid, const Deck& deck) {
        for (const auto& keyword_ptr : deck.getKeywordList<ParserKeywords::NNC>()) {
            for (const auto& record : *keyword_ptr) {
                auto index_pair = make_index_pair(grid, record);
                if (!index_pair)
                    continue;

                auto [g1, g2] = index_pair.value();
                double trans = record.getItem(6).getSIDouble(0);
                this->m_input.emplace_back(g1, g2, trans);
            }

            if (!this->m_nnc_location)
                this->m_nnc_location = keyword_ptr->location();
        }

        std::sort(this->m_input.begin(), this->m_input.end());
    }


    void NNC::load_edit(const EclipseGrid& grid, const Deck& deck) {
        std::vector<NNCdata> nnc_edit;
        for (const auto& keyword_ptr : deck.getKeywordList<ParserKeywords::EDITNNC>()) {
            for (const auto& record : *keyword_ptr) {
                double tran_mult = record.getItem(6).get<double>(0);
                if (tran_mult == 1.0)
                    continue;

                auto index_pair = make_index_pair(grid, record);
                if (!index_pair)
                    continue;

                auto [g1, g2] = index_pair.value();
                if (is_neighbor(grid, g1, g2))
                    continue;

                nnc_edit.emplace_back( g1, g2, tran_mult);
            }

            if (!this->m_edit_location)
                this->m_edit_location = keyword_ptr->location();
        }

        std::sort(nnc_edit.begin(), nnc_edit.end());

        auto current_input = this->m_input.begin();
        for (const auto& current_edit : nnc_edit) {
            if (current_input == this->m_input.end()) {
                this->add_edit(current_edit);
                continue;
            }

            if (current_input->cell1 != current_edit.cell1 || current_input->cell2 != current_edit.cell2) {
                current_input = std::lower_bound(this->m_input.begin(),
                                                 this->m_input.end(),
                                                 NNCdata(current_edit.cell1, current_edit.cell2, 0));
                if (current_input == this->m_input.end()) {
                    this->add_edit(current_edit);
                    continue;
                }

            }

            bool edit_processed = false;
            while (current_input != this->m_input.end()
                   && current_input->cell1 == current_edit.cell1
                   && current_input->cell2 == current_edit.cell2)
            {
                current_input->trans *= current_edit.trans;
                ++current_input;
                edit_processed = true;
            }

            if (!edit_processed)
                this->add_edit(current_edit);
        }
    }


    NNC NNC::serializeObject()
    {
        NNC result;
        result.m_input= {{1,2,1.0},{2,3,2.0}};
        result.m_edit= {{1,2,1.0},{2,3,2.0}};
        result.m_nnc_location = {"NNC?", "File", 123};
        result.m_edit_location = {"EDITNNC?", "File", 123};
        return result;
    }

    bool NNC::addNNC(const size_t cell1, const size_t cell2, const double trans) {
        if (cell1 > cell2)
            return this->addNNC(cell2, cell1, trans);

        auto nnc = NNCdata(cell1, cell2, trans);
        auto insert_iter = std::lower_bound(this->m_input.begin(), this->m_input.end(), nnc);
        this->m_input.insert( insert_iter, nnc);
        return true;
    }

    bool NNC::operator==(const NNC& data) const {
        return m_input == data.m_input &&
               m_edit == data.m_edit &&
               m_edit_location == data.m_edit_location &&
               m_nnc_location == data.m_nnc_location;

    }

    void NNC::add_edit(const NNCdata& edit_node) {
        if (!this->m_edit.empty()) {
            auto& back = this->m_edit.back();
            if (back.cell1 == edit_node.cell1 && back.cell2 == edit_node.cell2) {
                back.trans *= edit_node.trans;
                return;
            }
        }

        this->m_edit.push_back(edit_node);
    }



   /*
     In principle we can have multiple NNC keywords, and to provide a good error
     message we should be able to return the location of the offending NNC. That
     will require some bookkeeping of which NNC originated in which NNC
     keyword/location. For now we just return the location of the first NNC
     keyword, but we should be ready for a more elaborate implementation without
     any API change.
    */
    KeywordLocation NNC::input_location([[maybe_unused]] const NNCdata& nnc) const {
        if (this->m_nnc_location)
            return *this->m_nnc_location;
        else
            return {};
    }


    KeywordLocation NNC::edit_location([[maybe_unused]] const NNCdata& nnc) const {
        if (this->m_edit_location)
            return *this->m_edit_location;
        else
            return {};
    }

} // namespace Opm

