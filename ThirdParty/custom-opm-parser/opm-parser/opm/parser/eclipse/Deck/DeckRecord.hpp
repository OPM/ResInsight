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

#ifndef DECKRECORD_HPP
#define DECKRECORD_HPP

#include <string>
#include <vector>
#include <memory>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>

namespace Opm {

    class DeckRecord {
    public:
        typedef std::vector< DeckItem >::const_iterator const_iterator;

        DeckRecord() = default;
        DeckRecord( size_t );

        size_t size() const;
        void addItem( DeckItem&& deckItem );

        DeckItem& getItem( size_t index );
        DeckItem& getItem( const std::string& name );
        DeckItem& getDataItem();

        const DeckItem& getItem( size_t index ) const;
        const DeckItem& getItem( const std::string& name ) const;
        const DeckItem& getDataItem() const;

        bool hasItem(const std::string& name) const;
        
        template <class Item>
        DeckItem& getItem() {
            return getItem( Item::itemName );
        }

        template <class Item>
        const DeckItem& getItem() const {
            return getItem( Item::itemName );
        }

        const_iterator begin() const;
        const_iterator end() const;

    private:
        std::vector< DeckItem > m_items;

    };

}
#endif  /* DECKRECORD_HPP */

