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

#include "DeckKeyword.hpp"
#include "DeckRecord.hpp"
#include "DeckItem.hpp"

namespace Opm {

    DeckKeyword::DeckKeyword(const std::string& keywordName) {
        m_knownKeyword = true;
        m_keywordName = keywordName;
        m_isDataKeyword = false;
        m_fileName = "";
        m_lineNumber = -1;
    }

    DeckKeyword::DeckKeyword(const std::string& keywordName, bool knownKeyword) {
        m_knownKeyword = knownKeyword;
        m_keywordName = keywordName;
        m_isDataKeyword = false;
        m_fileName = "";
        m_lineNumber = -1;
    }

    void DeckKeyword::setLocation(const std::string& fileName, int lineNumber) {
        m_fileName = fileName;
        m_lineNumber = lineNumber;
    }

    const std::string& DeckKeyword::getFileName() const {
        return m_fileName;
    }

    int DeckKeyword::getLineNumber() const {
        return m_lineNumber;
    }

    void DeckKeyword::setDataKeyword(bool isDataKeyword_) {
        m_isDataKeyword = isDataKeyword_;
    }

    bool DeckKeyword::isDataKeyword() const {
        return m_isDataKeyword;
    }


    const std::string& DeckKeyword::name() const {
        return m_keywordName;
    }

    size_t DeckKeyword::size() const {
        return m_recordList.size();
    }

    bool DeckKeyword::isKnown() const {
        return m_knownKeyword;
    }

    void DeckKeyword::addRecord(DeckRecord&& record) {
        this->m_recordList.push_back( std::move( record ) );
    }

    DeckKeyword::const_iterator DeckKeyword::begin() const {
        return m_recordList.begin();
    }

    DeckKeyword::const_iterator DeckKeyword::end() const {
        return m_recordList.end();
    }

    const DeckRecord& DeckKeyword::getRecord(size_t index) const {
        return this->m_recordList.at( index );
    }

    DeckRecord& DeckKeyword::getRecord(size_t index) {
        return this->m_recordList.at( index );
    }

    const DeckRecord& DeckKeyword::getDataRecord() const {
        if (m_recordList.size() == 1)
            return getRecord(0);
        else
            throw std::range_error("Not a data keyword ?");
    }


    size_t DeckKeyword::getDataSize() const {
        return this->getDataRecord().getDataItem().size();
    }


    const std::vector<int>& DeckKeyword::getIntData() const {
        return this->getDataRecord().getDataItem().getData< int >();
    }


    const std::vector<std::string>& DeckKeyword::getStringData() const {
        return this->getDataRecord().getDataItem().getData< std::string >();
    }


    const std::vector<double>& DeckKeyword::getRawDoubleData() const {
        return this->getDataRecord().getDataItem().getData< double >();
    }

    const std::vector<double>& DeckKeyword::getSIDoubleData() const {
        return this->getDataRecord().getDataItem().getSIDoubleData();
    }

}

