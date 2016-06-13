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
#include <list>

#include <opm/parser/eclipse/Utility/Stringview.hpp>

namespace Opm {

    /// Class representing the lowest level of the Raw datatypes, a record. A record is simply
    /// a vector containing the record elements, represented as strings. Some logic is present
    /// to handle special elements in a record string, particularly with quote characters.

    class RawRecord {
    public:
        RawRecord( const string_view&, const std::string& fileName = "", const std::string& keywordName = "");

        inline string_view pop_front();
        void push_front(std::string token);
        inline size_t size() const;

        std::string getRecordString() const;
        inline string_view getItem(size_t index) const;
        const std::string& getFileName() const;
        const std::string& getKeywordName() const;

        static bool isTerminatedRecordString( const string_view& );

       void dump() const;

    private:
        string_view m_sanitizedRecordString;
        std::deque< string_view > m_recordItems;
        std::list< std::string > expanded_items;
        const std::string m_fileName;
        const std::string m_keywordName;

        void setRecordString(const std::string& singleRecordString);
    };
    typedef std::shared_ptr<RawRecord> RawRecordPtr;
    typedef std::shared_ptr<const RawRecord> RawRecordConstPtr;

    /*
     * These are frequently called, but fairly trivial in implementation, and
     * inlining the calls gives a decent low-effort performance benefit.
     */
    string_view RawRecord::pop_front() {
        auto front = m_recordItems.front();
        this->m_recordItems.pop_front();
        return front;
    }

    size_t RawRecord::size() const {
        return m_recordItems.size();
    }

    string_view RawRecord::getItem(size_t index) const {
        return this->m_recordItems.at( index );
    }
}

#endif  /* RECORD_HPP */

