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

#ifndef RAWKEYWORD_HPP
#define RAWKEYWORD_HPP

#include <memory>
#include <string>
#include <vector>
#include <list>

#include <opm/parser/eclipse/RawDeck/RawEnums.hpp>
#include <opm/parser/eclipse/Utility/Stringview.hpp>

namespace Opm {

    class RawRecord;
    class string_view;

    /// Class representing a RawKeyword, meaning both the actual keyword phrase, and the records,
    /// represented as a list of RawRecord objects.
    /// The class also contains static functions to aid the parsing of the input file.
    /// The creating of an instance is performed by calling the addRawRecordString method repeatedly.

    class RawKeyword {
    public:
        RawKeyword(const string_view& name , Raw::KeywordSizeEnum sizeType , const std::string& filename, size_t lineNR);
        RawKeyword(const string_view& name , const std::string& filename, size_t lineNR , size_t inputSize , bool isTableCollection = false);

        const std::string& getKeywordName() const;
        void addRawRecordString( const string_view& );
        size_t size() const;
        Raw::KeywordSizeEnum getSizeType() const;

        // Special case method only for inspecting INCLUDE keywords;
        // the general getRecords functionality should use the
        // iterator interface.
        const RawRecord& getFirstRecord( ) const;

        static bool isKeywordPrefix(const string_view& line, std::string& keywordName);

        bool isPartialRecordStringEmpty() const;
        bool isFinished() const;
        bool unKnownSize() const;
        void finalizeUnknownSize();

        const std::string& getFilename() const;
        size_t getLineNR() const;

        using const_iterator = std::list< RawRecord >::const_iterator;
        using iterator = std::list< RawRecord >::iterator;

        const_iterator begin() const;
        const_iterator end() const;
        iterator begin();
        iterator end();

        bool is_title() const;

    private:
        Raw::KeywordSizeEnum m_sizeType;
        bool m_isFinished = false;
        size_t m_fixedSize;
        size_t m_numTables;
        size_t m_currentNumTables = 0;
        std::string m_name;
        std::list< RawRecord > m_records;
        string_view m_partialRecordString;

        size_t m_lineNR;
        std::string m_filename;
        bool m_is_title = false;

        void commonInit(const std::string& name,const std::string& filename, size_t lineNR);
        void setKeywordName(const std::string& keyword);
        static bool isValidKeyword(const std::string& keywordCandidate);
    };
    typedef std::shared_ptr<RawKeyword> RawKeywordPtr;
    typedef std::shared_ptr<const RawKeyword> RawKeywordConstPtr;

}
#endif  /* RAWKEYWORD_HPP */

