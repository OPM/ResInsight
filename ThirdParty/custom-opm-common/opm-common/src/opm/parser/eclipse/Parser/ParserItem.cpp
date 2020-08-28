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

#include <ostream>
#include <sstream>
#include <iomanip>
#include <cmath>

#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>
#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include "raw/RawRecord.hpp"
#include "raw/StarToken.hpp"

namespace Opm {
class UnitSystem;

namespace {

type_tag get_data_type_json( const std::string& str ) {
    if( str == "INT" )       return type_tag::integer;
    if( str == "DOUBLE" )    return type_tag::fdouble;
    if( str == "STRING" )    return type_tag::string;
    if( str == "RAW_STRING") return type_tag::raw_string;
    if( str == "UDA")        return type_tag::uda;
    throw std::invalid_argument( str + " cannot be converted to enum 'tag'" );
}

/*
  For very small numbers std::to_string() will just return the string "0.000000"
*/
std::string as_string(double value) {
    if (std::fabs(value) < 1e-4) {
        std::ostringstream ss;
        ss << std::setprecision(12) << value;
        return ss.str();
    } else
        return std::to_string(value);
}

}

ParserItem::item_size ParserItem::size_from_string( const std::string& str ) {
    if( str == "ALL" )   return item_size::ALL;
    if( str == "SINGLE") return item_size::SINGLE;
    throw std::invalid_argument( str + " can not be converted "
                                 "to enum 'item_size'" );
}

std::string ParserItem::string_from_size( ParserItem::item_size sz ) {
    switch( sz ) {
        case item_size::ALL:    return "ALL";
        case item_size::SINGLE: return "SINGLE";
    }

    throw std::logic_error( "ParserItem::string_from_size: Fatal error; should not be reachable" );
}

template<> const UDAValue& ParserItem::value_ref< UDAValue >() const {
    if( this->data_type != get_type< UDAValue >() )
        throw std::invalid_argument("ValueRef<UDAValue> Wrong type." );

    return this->uval;
}

template<> const int& ParserItem::value_ref< int >() const {
    if( this->data_type != get_type< int >() )
        throw std::invalid_argument( "ValueRef<int>: Wrong type." );
    return this->ival;
}

template<> const double& ParserItem::value_ref< double >() const {
    if( this->data_type != get_type< double >() )
        throw std::invalid_argument("ValueRef<double> Wrong type." );

    return this->dval;
}

template<> const std::string& ParserItem::value_ref< std::string >() const {
    if( this->data_type != get_type< std::string >() )
        throw std::invalid_argument( "ValueRef<std::string> Wrong type." );
    return this->sval;
}

template<> const RawString& ParserItem::value_ref< RawString >() const {
    if( this->data_type != get_type<RawString>() )
        throw std::invalid_argument( "ValueRef<RawString> Wrong type." );
    return this->rsval;
}

template< typename T >
T& ParserItem::value_ref() {
    return const_cast< T& >(
                const_cast< const ParserItem& >( *this ).value_ref< T >()
            );
}


ParserItem::ParserItem( const std::string& itemName, ParserItem::itype input_type_arg) :
    m_name(itemName),
    m_defaultSet(false)
{
    this->setInputType(input_type_arg);
}

ParserItem::ParserItem( const Json::JsonObject& json ) :
    m_name( json.get_string( "name" ) ),
    m_sizeType( json.has_item( "size_type" )
              ? ParserItem::size_from_string( json.get_string( "size_type" ) )
              : ParserItem::item_size::SINGLE ),
    m_description( json.has_item( "description" )
                 ? json.get_string( "description" )
                 : "" ),
    data_type( get_data_type_json( json.get_string( "value_type" ) ) ),
    input_type( ParserItem::from_string( json.get_string("value_type"))),
    m_defaultSet( false )
{
    if( json.has_item( "dimension" ) ) {
        const auto& dim = json.get_item( "dimension" );

        if( dim.is_string() ) {
            this->push_backDimension( dim.as_string() );
        }
        else if( dim.is_array() ) {
            for( size_t i = 0; i < dim.size(); ++i )
                this->push_backDimension( dim.get_array_item( i ).as_string() );
        }
        else {
            throw std::invalid_argument(
                "The 'dimension' attribute must be a string or list of strings"
            );
        }
    }
    if( !json.has_item( "default" ) ) return;

    switch( this->input_type ) {
    case itype::INT:
        this->setDefault( json.get_int( "default" ) );
        break;

    case itype::DOUBLE:
        this->setDefault( json.get_double( "default" ) );
        break;

    case itype::UDA:
        this->setDefault( UDAValue(json.get_double( "default" )) );
        break;

    case itype::STRING:
    case itype::RAW_STRING:
        this->setDefault( json.get_string( "default" ) );
        break;

    default:
        throw std::logic_error( "Item of unknown type: <" + json.get_string("value_type") + ">" );
    }
}

template< typename T >
void ParserItem::setDefault( T val ) {
    if( this->data_type != type_tag::fdouble && this->m_sizeType == item_size::ALL )
        throw std::invalid_argument( "The size type ALL can not be combined "
                                     "with an explicit default value." );

    this->value_ref< T >() = std::move( val );
    this->m_defaultSet = true;
}


void ParserItem::setInputType(ParserItem::itype input_type_arg) {
    this->input_type = input_type_arg;

    if (input_type == itype::INT)
        this->setDataType(int());

    else if (input_type == itype::DOUBLE)
        this->setDataType(double());

    else if (input_type == itype::STRING)
        this->setDataType( std::string() );

    else if (input_type == itype::RAW_STRING)
        this->setDataType( RawString() );

    else if (input_type == itype::UDA)
        this->setDataType( UDAValue(0) );

    else if (input_type == itype::CODE)
        this->setDataType( std::string() );
    else
        throw std::invalid_argument("BUG: input type not recognized in setInputType()");
}


template< typename T >
void ParserItem::setDataType( T) {
    this->data_type = get_type< T >();
}

bool ParserItem::hasDefault() const {
    return this->m_defaultSet;
}


template< typename T>
const T& ParserItem::getDefault() const {
    if( get_type< T >() != this->data_type )
        throw std::invalid_argument( "getDefault: Wrong type." );

    if( !this->hasDefault() )
        throw std::invalid_argument( "No default value available for item "
                                     + this->name() );

    return this->value_ref< T >();
}


const std::vector<std::string>& ParserItem::dimensions() const {
    return this->m_dimensions;
}

void ParserItem::push_backDimension( const std::string& dim ) {
    if (!(this->input_type == ParserItem::itype::DOUBLE || this->input_type == ParserItem::itype::UDA))
        throw std::invalid_argument( "Invalid type, does not have dimension." );

    if( this->sizeType() == item_size::SINGLE && this->m_dimensions.size() > 0 ) {
        throw std::invalid_argument(
            "Internal error: "
            "cannot add more than one dimension to an item of size 1" );
    }

    this->m_dimensions.push_back( dim );
}

    const std::string& ParserItem::name() const {
        return m_name;
    }

    const std::string ParserItem::className() const {
        return m_name;
    }


    type_tag ParserItem::dataType() const {
        return this->data_type;
    }


    ParserItem::item_size ParserItem::sizeType() const {
        return m_sizeType;
    }

    bool ParserItem::scalar() const {
        return this->m_sizeType == item_size::SINGLE;
    }

    std::string ParserItem::getDescription() const {
        return m_description;
    }


    void ParserItem::setSizeType(item_size size_type) {
        /*
          The restriction that data type UDA can only be combined with size_type
          SINGLE is due to the way units are bolted on to the Deck
          datastructures after the parsing has completed. UDA values are
          currently only used as scalars in well/group control and the
          restriction does not have any effect. If at some stage in the future
          this should change the way units are applied to the deck must be
          refactored.
         */
        if (this->data_type == type_tag::uda && size_type != item_size::SINGLE)
            throw std::invalid_argument("Sorry - the UDA datatype can only be used with size type SINGLE");

        this->m_sizeType = size_type;
    }


    void ParserItem::setDescription(const std::string& description) {
        m_description = description;
    }


    bool ParserItem::operator==( const ParserItem& rhs ) const {
    if( !( this->data_type      == rhs.data_type
           && this->m_name         == rhs.m_name
           && this->m_description  == rhs.m_description
           && this->input_type     == rhs.input_type
           && this->m_sizeType     == rhs.m_sizeType
           && this->m_defaultSet   == rhs.m_defaultSet ) )
        return false;

    if( this->m_defaultSet ) {
        switch( this->data_type ) {
            case type_tag::integer:
                if( this->ival != rhs.ival ) return false;
                break;

            case type_tag::fdouble:
                if( this->dval != rhs.dval ) {
                    double diff = std::fabs(this->dval - rhs.dval);
                    double sum = std::fabs(this->dval) + std::fabs(rhs.dval);
                    if ((diff / sum) > 1e-8)
                        return false;
                }
                break;

            case type_tag::string:
                if( this->sval != rhs.sval ) return false;
                break;

            case type_tag::raw_string:
                if( this->rsval != rhs.rsval ) return false;
                break;

            case type_tag::uda:
                if( this->uval != rhs.uval ) return false;
                break;

            default:
                throw std::logic_error( "Item of unknown type data_type:" + tag_name(this->data_type));
        }
    }
    if( this->data_type != type_tag::fdouble ) return true;
    return this->m_dimensions.size() == rhs.m_dimensions.size()
        && std::equal( this->m_dimensions.begin(),
                       this->m_dimensions.end(),
                       rhs.m_dimensions.begin() );
}

bool ParserItem::operator!=( const ParserItem& rhs ) const {
    return !( *this == rhs );
}


std::string ParserItem::size_literal() const {
    if (this->m_sizeType == item_size::ALL)
        return "ParserItem::item_size::ALL";
    else
        return "ParserItem::item_size::SINGLE";
}

std::string ParserItem::type_literal() const {
    if (this->input_type == itype::DOUBLE)
        return "ParserItem::itype::DOUBLE";

    if (this->input_type == itype::INT)
        return "ParserItem::itype::INT";

    if (this->input_type == itype::STRING)
        return "ParserItem::itype::STRING";

    if (this->input_type == itype::RAW_STRING)
        return "ParserItem::itype::RAW_STRING";

    if (this->input_type == itype::UDA)
        return "ParserItem::itype::UDA";

    throw std::invalid_argument("Could not resolve type literal");
}

std::string ParserItem::to_string(itype input_type) {
    if (input_type == itype::RAW_STRING)
        return "RAW_STRING";

    if (input_type == itype::STRING)
        return "STRING";

    if (input_type == itype::DOUBLE)
        return "DOUBLE";

    if (input_type == itype::INT)
        return "INT";

    throw std::invalid_argument("Can not convert to string");
}


ParserItem::itype ParserItem::from_string(const std::string& string_value) {
    if( string_value == "INT" )       return itype::INT;
    if( string_value == "DOUBLE" )    return itype::DOUBLE;
    if( string_value == "STRING" )    return itype::STRING;
    if( string_value == "RAW_STRING") return itype::RAW_STRING;
    if( string_value == "UDA")        return itype::UDA;
    throw std::invalid_argument( string_value + " cannot be converted to ParserInputType" );
}


std::string ParserItem::createCode(const std::string& indent) const {
    std::stringstream stream;
    stream << indent << "ParserItem item(\"" << this->name() <<"\", " << this->type_literal() << ");" << '\n';
    if (this->m_sizeType != ParserItem::item_size::SINGLE)
        stream << indent << "item.setSizeType(" << this->size_literal() << ");"  << '\n';

    if( m_defaultSet ) {
        stream << indent << "item.setDefault( ";
        switch( this->data_type ) {
        case type_tag::integer:
            stream << this->getDefault< int >();
            break;

        case type_tag::fdouble:
            stream << "double(" << as_string(this->getDefault<double>()) << ")";
            break;

        case type_tag::uda:
            {
                double double_value =this->getDefault<UDAValue>().get<double>();
                stream << "UDAValue(" << as_string(double_value) << ")";
            }
            break;

        case type_tag::string:
            stream << "std::string(\"" << this->getDefault< std::string >() << "\")";
            break;

        case type_tag::raw_string:
            stream << "RawString(\"" << this->getDefault< RawString >() << "\")";
            break;

        default:
            throw std::logic_error( "Item of unknown type." );
        }
        stream << " );" << '\n';
    }

    for (const auto& dim : this->m_dimensions)
        stream << indent <<"item.push_backDimension(\"" << dim << "\");" << '\n';

    if (this->m_description.size() > 0)
        stream << indent << "item.setDescription(\"" << this->m_description << "\");" << '\n';

    return stream.str();
}

namespace {

template< typename T >
void scan_item( DeckItem& deck_item, const ParserItem& parser_item, RawRecord& record ) {
    bool parse_raw = parser_item.parseRaw();

    if( parser_item.sizeType() == ParserItem::item_size::ALL ) {
        if (parse_raw) {
            while (record.size()) {
                auto token = record.pop_front();
                auto raw_string = RawString{ token.string() };
                deck_item.push_back( raw_string );
            }
            return;
        }

        while( record.size() > 0 ) {
            auto token = record.pop_front();

            std::string countString;
            std::string valueString;

            if( !isStarToken( token, countString, valueString ) ) {
                deck_item.push_back( readValueToken< T >( token ) );
                continue;
            }

            StarToken st(token, countString, valueString);

            if( st.hasValue() ) {
                deck_item.push_back( readValueToken< T >( st.valueString() ), st.count() );
                continue;
            }

            if (parser_item.hasDefault()) {
                auto value = parser_item.getDefault< T >();
                for (size_t i=0; i < st.count(); i++)
                    deck_item.push_backDefault( value );
            } else {
                for (size_t i=0; i < st.count(); i++)
                    deck_item.push_backDummyDefault<T>();
            }
        }

        return;
    }

    if( record.size() == 0 ) {
        // if the record was ended prematurely,
        if( parser_item.hasDefault() ) {
            // use the default value for the item, if there is one...
            deck_item.push_backDefault( parser_item.getDefault< T >() );
        } else {
            // ... otherwise indicate that the deck item should throw once the
            // item's data is accessed.
            deck_item.push_backDummyDefault<T>();
        }

        return;
    }

    if (parse_raw) {
        auto token = record.pop_front();
        auto raw_string = RawString{ token.string() };
        deck_item.push_back( raw_string );
        return;
    }

    // The '*' should be interpreted as a repetition indicator, but it must
    // be preceeded by an integer...
    auto token = record.pop_front();
    std::string countString;
    std::string valueString;
    if( !isStarToken(token, countString, valueString) ) {
        deck_item.push_back( readValueToken<T>( token) );
        return;
    }

    StarToken st(token, countString, valueString);

    if( st.hasValue() )
        deck_item.push_back(readValueToken< T >( st.valueString()) );
    else if( parser_item.hasDefault() )
        deck_item.push_backDefault( parser_item.getDefault< T >() );
    else
        deck_item.push_backDummyDefault<T>();

    const auto value_start = token.size() - valueString.size();
    // replace the first occurence of "N*FOO" by a sequence of N-1 times
    // "FOO". this is slightly hacky, but it makes it work if the
    // number of defaults pass item boundaries...
    // We can safely make a string_view of one_star because it
    // has static storage
    static const char* one_star = "1*";
    string_view rep = !st.hasValue()
                    ? string_view{ one_star }
                    : string_view{ token.begin() + value_start, token.end() };
    record.prepend( st.count() - 1, rep );

    return;
}

}


/// Scans the records data according to the ParserItems definition.
/// returns a DeckItem object.
/// NOTE: data are popped from the records deque!
DeckItem ParserItem::scan( RawRecord& record, UnitSystem& active_unitsystem, UnitSystem& default_unitsystem) const {
    switch( this->data_type ) {
    case type_tag::integer:
        {
            DeckItem item( this->name(), int());
            scan_item< int >( item, *this, record );
            return item;
        }
        break;
    case type_tag::fdouble:
        {
            std::vector<Dimension> active_dimensions;
            std::vector<Dimension> default_dimensions;
            for (const auto& dim_string : this->m_dimensions) {
                active_dimensions.push_back( active_unitsystem.getNewDimension(dim_string) );
                default_dimensions.push_back( default_unitsystem.getNewDimension(dim_string) );
            }

            DeckItem item(this->name(), double(), active_dimensions, default_dimensions);
            scan_item< double >( item, *this, record );
            return item;
        }
        break;
    case type_tag::string:
        {
            DeckItem item(this->name(), std::string());
            scan_item< std::string >( item, *this, record );
            return item;
        }
        break;
    case type_tag::raw_string:
        {
            DeckItem item(this->name(), RawString());
            scan_item<RawString>( item, *this, record );
            return item;
        }
        break;
    case type_tag::uda:
        {
            std::vector<Dimension> active_dimensions;
            std::vector<Dimension> default_dimensions;
            for (const auto& dim_string : this->m_dimensions) {
                active_dimensions.push_back( active_unitsystem.getNewDimension(dim_string) );
                default_dimensions.push_back( default_unitsystem.getNewDimension(dim_string) );
            }

            DeckItem item(this->name(), UDAValue(), active_dimensions, default_dimensions);
            scan_item<UDAValue>(item, *this, record);
            return item;
        }
        break;
    default:
        throw std::logic_error( "ParserItem::scan: Fatal error; should not be reachable" );
    }
}

std::ostream& ParserItem::inlineClass( std::ostream& stream, const std::string& indent ) const {
    std::string local_indent = indent + "    ";

    stream << indent << "class " << this->className() << " {" << '\n'
           << indent << "public:" << '\n'
           << local_indent << "static const std::string itemName;" << '\n';

    if( this->hasDefault() ) {
        stream << local_indent << "static const "
               << tag_name( this->data_type )
               << " defaultValue;" << '\n';
    }

    return stream << indent << "};" << '\n';
}

std::string ParserItem::inlineClassInit(const std::string& parentClass,
                                        const std::string* defaultValue ) const {

    std::stringstream ss;
    ss << "const std::string " << parentClass
       << "::" << this->className()
       << "::itemName = \"" << this->name()
       << "\";" << '\n';

    if( !this->hasDefault() ) return ss.str();

    auto typestring = tag_name( this->data_type );

    auto defval = [this]() -> std::string {
        switch( this->data_type ) {
            case type_tag::integer:
                return std::to_string( this->getDefault< int >() );
            case type_tag::fdouble:
                return std::to_string( this->getDefault< double >() );
            case type_tag::uda:
                {
                    double value = this->getDefault<UDAValue>().get<double>();
                    return "UDAValue(" + std::to_string(value) + ")";
                }
            case type_tag::string:
                return "\"" + this->getDefault< std::string >() + "\"";

            default:
                throw std::logic_error( "ParserItem::inlineClassInit: Fatal error; should not be reachable" );
        }
    };

    ss << "const " << typestring << " "
        << parentClass << "::" << this->className()
        << "::defaultValue = " << (defaultValue ? *defaultValue : defval() )
        << ";" << '\n';

    return ss.str();
}


std::ostream& operator<<( std::ostream& stream, const ParserItem::item_size& sz ) {
    return stream << ParserItem::string_from_size( sz );
}

std::ostream& operator<<( std::ostream& stream, const ParserItem& item ) {
    stream
        << "ParserItem " << item.name() << " { "
        << "size: " << item.sizeType() << " "
        << "description: '" << item.getDescription() << "' "
        ;

    if( item.hasDefault() ) {
        stream << "default: ";
        switch( item.data_type ) {
            case type_tag::integer:
                stream << item.getDefault< int >();
                break;

            case type_tag::fdouble:
                stream << item.getDefault< double >();
                break;

            case type_tag::string:
                stream << "'" << item.getDefault< std::string >() << "'";
                break;

            default:
                throw std::logic_error( "Item of unknown type." );
        }

        stream << " ";
    }

    if( item.dimensions().empty() )
        stream << "dimensions: none";
    else {
        stream << "dimensions: [ ";
        for (const auto& dim : item.dimensions())
            stream << "'" << dim << "' ";
        stream << "]";
    }

    return stream << " }";
}

bool ParserItem::parseRaw( ) const {
    return (this->input_type == itype::RAW_STRING);
}

template void ParserItem::setDefault( int );
template void ParserItem::setDefault( double );
template void ParserItem::setDefault( std::string );
template void ParserItem::setDefault( UDAValue );

template void ParserItem::setDataType( int );
template void ParserItem::setDataType( double );
template void ParserItem::setDataType( std::string );
template void ParserItem::setDataType( UDAValue );

template const int& ParserItem::getDefault() const;
template const double& ParserItem::getDefault() const;
template const std::string& ParserItem::getDefault() const;
template const UDAValue& ParserItem::getDefault() const;

}
