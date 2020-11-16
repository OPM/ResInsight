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

#include <array>
#include <algorithm>
#include <cctype>
#include <string>
#include <stdexcept>
#include <cstdlib>

#include <boost/spirit/include/qi.hpp>

#include <opm/parser/eclipse/Utility/Stringview.hpp>
#include <opm/parser/eclipse/Deck/UDAValue.hpp>

#include "StarToken.hpp"

namespace qi = boost::spirit::qi;

namespace Opm {

    bool isStarToken(const string_view& token,
                           std::string& countString,
                           std::string& valueString) {
        // find first character which is not a digit
        size_t pos = 0;
        for (; pos < token.length(); ++pos)
            if (!std::isdigit(token[pos]))
                break;

        // if no such character exists or if this character is not a star, the token is
        // not a "star token" (i.e. it is not a "repeat this value N times" token.
        if (pos >= token.size() || token[pos] != '*')
            return false;
        // Quote from the Eclipse Reference Manual: "An asterisk by
        // itself is not sufficent". However, our experience is that
        // Eclipse accepts such tokens and we therefore interpret "*"
        // as "1*".
        //
        // Tokens like "*12" are recognized as a star token
        // here, but we will throw in the code which uses
        // StarToken<T>. (Because Eclipse does not seem to
        // accept these and we would stay as closely to the spec as
        // possible.)
        else if (pos == 0) {
            countString = "";
            valueString = token.substr(pos + 1);
            return true;
        }

        // if a star is prefixed by an unsigned integer N, then this should be
        // interpreted as "repeat value after star N times"
        countString = token.substr(0, pos);
        valueString = token.substr(pos + 1);
        return true;
    }

    template<>
    int readValueToken< int >( string_view view ) {
        int n = 0;
        auto cursor = view.begin();
        const bool ok = qi::parse( cursor, view.end(), qi::int_, n );

        if( ok && cursor == view.end() ) return n;
        throw std::invalid_argument( "Malformed integer '" + view + "'" );
    }

    template< typename T >
    struct fortran_double : qi::real_policies< T > {
        // Eclipse supports Fortran syntax for specifying exponents of floating point
        // numbers ('D' and 'E', e.g., 1.234d5)
        template< typename It >
        static bool parse_exp( It& first, const It& last ) {
            if( first == last ||
                (*first != 'e' && *first != 'E' &&
                *first != 'd' && *first != 'D' ) )
                return false;
            ++first;
            return true;
        }
    };

    template<>
    double readValueToken< double >( string_view view ) {
        double n = 0;
        qi::real_parser< double, fortran_double< double > > double_;
        auto cursor = view.begin();
        const auto ok = qi::parse( cursor, view.end(), double_, n );

        if( ok && cursor == view.end() ) return n;
        throw std::invalid_argument( "Malformed floating point number '" + view + "'" );
    }


    template <>
    std::string readValueToken< std::string >( string_view view ) {
        if( view.size() == 0 || view[ 0 ] != '\'' )
            return view.string();

        if( view.size() < 2 || view[ view.size() - 1 ] != '\'')
            throw std::invalid_argument("Unable to parse string '" + view + "' as a string token");

        return view.substr( 1, view.size() - 2 );
    }

    template <>
    RawString readValueToken<RawString>( string_view view ) {
        return { view.string() };
    }


    template<>
    UDAValue readValueToken< UDAValue >( string_view view ) {
        double n = 0;
        qi::real_parser< double, fortran_double< double > > double_;
        auto cursor = view.begin();
        const auto ok = qi::parse( cursor, view.end(), double_, n );

        if( ok && cursor == view.end() ) return UDAValue(n);
        return UDAValue( readValueToken<std::string>(view) );
    }

    void StarToken::init_( const string_view& token ) {
        // special-case the interpretation of a lone star as "1*" but do not
        // allow constructs like "*123"...
        if (m_countString == "") {
            if (m_valueString != "")
                // TODO: decorate the deck with a warning instead?
                throw std::invalid_argument("Not specifying a count also implies not specifying a value. Token: \'" + token + "\'.");

            // TODO: since this is explicitly forbidden by the documentation it might
            // be a good idea to decorate the deck with a warning?
            m_count = 1;
        }
        else {
            const auto cnt = std::stoi( m_countString );

            if (cnt < 1)
                // TODO: decorate the deck with a warning instead?
                throw std::invalid_argument("Specifing zero repetitions is not allowed. Token: \'" + token + "\'.");

            m_count = static_cast<std::size_t>(cnt);
        }
    }

}
