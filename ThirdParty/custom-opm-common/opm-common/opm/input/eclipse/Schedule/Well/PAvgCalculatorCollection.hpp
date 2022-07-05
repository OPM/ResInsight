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


#ifndef PAVE_CALC_COLLECTIONHPP
#define PAVE_CALC_COLLECTIONHPP

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <opm/input/eclipse/Schedule/Well/PAvg.hpp>
#include <opm/input/eclipse/Schedule/Well/PAvgCalculator.hpp>


namespace Opm {

class PAvgCalculatorCollection {
public:
    bool empty() const;
    void add(const PAvgCalculator& calculator);
    bool has(const std::string& wname) const;
    const PAvgCalculator& get(const std::string& wname) const;
    const std::vector<std::size_t>& index_list() const;
    void add_pressure(std::size_t index, double pressure);
private:
    std::unordered_map<std::string, PAvgCalculator> calculators;
    mutable std::optional<std::vector<std::size_t>> indexlist;
};

}
#endif
