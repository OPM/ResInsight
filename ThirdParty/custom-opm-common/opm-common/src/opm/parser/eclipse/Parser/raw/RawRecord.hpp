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

#ifndef RECORD_HPP
#define RECORD_HPP

#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <list>

namespace Opm {

    /// Class representing the lowest level of the Raw datatypes, a record. A record is simply
    /// a vector containing the record elements, represented as strings. Some logic is present
    /// to handle special elements in a record string, particularly with quote characters.

    class RawRecord {
    public:
        RawRecord( const std::string_view&, bool text);
        explicit RawRecord( const std::string_view&);

        inline std::string_view pop_front();
        inline std::string_view front() const;
        void push_front( std::string_view token );
        void prepend( size_t count, std::string_view token );
        inline size_t size() const;

        std::string getRecordString() const;
        inline std::string_view getItem(size_t index) const;

    private:
        std::string_view m_sanitizedRecordString;
        std::deque< std::string_view > m_recordItems;
    };

    /*
     * These are frequently called, but fairly trivial in implementation, and
     * inlining the calls gives a decent low-effort performance benefit.
     */
    std::string_view RawRecord::pop_front() {
        auto front = m_recordItems.front();
        this->m_recordItems.pop_front();
        return front;
    }

    std::string_view RawRecord::front() const {
        return this->m_recordItems.front();
    }

    size_t RawRecord::size() const {
        return m_recordItems.size();
    }

    std::string_view RawRecord::getItem(size_t index) const {
        return this->m_recordItems.at( index );
    }
}

#endif  /* RECORD_HPP */

