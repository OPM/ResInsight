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
#ifndef PARSER_ITEM_H
#define PARSER_ITEM_H

#include <iosfwd>
#include <string>
#include <vector>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Utility/Typetools.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

namespace Json {
    class JsonObject;
}

namespace Opm {

    class UnitSystem;
    class RawRecord;


    /*
      The ParserItem class describes one item handled by the parser. A parser
      item is the schema for parsing values from the deck, when configuring the
      ParserItem *two* types are in action:

        InputType: These are the types specified when instantiating a
           ParserItem, the available types are currently: INT, DOUBLE, STRING,
           RAW_STRING and UDA.

        DataType: This the C++ type of items generated when parsing the deck,
           currently the available datatypes are int, double, std::string and
           the user defined type UDAValue.

      Splitting the type treatment in two layers in this way enables
      properties/transformations to be added to the data before they are
      internalized as data in a DataType instance; e.g. the difference between
      STRING and RAW_STRING is that for the latter quotes and '*' tokens are
      retained.
    */


    class ParserItem {
    public:
        enum class item_size { ALL, SINGLE };
        static item_size   size_from_string( const std::string& );
        static std::string string_from_size( item_size );

        enum class itype {UNKNOWN, DOUBLE, INT, STRING, RAW_STRING, UDA, CODE};
        static itype from_string(const std::string& string_value);
        static std::string to_string(itype input_type);
        std::string type_literal() const;


        explicit ParserItem( const std::string& name, ParserItem::itype input_type );
        explicit ParserItem( const Json::JsonObject& jsonConfig );

        void push_backDimension( const std::string& );
        const std::vector<std::string>& dimensions() const;
        const std::string& name() const;
        item_size sizeType() const;
        type_tag dataType() const;
        void setSizeType(item_size size_type);
        std::string getDescription() const;
        bool scalar() const;
        void setDescription(const std::string& helpText);

        template< typename T > void setDefault( T );
        /* set type without a default value. will reset dimension etc. */
        void setInputType( itype input_type );
        bool parseRaw() const;
        bool hasDefault() const;
        template< typename T > const T& getDefault() const;

        bool operator==( const ParserItem& ) const;
        bool operator!=( const ParserItem& ) const;

        DeckItem scan( RawRecord& rawRecord, UnitSystem& active_unitsystem, UnitSystem& default_unitsystem) const;

        std::string size_literal() const;
        const std::string className() const;
        std::string createCode(const std::string& indent) const;
        std::ostream& inlineClass(std::ostream&, const std::string& indent) const;
        std::string inlineClassInit(const std::string& parentClass,
                                    const std::string* defaultValue = nullptr ) const;

    private:
        double dval;
        int ival;
        std::string sval;
        RawString rsval;
        UDAValue uval;
        std::vector< std::string > m_dimensions;

        std::string m_name;
        item_size m_sizeType = item_size::SINGLE;
        std::string m_description;

        type_tag data_type = type_tag::unknown;
        itype input_type = itype::UNKNOWN;
        bool m_defaultSet;

        template< typename T > T& value_ref();
        template< typename T > const T& value_ref() const;
        template< typename T > void setDataType( T );
        friend std::ostream& operator<<( std::ostream&, const ParserItem& );
    };

std::ostream& operator<<( std::ostream&, const ParserItem::item_size& );

}

#endif
