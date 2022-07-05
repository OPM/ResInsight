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

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DECKVIEW_HPP
#define DECKVIEW_HPP

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <unordered_map>

namespace Opm {


class DeckView {
public:
    typedef std::vector<std::reference_wrapper<const DeckKeyword>> storage_type;


    struct Iterator : public storage_type::iterator {
        Iterator(storage_type::const_iterator inner_iter) :
            inner(inner_iter)
        {}

        const DeckKeyword& operator*()  { return this->inner->get(); }
        const DeckKeyword* operator->() { return &this->inner->get(); }

        Iterator& operator++()    { ++this->inner; return *this; }
        Iterator  operator++(int) { auto tmp = *this; ++this->inner; return tmp; }


        Iterator& operator--()    { --this->inner; return *this; }
        Iterator  operator--(int) { auto tmp = *this; --this->inner; return tmp; }

        Iterator::difference_type operator-(const Iterator &other) { return this->inner - other.inner; }
        Iterator operator+(Iterator::difference_type shift) { Iterator tmp = *this; tmp.inner += shift; return tmp;}

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.inner == b.inner; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.inner != b.inner; };
    private:
        storage_type::const_iterator inner;
    };

    Iterator begin() const { return Iterator(this->keywords.begin()); }
    Iterator end() const { return Iterator(this->keywords.end()); }

    const DeckKeyword& operator[](std::size_t index) const;
    DeckView operator[](const std::string& keyword) const;
    std::vector<std::size_t> index(const std::string& keyword) const;
    std::size_t count(const std::string& keyword) const;
    const DeckKeyword& front() const;
    const DeckKeyword& back() const;

    DeckView() = default;
    void add_keyword(const DeckKeyword& kw);
    bool has_keyword(const std::string& kw) const;
    bool empty() const;
    std::size_t size() const;

    template<class Keyword>
    bool has_keyword() const {
        return this->has_keyword( Keyword::keywordName );
    }

    template<class Keyword>
    DeckView get() const {
        return this->operator[](Keyword::keywordName);
    }

private:
    storage_type keywords;
    std::unordered_map<std::string, std::vector<std::size_t>> keyword_index;
};

}
#endif
