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
#include <iostream>

#include <opm/input/eclipse/Utility/Typetools.hpp>

#include <opm/input/eclipse/Parser/ParserKeyword.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Deck/DeckOutput.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckValue.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>


namespace Opm {

    DeckKeyword::DeckKeyword(const ParserKeyword& parserKeyword) :
        m_keywordName(parserKeyword.getName()),
        m_isDataKeyword(false),
        m_slashTerminated(!parserKeyword.hasFixedSize())
    {
    }

    DeckKeyword::DeckKeyword(const KeywordLocation& location, const std::string& keywordName) :
        m_keywordName(keywordName),
        m_location(location),
        m_isDataKeyword(false),
        m_slashTerminated(true)
    {
    }

    DeckKeyword::DeckKeyword() :
        m_isDataKeyword(false),
        m_slashTerminated(false)
    {
    }

    DeckKeyword DeckKeyword::serializeObject()
    {
        DeckKeyword result;
        result.m_keywordName = "test";
        result.m_location = KeywordLocation::serializeObject();
        result.m_recordList = {DeckRecord::serializeObject()};
        result.m_isDataKeyword = true;
        result.m_slashTerminated = true;
        result.m_isDoubleRecordKeyword = true;

        return result;
    }

    namespace {
    template <typename T>
    void add_deckvalue( DeckItem deck_item, DeckRecord& deck_record, const ParserItem& parser_item, const std::vector<DeckValue>& input_record, size_t j) {
         if (j >= input_record.size() || input_record[j].is_default()) {
              if (parser_item.hasDefault())
                  deck_item.push_backDefault( parser_item.getDefault<T>() );
              else
                  deck_item.push_backDummyDefault<T>();
         }
         else if (input_record[j].is_compatible<T>())
             deck_item.push_back( input_record[j].get<T>() );
         else
             throw std::invalid_argument("For input to DeckKeyword '" + deck_item.name() +
                                          ", item '" + parser_item.name() +
                                          "': wrong type.");
         deck_record.addItem( std::move(deck_item) );
    }
    }


    DeckKeyword::DeckKeyword(const ParserKeyword& parserKeyword,  const std::vector<std::vector<DeckValue>>& record_list, const UnitSystem& system_active, const UnitSystem& system_default) :
        DeckKeyword(parserKeyword)
    {
        if (parserKeyword.hasFixedSize() && (record_list.size() != parserKeyword.getFixedSize()))
             throw std::invalid_argument("Wrong number of records added to constructor for deckkeyword '" + name() + "'.");

        for (size_t i = 0; i < record_list.size(); i++) {

             const ParserRecord& parser_record = parserKeyword.getRecord(i);
             const std::vector<DeckValue>& input_record = record_list[i];
             DeckRecord deck_record;

             for (size_t j = 0; j < parser_record.size(); j++) {
                  const ParserItem& parser_item = parser_record.get(j);
                  if (parser_item.sizeType() == ParserItem::item_size::ALL)
                      throw std::invalid_argument("constructor  DeckKeyword::DeckKeyword(const ParserKeyword&,  const std::vector<std::vector<DeckValue>>&) does not handle sizetype ALL.");

                  switch( parser_item.dataType() ) {
                      case type_tag::integer:
                          {
                              DeckItem deck_item(parser_item.name(), int());
                              add_deckvalue<int>(std::move(deck_item), deck_record, parser_item, input_record, j);
                          }
                          break;
                      case type_tag::fdouble:
                          {
                              auto& dim = parser_item.dimensions();
                              std::vector<Dimension> active_dimensions;
                              std::vector<Dimension> default_dimensions;
                              if (dim.size() > 0) {
                                 active_dimensions.push_back( system_active.parse(dim[0]) );
                                 default_dimensions.push_back( system_default.parse(dim[0]) );
                              }
                              DeckItem deck_item(parser_item.name(), double(), active_dimensions, default_dimensions);
                              add_deckvalue<double>(std::move(deck_item), deck_record, parser_item, input_record, j);
                          }
                          break;
                      case type_tag::string:
                          {
                              DeckItem deck_item(parser_item.name(), std::string());
                              add_deckvalue<std::string>(std::move(deck_item), deck_record, parser_item, input_record, j);
                          }
                          break;
                      case type_tag::uda:
                         {
                             auto& dimensions = parser_item.dimensions();
                             std::vector<Dimension> active_dimensions;
                             std::vector<Dimension> default_dimensions;
                             for (const auto& dim_string : dimensions) {
                                 active_dimensions.push_back(
                                     system_active.parse(dim_string));
                                 default_dimensions.push_back(
                                     system_default.parse(dim_string));
                             }
                             DeckItem deck_item(parser_item.name(), UDAValue(),
                                 active_dimensions, default_dimensions);
                             add_deckvalue<UDAValue>(std::move(deck_item),
                                 deck_record, parser_item, input_record, j);
                         }
                         break;
                      default: throw std::invalid_argument("For input to DeckKeyword '" + name() + ": unsupported type. (only support for string, double and int.)");
                  }
             }

             this->addRecord( std::move(deck_record) );

        }
    }

    DeckKeyword::DeckKeyword(const ParserKeyword& parserKeyword, const std::vector<int>& data) :
        DeckKeyword(parserKeyword)
    {
        if (!parserKeyword.isDataKeyword())
            throw std::invalid_argument("Deckkeyword '" + name() + "' is not a data keyword.");

        const ParserRecord& parser_record = parserKeyword.getRecord(0);
        const ParserItem& parser_item = parser_record.get(0);

        setDataKeyword();
        if (parser_item.dataType() != type_tag::integer)
            throw std::invalid_argument("Input to DeckKeyword '" + name() + "': cannot be std::vector<int>.");

        DeckItem item(parser_item.name(), int() );
        for (int val : data)
            item.push_back(val);

        DeckRecord deck_record;
        deck_record.addItem( std::move(item) );
        addRecord( std::move(deck_record) );
    }


    DeckKeyword::DeckKeyword(const ParserKeyword& parserKeyword, const std::vector<double>& data, const UnitSystem& system_active, const UnitSystem& system_default) :
        DeckKeyword(parserKeyword)
    {
        if (!parserKeyword.isDataKeyword())
            throw std::invalid_argument("Deckkeyword '" + name() + "' is not a data keyword.");

        const ParserRecord& parser_record = parserKeyword.getRecord(0);
        const ParserItem& parser_item = parser_record.get(0);

        setDataKeyword();
        if (parser_item.dataType() != type_tag::fdouble)
            throw std::invalid_argument("Input to DeckKeyword '" + name() + "': cannot be std::vector<double>.");

        auto& dim = parser_item.dimensions();
        std::vector<Dimension> active_dimensions;
        std::vector<Dimension> default_dimensions;
        if (dim.size() > 0) {
             active_dimensions.push_back( system_active.parse(dim[0]) );
             default_dimensions.push_back( system_default.parse(dim[0]) );
        }
        DeckItem item(parser_item.name(), double(), active_dimensions, default_dimensions);
        for (double val : data)
            item.push_back(val);

        DeckRecord deck_record;
        deck_record.addItem( std::move(item) );
        addRecord( std::move(deck_record) );
    }


    void DeckKeyword::setFixedSize() {
        m_slashTerminated = false;
    }


    const KeywordLocation& DeckKeyword::location() const {
        return this->m_location;
    }

    void DeckKeyword::setDataKeyword(bool isDataKeyword_) {
        m_isDataKeyword = isDataKeyword_;
    }

   void DeckKeyword::setDoubleRecordKeyword(bool isDoubleRecordKeyword) {
        m_isDoubleRecordKeyword = isDoubleRecordKeyword;
   }

    bool DeckKeyword::isDataKeyword() const {
        return m_isDataKeyword;
    }

    bool DeckKeyword::isDoubleRecordKeyword() const {
        return m_isDoubleRecordKeyword;
    }

    const std::string& DeckKeyword::name() const {
        return m_keywordName;
    }

    size_t DeckKeyword::size() const {
        return m_recordList.size();
    }

    bool DeckKeyword::empty() const {
        return this->m_recordList.empty();
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

    const DeckRecord& DeckKeyword::operator[](std::size_t index) const {
        return this->m_recordList.at( index );
    }

    DeckRecord& DeckKeyword::operator[](std::size_t index) {
        return this->m_recordList.at( index );
    }

    const DeckRecord& DeckKeyword::getRecord(size_t index) const {
        return this->operator[](index);
    }

    DeckRecord& DeckKeyword::getRecord(size_t index) {
        return this->operator[](index);
    }

    const DeckRecord& DeckKeyword::getDataRecord() const {
        if (m_recordList.size() == 1)
            return getRecord(0);
        else
            throw std::range_error("Not a data keyword \"" + name() + "\"?");
    }


    size_t DeckKeyword::getDataSize() const {
        return this->getDataRecord().getDataItem().data_size();
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

    const std::vector<value::status>& DeckKeyword::getValueStatus() const {
        return this->getDataRecord().getDataItem().getValueStatus();
   }

    void DeckKeyword::write_data( DeckOutput& output ) const {
        for (const auto& record: *this)
            record.write( output );
    }

    void DeckKeyword::write_TITLE( DeckOutput& output ) const {
        output.start_keyword( this->name( ), false );
        {
            const auto& record = this->getRecord(0);
            output.write_string("  ");
            record.write_data( output );
            output.endl();
        }
    }

    void DeckKeyword::write( DeckOutput& output ) const {
        if (this->name() == "TITLE")
            this->write_TITLE( output );
        else {
            bool split_line = this->isDataKeyword();
            if (this->name() == "VFPPROD" || this->name() == "VFPINJ" || this->name() == "TSTEP")
                split_line = true;

            output.start_keyword( this->name( ), split_line );
            this->write_data( output );
            output.end_keyword( this->m_slashTerminated );
        }
    }

    std::ostream& operator<<(std::ostream& os, const DeckKeyword& keyword) {
        DeckOutput out( os );
        keyword.write( out );
        return os;
    }

    bool DeckKeyword::equal_data(const DeckKeyword& other, bool cmp_default, bool cmp_numeric) const {
        if (this->size() != other.size())
            return false;

        for (size_t index = 0; index < this->size(); index++) {
            const auto& this_record = this->getRecord( index );
            const auto& other_record = other.getRecord( index );
            if (!this_record.equal( other_record , cmp_default, cmp_numeric))
                return false;
        }
        return true;
    }

    bool DeckKeyword::equal(const DeckKeyword& other, bool cmp_default, bool cmp_numeric) const {
        if (this->name() != other.name())
            return false;

        return this->equal_data(other, cmp_default, cmp_numeric);
    }

    bool DeckKeyword::operator==(const DeckKeyword& other) const {
        bool cmp_default = false;
        bool cmp_numeric = true;
        return this->equal( other , cmp_default, cmp_numeric);
    }

    bool DeckKeyword::operator!=(const DeckKeyword& other) const {
        return !(*this == other);
    }

}

