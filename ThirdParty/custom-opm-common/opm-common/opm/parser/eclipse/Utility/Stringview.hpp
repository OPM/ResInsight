/*
  Copyright (C) 2016 by Statoil ASA

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

#ifndef OPM_UTILITY_SUBSTRING_HPP
#define OPM_UTILITY_SUBSTRING_HPP

#include <algorithm>
#include <cstring>
#include <iosfwd>
#include <stdexcept>
#include <string>

namespace Opm {
    /*
     * string_view is a simple view-into-substring feature whose primary
     * usecase is to avoid deep copying strings in the inner loops of the
     * parser. Relies on whatever it's viewing into is kept alive (as all
     * iterators do):
     *
     * auto rec = make_raw_record();
     * string_view view = rec.getItem( 3 );
     *
     * view.size(); view[ 10 ]; // ok
     * ~rec();
     * view[ 3 ]; // not ok
     *
     * This is desired to fill out a gap in the C++ standard library, since
     * string_view has yet to be standardised
     *
     * http://en.cppreference.com/w/cpp/experimental/basic_string_view
     */
    class string_view {
        public:
            using const_iterator = const char*;

            inline string_view() = default;
            inline string_view( const_iterator, const_iterator );
            inline string_view( const_iterator, size_t );
            //cppcheck-suppress noExplicitConstructor
            inline string_view( const std::string& );
            inline string_view( const std::string&, size_t );
            //cppcheck-suppress noExplicitConstructor
            inline string_view( const char* );

            inline const_iterator begin() const;
            inline const_iterator end() const;

            inline char front() const;
            inline char back() const;

            inline char operator[]( size_t ) const;
            inline bool operator<( const string_view& ) const;
            inline bool operator==( const string_view& ) const;

            inline bool empty() const;
            inline size_t size() const;
            inline size_t length() const;

            inline std::string string() const;
            inline std::string substr( size_t from = 0 ) const;
            inline std::string substr( size_t from, size_t len ) const;

            inline bool starts_with(const std::string& str) const;
            inline std::size_t find(const std::string& substring) const;
            inline std::size_t find(char c) const;
        private:
            const_iterator fst = nullptr;
            const_iterator lst = nullptr;
    };

    /*
     * The implementation of string_view is inline and therefore the definitions
     * are also in this file. The reason for this is performance; string_view's
     * logic is trivial and function call and indirection overhead is significant
     * compared to the useful work it does. Additionally, string_view is a *very*
     * much used class in the inner loops of the parser - inlining the
     * implementation measured to improve performance by some 10%.
     */


    // Non-member operators using string_view.

    std::ostream& operator<<( std::ostream& stream, const Opm::string_view& view );

    inline std::string operator+( std::string str, const Opm::string_view& view ) {
        return str.append( view.begin(), view.end() );
    }

    inline std::string operator+( const Opm::string_view& view, const std::string& str ) {
        return view.string().append( str.begin(), str.end() );
    }

    inline bool operator==( const Opm::string_view& view, const std::string& rhs ) {
        return rhs.size() == view.size() &&
            std::equal( view.begin(), view.end(), std::begin( rhs ) );
    }

    inline bool operator==( const Opm::string_view& view, const char* rhs ) {
        return std::strlen( rhs ) == view.size() &&
               std::equal( view.begin(), view.end(), rhs );
    }

    inline bool operator==( const std::string& lhs, const Opm::string_view& view ) {
        return view == lhs;
    }

    inline bool operator==( const char* lhs, const Opm::string_view& view ) {
        return view == lhs;
    }

    inline bool operator!=( const Opm::string_view& view, const std::string& rhs ) {
        return !( view == rhs );
    }

    inline bool operator!=( const std::string& lhs, const Opm::string_view& view ) {
        return !( view == lhs );
    }

    inline bool operator!=( const Opm::string_view& view, const char* rhs ) {
        return !( view == rhs );
    }

    inline bool operator!=( const char* lhs, const Opm::string_view& view ) {
        return !( view == lhs );
    }


    // Member functions of string_view.

    inline string_view::string_view( const_iterator first,
                                     const_iterator last ) :
        fst( first ),
        lst( last )
    {}

    inline string_view::string_view( const_iterator first ,
                                     size_t count ) :
        fst( first ),
        lst( first + count )
    {}

    inline string_view::string_view( const std::string& str ) :
        string_view( str.data(), str.size() )
    {}

    inline string_view::string_view( const std::string& str, size_t count ) :
        string_view( str.data(), count )
    {}

    inline string_view::string_view( const char* str ) :
        string_view( str, str + std::strlen( str ) )
    {}

    inline string_view::const_iterator string_view::begin() const {
        return this->fst;
    }

    inline string_view::const_iterator string_view::end() const {
        return this->lst;
    }

    inline char string_view::front() const {
        return *this->fst;
    }

    inline char string_view::back() const {
        return *(this->lst - 1);
    }

    inline char string_view::operator[]( size_t i ) const {
        return *(this->begin() + i);
    }

    inline bool string_view::operator<( const string_view& rhs ) const {
        return std::lexicographical_compare( this->begin(), this->end(),
                                             rhs.begin(), rhs.end() );
    }

    inline bool string_view::operator==( const string_view& rhs ) const {
        return std::equal( this->begin(), this->end(), rhs.begin() );
    }

    inline bool string_view::empty() const {
        return std::distance( this->begin(), this->end() ) == 0;
    }

    inline size_t string_view::size() const {
        return std::distance( this->begin(), this->end() );
    }

    inline size_t string_view::length() const {
        return std::distance( this->begin(), this->end() );
    }

    inline std::string string_view::string() const {
        return this->substr();
    }

    inline std::string string_view::substr( size_t from ) const {
        return this->substr( from, this->size() );
    }

    inline std::string string_view::substr( size_t from, size_t len ) const {
        if( from > this->size() )
            throw std::out_of_range( "'from' is greater than length" );

        if (from + len > this->size())
            return std::string( this->begin() + from, this->lst );

        return std::string( this->begin() + from, this->begin() + len  + from);
    }

    inline std::size_t string_view::find(const std::string& substring) const {
        auto substring_size = substring.size();
        if (substring_size > this->size())
            return std::string::npos;

        auto substring_data = substring.data();
        auto pos = this->fst;
        auto last_pos = this->lst - substring.size() + 1;

        while (pos != last_pos) {
            std::size_t si = 0;
            while (substring_data[si] == *(pos + si)) {
                si += 1;
                if (si == substring_size)
                    return pos - this->fst;
            }
            ++pos;
        }

        return std::string::npos;
    }

    inline std::size_t string_view::find(char c) const {
        auto pos = this->fst;

        while (pos != this->lst) {
            if (*pos == c)
                return pos - this->fst;

            ++pos;
        }

        return std::string::npos;
    }


    inline bool string_view::starts_with(const std::string& str) const {
        auto str_size = str.size();
        if (str_size > this->size())
            return false;

        auto str_data = str.data();
        auto pos = this->fst;

        std::size_t si = 0;
        while (true) {
            if (*pos != str_data[si])
                return false;

            ++pos;
            ++si;

            if (si == str_size)
                return true;
        }
    }


}

#endif //OPM_UTILITY_SUBSTRING_HPP
