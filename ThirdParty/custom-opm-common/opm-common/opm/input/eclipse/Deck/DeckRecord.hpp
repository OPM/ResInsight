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
#include <ostream>

#include <opm/input/eclipse/Deck/DeckItem.hpp>

namespace Opm {

    class DeckRecord {
    public:
        typedef std::vector< DeckItem >::const_iterator const_iterator;

        DeckRecord() = default;
        DeckRecord( std::vector< DeckItem >&& items, const bool check_for_duplicate_names = true );

        static DeckRecord serializeObject();

        size_t size() const;
        void addItem( DeckItem deckItem );

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

        void write(DeckOutput& writer, std::size_t item_offset = 0) const;
        void write_data(DeckOutput& writer, std::size_t item_offset = 0) const;
        friend std::ostream& operator<<(std::ostream& os, const DeckRecord& record);

        bool equal(const DeckRecord& other, bool cmp_default, bool cmp_numeric) const;
        bool operator==(const DeckRecord& other) const;
        bool operator!=(const DeckRecord& other) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer.vector(m_items);
        }

    private:
        std::vector< DeckItem > m_items;

    };

}
#endif  /* DECKRECORD_HPP */

