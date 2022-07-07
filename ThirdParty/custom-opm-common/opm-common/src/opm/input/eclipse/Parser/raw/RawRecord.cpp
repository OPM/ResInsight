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
#include <iostream>
#include <stdexcept>
#include <vector>
#include <deque>

#include <fmt/format.h>
#include <opm/common/OpmLog/KeywordLocation.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include "RawRecord.hpp"
#include "RawConsts.hpp"


using namespace Opm;
using namespace std;

namespace Opm {

namespace {

std::deque< std::string_view > splitSingleRecordString( const std::string_view& record ) {
    auto first_nonspace = []( std::string_view::const_iterator begin,
                              std::string_view::const_iterator end ) {
        return std::find_if_not( begin, end, RawConsts::is_separator() );
    };

    std::deque< std::string_view > dst;
    auto current = record.begin();
    while( (current = first_nonspace( current, record.end() )) != record.end() )
    {
        if( *current == RawConsts::quote ) {
            auto quote_end = std::find( current + 1, record.end(), RawConsts::quote ) + 1;
            std::size_t size = std::distance(current, quote_end);
            dst.push_back( { &*current, size} );
            current = quote_end;
        } else {
            auto token_end = std::find_if( current, record.end(), RawConsts::is_separator() );
            std::size_t size = std::distance(current, token_end);
            dst.push_back( { &*current, size } );
            current = token_end;
        }
    }

    return dst;
}

/*
    * It is assumed that after a record is terminated, there is no quote marks
    * in the subsequent comment. This is in accordance with the Eclipse user
    * manual.
    *
    * If a "non-complete" record string is supplied, an invalid_argument
    * exception is thrown.
    *
    */

template< typename T >
inline bool even_quotes( const T& str ) {
    return std::count( str.begin(), str.end(), RawConsts::quote ) % 2 == 0;
}

}

    RawRecord::RawRecord(const std::string_view& singleRecordString, const KeywordLocation& location, bool text) :
        m_sanitizedRecordString( singleRecordString )
    {

        if (text)
            this->m_recordItems.push_back(this->m_sanitizedRecordString);
        else {
            this->m_recordItems = splitSingleRecordString( m_sanitizedRecordString );

            if( !even_quotes( singleRecordString ) ) {
                std::string error = fmt::format("Quotes are not balanced in: \"{}\"", std::string(singleRecordString));
                throw OpmInputError(error, location);
            }
        }
        this->m_max_size = this->m_recordItems.size();
    }

    RawRecord::RawRecord(const std::string_view& singleRecordString, const KeywordLocation& location) :
        RawRecord(singleRecordString, location, false)
    {}

    void RawRecord::push_front( std::string_view tok, std::size_t count ) {
        this->m_recordItems.insert( this->m_recordItems.begin(), count, tok );
        this->m_max_size += count;
    }

    std::string RawRecord::getRecordString() const {
        return std::string(m_sanitizedRecordString);
    }

    std::size_t RawRecord::max_size() const {
        return this->m_max_size;
    }
}
