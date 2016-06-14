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

#include <opm/parser/eclipse/RawDeck/RawRecord.hpp>
#include <opm/parser/eclipse/RawDeck/RawConsts.hpp>

#include <opm/parser/eclipse/Utility/Stringview.hpp>

using namespace Opm;
using namespace std;

namespace Opm {

    template< typename Itr >
    static inline Itr first_nonspace( Itr begin, Itr end ) {
        return std::find_if_not( begin, end, RawConsts::is_separator );
    }

    static std::deque< string_view > splitSingleRecordString( const string_view& line ) {

        std::deque< string_view > dst;
        string_view record( line );

        for( auto current = first_nonspace( record.begin(), record.end() );
                current != record.end();
                current = first_nonspace( current, record.end() ) )
        {
            if( *current == RawConsts::quote ) {
                auto quote_end = std::find( current + 1, record.end(), RawConsts::quote ) + 1;
                dst.push_back( { current, quote_end } );
                current = quote_end;
            } else {
                auto token_end = std::find_if( current, record.end(), RawConsts::is_separator );
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
    static inline bool even_quotes( const T& str ) {
        return std::count( str.begin(), str.end(), RawConsts::quote ) % 2 == 0;
    }

    RawRecord::RawRecord(const string_view& singleRecordString,
                         const std::string& fileName,
                         const std::string& keywordName) :
        m_sanitizedRecordString( singleRecordString ),
        m_recordItems( splitSingleRecordString( m_sanitizedRecordString ) ),
        m_fileName(fileName),
        m_keywordName(keywordName)
    {

        if( !even_quotes( singleRecordString ) )
            throw std::invalid_argument(
                "Input string is not a complete record string, "
                "offending string: '" + singleRecordString + "'"
            );
    }

    const std::string& RawRecord::getFileName() const {
        return m_fileName;
    }

    const std::string& RawRecord::getKeywordName() const {
        return m_keywordName;
    }

    void RawRecord::push_front(std::string tok ) {
        this->expanded_items.push_back( tok );
        this->m_recordItems.emplace_front( this->expanded_items.back() );
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

    bool RawRecord::isTerminatedRecordString( const string_view& str ) {
        return str.back() == RawConsts::slash;
    }
}
