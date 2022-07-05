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

#ifndef PARSERRECORD_HPP
#define PARSERRECORD_HPP

#include <iosfwd>
#include <vector>
#include <memory>

#include <opm/input/eclipse/Parser/ParserItem.hpp>

namespace Opm {

    class Deck;
    class DeckRecord;
    class ParseContext;
    class RawRecord;
    class ErrorGuard;
    class UnitSystem;
    class KeywordLocation;

    class ParserRecord {
    public:
        ParserRecord();
        size_t size() const;
        void addItem( ParserItem );
        void addDataItem( ParserItem item );
        const ParserItem& get(size_t index) const;
        const ParserItem& get(const std::string& itemName) const;
        DeckRecord parse( const ParseContext&, ErrorGuard&, RawRecord&, UnitSystem& active_unitsystem, UnitSystem& default_unitsystem, const KeywordLocation& location) const;
        bool isDataRecord() const;
        bool equal(const ParserRecord& other) const;
        bool hasDimension() const;
        bool hasItem(const std::string& itemName) const;
        std::vector< ParserItem >::const_iterator begin() const;
        std::vector< ParserItem >::const_iterator end() const;

        bool operator==( const ParserRecord& ) const;
        bool operator!=( const ParserRecord& ) const;
        bool rawStringRecord() const;
        const std::string& end_string() const;

    private:
        bool m_dataRecord;
        std::vector< ParserItem > m_items;
        bool raw_string_record = false;
        std::string record_end = "/";
    };

std::ostream& operator<<( std::ostream&, const ParserRecord& );
}


#endif  /* PARSERRECORD_HPP */

