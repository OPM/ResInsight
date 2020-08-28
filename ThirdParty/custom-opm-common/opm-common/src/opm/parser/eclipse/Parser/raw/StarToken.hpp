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

#ifndef STAR_TOKEN_HPP
#define STAR_TOKEN_HPP

#include <cctype>
#include <string>

#include <opm/parser/eclipse/Utility/Stringview.hpp>
#include <opm/parser/eclipse/Utility/Typetools.hpp>

namespace Opm {
    bool isStarToken(const string_view& token,
                           std::string& countString,
                           std::string& valueString);

    template <class T>
    T readValueToken( string_view );

class StarToken {
public:
    StarToken(const string_view& token)
    {
        if (!isStarToken(token, m_countString, m_valueString))
            throw std::invalid_argument("Token \""+ token +"\" is not a repetition specifier");
        init_(token);
    }

    StarToken(const string_view& token, const std::string& countStr, const std::string& valueStr)
        : m_countString(countStr)
        , m_valueString(valueStr)
    {
        init_(token);
    }

    std::size_t count() const {
        return m_count;
    }

    bool hasValue() const {
        return !m_valueString.empty();
    }

    // returns the coubt as rendered in the deck. note that this might be different
    // than just converting the return value of count() to a string because an empty
    // count is interpreted as 1...
    const std::string& countString() const {
        return m_countString;
    }

    // returns the value as rendered in the deck. note that this might be different
    // than just converting the return value of value() to a string because values
    // might have different representations in the deck (e.g. strings can be
    // specified with and without quotes and but spaces are only allowed using the
    // first representation.)
    const std::string& valueString() const {
        return m_valueString;
    }

private:
    // internal initialization method. the m_countString and m_valueString attributes
    // must be set before calling this method.
    void init_(const string_view& token);

    std::size_t m_count;
    std::string m_countString;
    std::string m_valueString;
};
}


#endif
