/*
  Copyright 2021 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <functional>

#include <opm/input/eclipse/Deck/DeckView.hpp>


void Opm::DeckView::add_keyword(const Opm::DeckKeyword& kw) {
    this->keyword_index[kw.name()].push_back(this->keywords.size());
    this->keywords.push_back(std::cref(kw));
}

bool Opm::DeckView::has_keyword(const std::string& kw) const {
    return this->keyword_index.find(kw) != this->keyword_index.end();
}

bool Opm::DeckView::empty() const {
    return this->keywords.empty();
}

std::size_t Opm::DeckView::size() const {
    return this->keywords.size();
}

const Opm::DeckKeyword& Opm::DeckView::operator[](std::size_t kw_index) const {
    return this->keywords.at(kw_index).get();
}

Opm::DeckView Opm::DeckView::operator[](const std::string& kw_name) const {
    DeckView dw;
    auto iter = this->keyword_index.find(kw_name);
    if (iter != this->keyword_index.end()) {
        for (const auto& kw_index : iter->second) {
            const auto& kw = this->keywords[kw_index].get();
            dw.add_keyword(kw);
        }
    }
    return dw;
}

const Opm::DeckKeyword& Opm::DeckView::front() const {
    if (this->empty())
        throw std::logic_error("Tried to get front() from empty DeckView");

    return this->keywords.front().get();
}

const Opm::DeckKeyword& Opm::DeckView::back() const {
    if (this->empty())
        throw std::logic_error("Tried to get back() from empty DeckView");

    return this->keywords.back().get();
}

std::vector<std::size_t> Opm::DeckView::index(const std::string& keyword) const {
    auto iter = this->keyword_index.find(keyword);
    if (iter != this->keyword_index.end())
        return iter->second;

    return {};
}

std::size_t Opm::DeckView::count(const std::string& keyword) const {
    auto iter = this->keyword_index.find(keyword);
    if (iter == this->keyword_index.end())
        return 0;

    return iter->second.size();
}
