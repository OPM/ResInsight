/*
  Copyright 2020 Equinor ASA.

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


#ifndef UDQ_TOKEN_HPP
#define UDQ_TOKEN_HPP

#include <string>
#include <vector>
#include <variant>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>

namespace Opm {

class UDQToken {
public:
    UDQToken(const std::string& string_token, UDQTokenType token_type);
    UDQToken(const std::string& string_token, const std::vector<std::string>& selector);

    const std::vector<std::string>& selector() const;
    const std::variant<std::string, double>& value() const;
    UDQTokenType type() const;
    std::string str() const;
private:
    UDQTokenType token_type;
    std::variant<std::string,double> m_value;
    std::vector<std::string> m_selector;
};

}


#endif
