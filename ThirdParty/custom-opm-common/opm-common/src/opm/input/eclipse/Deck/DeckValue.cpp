/*
  Copyright 2019 Statoil ASA.

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
#include <iostream>

#include <opm/input/eclipse/Deck/DeckValue.hpp>

namespace Opm {

DeckValue::DeckValue():
    default_value(true),
    value_enum(type_tag::unknown)
{}

DeckValue::DeckValue(int value):
    default_value(false),
    value_enum(type_tag::integer),
    int_value(value)
{}

DeckValue::DeckValue(double value):
    default_value(false),
    value_enum(type_tag::fdouble),
    double_value(value)
{}

DeckValue::DeckValue(const std::string& value):
    default_value(false),
    value_enum(type_tag::string),
    string_value(value)
{}

DeckValue::DeckValue(const UDAValue& value):
    default_value(false),
    value_enum(type_tag::uda),
    uda_value(value)
{}

bool DeckValue::is_default() const {
    return default_value;
}

template<>
int DeckValue::get() const {
    if (value_enum == type_tag::integer)
        return this->int_value;

    throw std::invalid_argument("DeckValue does not hold an integer value");
}

template<>
double DeckValue::get() const {
    if (value_enum == type_tag::fdouble)
        return this->double_value;

    if (value_enum == type_tag::integer)
        return this->int_value;

    throw std::invalid_argument("DeckValue does not hold a double value");
}

template<>
std::string DeckValue::get() const {
    if (value_enum == type_tag::string)
        return this->string_value;

    throw std::invalid_argument("DeckValue does not hold a string value");
}

template<>
UDAValue DeckValue::get() const {
    if (value_enum == type_tag::uda)
        return this->uda_value;

    throw std::invalid_argument("DeckValue does not hold an UDAValue");
}

template<>
bool DeckValue::is_compatible<int>() const {
    return (value_enum == type_tag::integer);
}

template<>
bool DeckValue::is_compatible<double>() const {
    return (value_enum == type_tag::fdouble || value_enum == type_tag::integer);
}

template<>
bool DeckValue::is_compatible<std::string>() const {
    return (value_enum == type_tag::string);
}

template<>
bool DeckValue::is_compatible<UDAValue>() const {
    return (value_enum == type_tag::uda);
}

}
