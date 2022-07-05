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

#include <unordered_set>
#include <fmt/format.h>
#include <opm/input/eclipse/Schedule/Well/PAvgCalculatorCollection.hpp>


namespace Opm {

bool PAvgCalculatorCollection::empty() const {
    return true;
}

bool PAvgCalculatorCollection::has(const std::string& wname) const {
    return this->calculators.count(wname) > 0;
}

const PAvgCalculator& PAvgCalculatorCollection::get(const std::string& wname) const {
    auto iter = this->calculators.find(wname);
    if (iter == this->calculators.end())
        throw std::logic_error(fmt::format("No PAvgCalculator registered for well: {}", wname));

    return iter->second;
}

void PAvgCalculatorCollection::add(const PAvgCalculator& calculator) {
    this->calculators.emplace( calculator.wname(), calculator );
    this->indexlist.reset();
}

const std::vector<std::size_t>& PAvgCalculatorCollection::index_list() const {
    if (!this->indexlist.has_value()) {
        std::unordered_set<std::size_t> il;
        for ( const auto&[_, calculator] : this->calculators) {
            (void)_;
            const auto& calc_il = calculator.index_list();
            il.insert(calc_il.begin(), calc_il.end());
        }
        this->indexlist = std::vector<std::size_t>{ il.begin(), il.end() };
    }
    return this->indexlist.value();
}

void PAvgCalculatorCollection::add_pressure(std::size_t index, double pressure) {
    for (auto& [_, calculator] : this->calculators) {
        (void)_;
        calculator.add_pressure(index, pressure);
    }
}


}
