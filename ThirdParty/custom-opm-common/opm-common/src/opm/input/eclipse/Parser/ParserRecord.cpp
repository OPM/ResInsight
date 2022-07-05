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

#include <fmt/format.h>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/common/OpmLog/KeywordLocation.hpp>

#include "raw/RawRecord.hpp"

namespace Opm {

namespace {
    struct name_eq {
        name_eq( const std::string& x ) : name( x ) {}
        const std::string& name;
        bool operator()( const ParserItem& x ) const {
            return x.name() == this->name;
        }
    };
}

    ParserRecord::ParserRecord()
        : m_dataRecord( false )
    {
    }

    size_t ParserRecord::size() const {
        return m_items.size();
    }

    bool ParserRecord::rawStringRecord() const {
        return this->raw_string_record;
    }

    void ParserRecord::addItem( ParserItem item ) {
        if (m_dataRecord)
            throw std::invalid_argument("Record is already marked as DataRecord - can not add items");

        auto itr = std::find_if( this->m_items.begin(),
                                 this->m_items.end(),
                                 name_eq( item.name() ) );

        if( itr != this->m_items.end() )
            throw std::invalid_argument("Itemname: " + item.name() + " already exists.");

        if (item.parseRaw())
            this->raw_string_record = true;

        this->m_items.push_back( std::move( item ) );
    }

    void ParserRecord::addDataItem( ParserItem item ) {
        if (m_items.size() > 0)
            throw std::invalid_argument("Record already contains items - can not add Data Item");

        this->addItem( std::move( item) );
        m_dataRecord = true;
    }



    std::vector< ParserItem >::const_iterator ParserRecord::begin() const {
        return m_items.begin();
    }


    std::vector< ParserItem >::const_iterator ParserRecord::end() const {
        return m_items.end();
    }


    bool ParserRecord::hasDimension() const {
        return std::any_of( this->begin(), this->end(),
                            []( const ParserItem& x ) { return x.dimensions().size() > 0; } );
    }




    const ParserItem& ParserRecord::get(size_t index) const {
        return this->m_items.at( index );
    }

    bool ParserRecord::hasItem( const std::string& name ) const {
        return std::any_of( this->m_items.begin(),
                            this->m_items.end(),
                            name_eq( name ) );
    }

    const ParserItem& ParserRecord::get( const std::string& name ) const {
        auto itr = std::find_if( this->m_items.begin(),
                                 this->m_items.end(),
                                 name_eq( name ) );

        if( itr == this->m_items.end() )
            throw std::out_of_range( "No item '" + name + "'" );

        return *itr;
    }

    DeckRecord ParserRecord::parse(const ParseContext& parseContext , ErrorGuard& errors , RawRecord& rawRecord, UnitSystem& active_unitsystem, UnitSystem& default_unitsystem, const KeywordLocation& location) const {
        std::vector< DeckItem > items;
        items.reserve( this->size() );
        for( const auto& parserItem : *this )
            items.emplace_back( parserItem.scan( rawRecord, active_unitsystem, default_unitsystem ) );

        if (rawRecord.size() > 0) {
            std::string msg_format = fmt::format("Record contains too many items in keyword {{0}}. Expected {} items, found {}.\n", this->size(), rawRecord.max_size()) +
                                                 "In file {1} at line {2}.\n" +
                                     fmt::format("Record is \"{}\".", rawRecord.getRecordString());
            parseContext.handleError(ParseContext::PARSE_EXTRA_DATA , msg_format, location, errors);
        }

        return { std::move( items ), false };
    }

    bool ParserRecord::equal(const ParserRecord& other) const {
        bool equal_ = true;
        if (size() == other.size()) {
           size_t itemIndex = 0;
           while (true) {
               if (itemIndex == size())
                   break;
               {
                   const auto& item = get(itemIndex);
                   const auto& otherItem = other.get(itemIndex);

                   if (item != otherItem ) {
                       equal_ = false;
                       break;
                   }
               }
               itemIndex++;
            }
        } else
            equal_ = false;
        return equal_;
    }

    bool ParserRecord::isDataRecord() const {
        return m_dataRecord;
    }

    bool ParserRecord::operator==( const ParserRecord& rhs ) const {
        return this->equal( rhs );
    }

    bool ParserRecord::operator!=( const ParserRecord& rhs ) const {
        return !( *this == rhs );
    }

    std::ostream& operator<<( std::ostream& stream, const ParserRecord& rec ) {
        stream << "  ParserRecord { " << std::endl;

        for( const auto& item : rec )
            stream << "      " << item << std::endl;

        return stream << "    }";
    }


    const std::string& ParserRecord::end_string() const {
        return this->record_end;
    }

}
