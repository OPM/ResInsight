/*
  Copyright 2013 Statoil ASA.

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

#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/G.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>

namespace Opm {


const std::unordered_map<std::string, std::size_t> section_index = {
    {"RUNSPEC", 0},
    {"GRID", 1},
    {"EDIT", 2},
    {"PROPS", 3},
    {"REGIONS", 4},
    {"SOLUTION", 5},
    {"SUMMARY", 6},
    {"SCHEDULE", 7}
};

    std::pair<std::size_t, std::size_t> index_pair(const Deck& deck, const std::string& section) {
        if (!deck.hasKeyword(section))
            return {0,0};

        auto start_index = deck.index(section).front();
        std::unordered_set<std::string> end_set;
        {
            auto this_section_index = section_index.at(section);

            for (const auto& [section_name, index] : section_index) {
                if (index > this_section_index)
                    end_set.insert(section_name);
            }
        }

        if (end_set.empty())
            return {start_index, deck.size()};

        std::size_t deck_index = start_index;
        while (true) {
            const auto& kw = deck[deck_index];
            if (end_set.count(kw.name()) == 1)
                break;

            deck_index++;

            if (deck_index == deck.size())
                break;
        }
        return {start_index, deck_index};
    }


    DeckSection::DeckSection( const Deck& deck, const std::string& section )
        : section_name( section )
        , units( deck.getActiveUnitSystem() )
    {
        auto [start_index, end_index] = index_pair(deck, section);
        for (std::size_t index = start_index; index < end_index; index++)
            this->add_keyword(deck[index]);
    }


    const std::string& DeckSection::name() const {
        return this->section_name;
    }

    const UnitSystem& DeckSection::unitSystem() const {
        return this->units;
    }

bool DeckSection::hasRUNSPEC(const Deck& deck) { return deck.hasKeyword<ParserKeywords::RUNSPEC>(); }
bool DeckSection::hasGRID(const Deck& deck) { return deck.hasKeyword<ParserKeywords::GRID>(); }
bool DeckSection::hasEDIT(const Deck& deck) { return deck.hasKeyword<ParserKeywords::EDIT>(); }
bool DeckSection::hasPROPS(const Deck& deck) { return deck.hasKeyword<ParserKeywords::PROPS>(); }
bool DeckSection::hasREGIONS(const Deck& deck) { return deck.hasKeyword<ParserKeywords::REGIONS>(); }
bool DeckSection::hasSOLUTION(const Deck& deck) { return deck.hasKeyword<ParserKeywords::SOLUTION>(); }
bool DeckSection::hasSUMMARY(const Deck& deck) { return deck.hasKeyword<ParserKeywords::SUMMARY>(); }
bool DeckSection::hasSCHEDULE(const Deck& deck) { return deck.hasKeyword<ParserKeywords::SCHEDULE>(); }

}
