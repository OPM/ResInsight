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
#include <fmt/format.h>

#include <iostream>

#include <opm/input/eclipse/Deck/UDAValue.hpp>


namespace Opm {

UDAValue::UDAValue(double value):
    numeric_value(true),
    double_value(value)
{
}

UDAValue::UDAValue(double value, const Dimension& dim_):
    numeric_value(true),
    double_value(value),
    dim(dim_)
{
}

UDAValue::UDAValue(const Dimension& dim_):
    UDAValue(0, dim_)
{
}

UDAValue::UDAValue() :
    UDAValue(0)
{}

UDAValue::UDAValue(const std::string& value):
    numeric_value(false),
    string_value(value)
{
}

UDAValue::UDAValue(const std::string& value, const Dimension& dim_):
    numeric_value(false),
    string_value(value),
    dim(dim_)
{
}

UDAValue UDAValue::serializeObject()
{
    UDAValue result;
    result.numeric_value = true;
    result.double_value = 1.0;
    result.string_value = "test";
    result.dim = Dimension::serializeObject();

    return result;
}


void UDAValue::assert_numeric() const {
    if (!this->numeric_value) {
        std::string msg = fmt::format("Internal error: The support for use of UDQ/UDA is not complete in opm/flow. The string: '{}' must be numeric", this->string_value);
        this->assert_numeric(msg);
    }
}


void UDAValue::assert_numeric(const std::string& error_msg) const {
    if (this->numeric_value)
        return;

    throw std::invalid_argument(error_msg);
}
double UDAValue::epsilonLimit() const {
        return 1.E-20;
}

template<>
bool UDAValue::is<double>() const {
    return this->numeric_value;
}


template<>
bool UDAValue::is<std::string>() const {
  return !this->numeric_value;
}


template<>
double UDAValue::get() const {
    this->assert_numeric();
    return this->double_value;
}


double UDAValue::getSI() const {
    this->assert_numeric();
    return this->dim.convertRawToSi(this->double_value);
}


void UDAValue::update(double value) {
    this->double_value = value;
    this->numeric_value = true;
}

void UDAValue::update(const std::string& value) {
    this->string_value = value;
    this->numeric_value = false;
}



template<>
std::string UDAValue::get() const {
    if (!this->numeric_value)
        return this->string_value;

    throw std::invalid_argument("UDAValue does not hold a string value");
}

bool UDAValue::zero() const {
    this->assert_numeric();
    return (this->double_value == 0.0);
}

const Dimension& UDAValue::get_dim() const {
    return this->dim;
}


bool UDAValue::operator==(const UDAValue& other) const {
    if (this->numeric_value != other.numeric_value)
        return false;

    if (this->dim != other.dim)
        return false;

    if (this->numeric_value)
        return (this->double_value == other.double_value);

    return this->string_value == other.string_value;
}


bool UDAValue::operator!=(const UDAValue& other) const {
    return !(*this == other);
}

std::ostream& operator<<( std::ostream& stream, const UDAValue& uda_value ) {
    if (uda_value.is<double>())
        stream << uda_value.get<double>();
    else
        stream << "'" << uda_value.get<std::string>() << "'";
    return stream;
}

void UDAValue::update_value(const UDAValue& other) {
    if (other.is<double>()) {
        this->double_value = other.get<double>();
        this->numeric_value = true;
    } else {
        this->string_value = other.get<std::string>();
        this->numeric_value = false;
    }
}

void UDAValue::operator*=(double factor) {
    if (this->is<double>())
        this->double_value *=factor;
    else
        throw std::logic_error(fmt::format("Can not multiply UDA: {} with numeric value", this->get<std::string>()));
}


}
