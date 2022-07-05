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

#ifndef UDQFUNCTIONTABLE_HPP
#define UDQFUNCTIONTABLE_HPP

#include <unordered_map>
#include <memory>

#include <opm/input/eclipse/Schedule/UDQ/UDQFunction.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQParams.hpp>

namespace Opm {

class UDQFunctionTable {
public:
    using FunctionMap = std::unordered_map<std::string,
                                           std::shared_ptr<UDQFunction>>;
    explicit UDQFunctionTable(const UDQParams& params);
    UDQFunctionTable();
    UDQFunctionTable(const UDQParams& param,
                     const FunctionMap& map);
    bool has_function(const std::string& name) const;
    const UDQFunction& get(const std::string& name) const;

    const UDQParams& getParams() const;
    const FunctionMap& functionMap() const;

    bool operator==(const UDQFunctionTable& data) const;

private:
    void insert_function(std::shared_ptr<UDQFunction> func);
    UDQParams params;
    FunctionMap function_table;
};
}
#endif
