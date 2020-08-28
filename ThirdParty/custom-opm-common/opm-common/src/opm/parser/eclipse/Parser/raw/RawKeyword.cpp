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
#include <stdexcept>

#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/common/utility/String.hpp>

#include "RawConsts.hpp"
#include "RawKeyword.hpp"
#include "RawRecord.hpp"

namespace Opm {

namespace {

     std::string keyword_name(const std::string& input_name) {
         std::string name = rtrim_copy(input_name);
         if (!ParserKeyword::validDeckName(name))
            throw std::invalid_argument("Not a valid keyword:" + name);

         if (name.size() > Opm::RawConsts::maxKeywordLength)
            throw std::invalid_argument("Too long keyword:" + name);

         if (name[0] == ' ')
             throw std::invalid_argument("Illegal whitespace start of keyword:" + name);

         return name;
    }

}


    RawKeyword::RawKeyword(const std::string& name, const std::string& filename, std::size_t lineNR, bool raw_string, Raw::KeywordSizeEnum sizeType, std::size_t size_arg) :
        m_name(keyword_name(name)),
        m_location(filename, lineNR),
        raw_string_keyword(raw_string),
        m_sizeType(sizeType)
    {
        if (this->m_sizeType == Raw::FIXED) {
            this->m_fixedSize = size_arg;
            if (size_arg == 0)
                this->m_isFinished = true;
        }

        if (this->m_sizeType == Raw::TABLE_COLLECTION) {
            if (size_arg == 0)
                throw std::logic_error("Bug in opm/flow: Attempt to create a TableCollection with zero tables. Keyword: " + name + " at " + filename + ":" + std::to_string(lineNR));
            this->m_numTables = size_arg;
        }

        if (this->m_sizeType == Raw::SLASH_TERMINATED || this->m_sizeType == Raw::UNKNOWN) {
            if (size_arg != 0)
                throw std::logic_error("Bug in opm/flow: Must have size_arg == 0 for SLASH_TEMINATED and UNKNOWN. Keyword: " + name + " at " + filename + ":" + std::to_string(lineNR));
        }


        if (this->m_sizeType == Raw::CODE) {
            if (size_arg != 1)
                throw std::logic_error("Bug in opm/flow: Must have size_arg == 1 for CODE. Keyword: " + name + " at " + filename + ":" + std::to_string(lineNR));
            this->m_fixedSize = size_arg;
        }
    }


    RawKeyword::RawKeyword(const std::string& name, const std::string& filename, std::size_t lineNR, bool raw_string, Raw::KeywordSizeEnum sizeType) :
        RawKeyword(name, filename, lineNR, raw_string, sizeType, sizeType == Raw::CODE ? 1 : 0)
    {
        if (this->m_sizeType == Raw::FIXED || this->m_sizeType == Raw::TABLE_COLLECTION)
            throw std::logic_error("Internal error - wrong constructor has been used. Keyword: " + name + " at " + filename + ":" + std::to_string(lineNR));
    }


    const std::string& RawKeyword::getKeywordName() const {
        return m_name;
    }


    bool RawKeyword::terminateKeyword() {
        if (this->m_sizeType == Raw::SLASH_TERMINATED)
            this->m_isFinished = true;

        if (this->m_sizeType == Raw::DOUBLE_SLASH_TERMINATED) {
            if (m_isTempFinished)
                this->m_isFinished = true;
            else
                this->m_isTempFinished = true; 
        }

        if (m_sizeType == Raw::TABLE_COLLECTION) {
            m_currentNumTables += 1;
            if (m_currentNumTables == m_numTables)
                m_isFinished = true;
        }

        if( m_sizeType == Raw::UNKNOWN)
            m_isFinished = true;

        return this->m_isFinished;
    }


    bool RawKeyword::addRecord(RawRecord record) {

        if (record.size() > 0)
            m_isTempFinished = false;

        this->m_records.push_back(std::move(record));
        if (m_records.size() == this->m_fixedSize) {
            if( this->m_sizeType == Raw::FIXED || this->m_sizeType == Raw::CODE)
                this->m_isFinished = true;
        }
        return this->m_isFinished;
    }



    const RawRecord& RawKeyword::getFirstRecord() const {
        return *m_records.begin();
    }


    bool RawKeyword::isFinished() const {
        return m_isFinished;
    }

    const Location& RawKeyword::location() const {
        return this->m_location;
    }

    RawKeyword::const_iterator RawKeyword::begin() const {
        return this->m_records.begin();
    }

    RawKeyword::const_iterator RawKeyword::end() const {
        return this->m_records.end();
    }

    RawKeyword::iterator RawKeyword::begin() {
        return this->m_records.begin();
    }

    RawKeyword::iterator RawKeyword::end() {
        return this->m_records.end();
    }


    Raw::KeywordSizeEnum RawKeyword::getSizeType() const {
        return m_sizeType;
    }

    bool RawKeyword::rawStringKeyword() const {
        return this->raw_string_keyword;
    }

    std::size_t RawKeyword::size() const {
        return this->m_records.size();
    }

}

