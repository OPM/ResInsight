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

#include <opm/parser/eclipse/Utility/Stringview.hpp>

#include "RawRecord.hpp"
#include "RawConsts.hpp"


using namespace Opm;
using namespace std;

namespace Opm {

namespace {

std::deque< string_view > splitSingleRecordString( const string_view& record ) {
    auto first_nonspace = []( string_view::const_iterator begin,
                              string_view::const_iterator end ) {
        return std::find_if_not( begin, end, RawConsts::is_separator() );
    };

    std::deque< string_view > dst;
    auto current = record.begin();
    while( (current = first_nonspace( current, record.end() )) != record.end() )
    {
        if( *current == RawConsts::quote ) {
            auto quote_end = std::find( current + 1, record.end(), RawConsts::quote ) + 1;
            dst.push_back( { current, quote_end } );
            current = quote_end;
        } else {
            auto token_end = std::find_if( current, record.end(), RawConsts::is_separator() );
            dst.push_back( { current, token_end } );
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

    RawRecord::RawRecord(const string_view& singleRecordString, bool text) :
        m_sanitizedRecordString( singleRecordString )
    {

        if (text)
            this->m_recordItems.push_back(this->m_sanitizedRecordString);
        else {
            this->m_recordItems = splitSingleRecordString( m_sanitizedRecordString );

            if( !even_quotes( singleRecordString ) )
                throw std::invalid_argument("Input string is not a complete record string, "
                                            "offending string: '" + singleRecordString + "'");
        }
    }

    RawRecord::RawRecord(const string_view& singleRecordString) :
        RawRecord(singleRecordString, false)
    {}

    void RawRecord::prepend( size_t count, string_view tok ) {
        this->m_recordItems.insert( this->m_recordItems.begin(), count, tok );
    }

    void RawRecord::dump() const {
        std::cout << "RecordDump: ";
        for (size_t i = 0; i < m_recordItems.size(); i++) {
            std::cout
                << this->m_recordItems[i] << "/"
                << getItem( i ) << " ";
        }
        std::cout << std::endl;
    }

    std::string RawRecord::getRecordString() const {
        return m_sanitizedRecordString.string();
    }
}
