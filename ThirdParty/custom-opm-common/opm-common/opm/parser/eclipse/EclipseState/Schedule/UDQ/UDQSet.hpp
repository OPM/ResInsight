/*
  Copyright 2019 Equinor ASA.

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

#ifndef UDQSET_HPP
#define UDQSET_HPP

#include <stdexcept>
#include <vector>
#include <string>
#include <unordered_map>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>

namespace Opm {

class UDQScalar {
public:
    UDQScalar() = default;
    explicit UDQScalar(double value);
    explicit UDQScalar(const std::string& wgname);
    UDQScalar(const std::string& wgname, double value);

    void operator+=(const UDQScalar& rhs);
    void operator+=(double rhs);
    void operator*=(const UDQScalar& rhs);
    void operator*=(double rhs);
    void operator/=(const UDQScalar& rhs);
    void operator/=(double rhs);
    void operator-=(const UDQScalar& rhs);
    void operator-=(double rhs);

    operator bool() const;
    void assign(double value);
    bool defined() const;
    double value() const;
    const std::string& wgname() const;
public:
    double m_value;
    std::string m_wgname;
    bool m_defined = false;
};


class UDQSet {
public:
    UDQSet(const std::string& name);
    UDQSet(const std::string& name, UDQVarType var_type);
    UDQSet(const std::string& name, UDQVarType var_type, const std::vector<std::string>& wgnames);
    UDQSet(const std::string& name, UDQVarType var_type, std::size_t size);
    UDQSet(const std::string& name, std::size_t size);
    static UDQSet scalar(const std::string& name, double value);
    static UDQSet empty(const std::string& name);
    static UDQSet wells(const std::string& name, const std::vector<std::string>& wells);
    static UDQSet wells(const std::string& name, const std::vector<std::string>& wells, double scalar_value);
    static UDQSet groups(const std::string& name, const std::vector<std::string>& groups);
    static UDQSet groups(const std::string& name, const std::vector<std::string>& groups, double scalar_value);
    static UDQSet field(const std::string& name, double scalar_value);

    void assign(double value);
    void assign(std::size_t index, double value);
    void assign(const std::string& wgname, double value);

    std::size_t size() const;
    void operator+=(const UDQSet& rhs);
    void operator+=(double rhs);
    void operator-=(const UDQSet& rhs);
    void operator-=(double rhs);
    void operator*=(const UDQSet& rhs);
    void operator*=(double rhs);
    void operator/=(const UDQSet& rhs);
    void operator/=(double rhs);

    const UDQScalar& operator[](std::size_t index) const;
    const UDQScalar& operator[](const std::string& wgname) const;
    std::vector<UDQScalar>::const_iterator begin() const;
    std::vector<UDQScalar>::const_iterator end() const;

    std::vector<std::string> wgnames() const;
    std::vector<double> defined_values() const;
    std::size_t defined_size() const;
    const std::string& name() const;
    void name(const std::string& name);
    UDQVarType var_type() const;
private:
    UDQSet() = default;

    std::string m_name;
    UDQVarType m_var_type = UDQVarType::NONE;
    std::vector<UDQScalar> values;
};


UDQScalar operator+(const UDQScalar&lhs, const UDQScalar& rhs);
UDQScalar operator+(const UDQScalar&lhs, double rhs);
UDQScalar operator+(double lhs, const UDQScalar& rhs);

UDQScalar operator-(const UDQScalar&lhs, const UDQScalar& rhs);
UDQScalar operator-(const UDQScalar&lhs, double rhs);
UDQScalar operator-(double lhs, const UDQScalar& rhs);

UDQScalar operator*(const UDQScalar&lhs, const UDQScalar& rhs);
UDQScalar operator*(const UDQScalar&lhs, double rhs);
UDQScalar operator*(double lhs, const UDQScalar& rhs);

UDQScalar operator/(const UDQScalar&lhs, const UDQScalar& rhs);
UDQScalar operator/(const UDQScalar&lhs, double rhs);
UDQScalar operator/(double lhs, const UDQScalar& rhs);

UDQSet operator+(const UDQSet&lhs, const UDQSet& rhs);
UDQSet operator+(const UDQSet&lhs, double rhs);
UDQSet operator+(double lhs, const UDQSet& rhs);

UDQSet operator-(const UDQSet&lhs, const UDQSet& rhs);
UDQSet operator-(const UDQSet&lhs, double rhs);
UDQSet operator-(double lhs, const UDQSet& rhs);

UDQSet operator*(const UDQSet&lhs, const UDQSet& rhs);
UDQSet operator*(const UDQSet&lhs, double rhs);
UDQSet operator*(double lhs, const UDQSet& rhs);

UDQSet operator/(const UDQSet&lhs, const UDQSet& rhs);
UDQSet operator/(const UDQSet&lhs, double rhs);
UDQSet operator/(double lhs, const UDQSet&rhs);

}



#endif
