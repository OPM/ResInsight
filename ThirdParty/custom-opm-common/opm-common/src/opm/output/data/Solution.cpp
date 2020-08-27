/*
  Copyright 2016 Statoil ASA.

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

#include <algorithm>

#include <opm/output/data/Solution.hpp>
#include <opm/output/data/Cells.hpp>


namespace Opm {
namespace data {

Solution::Solution( bool init_si ) : si( init_si ) {}

bool Solution::has(const std::string& keyword) const {
    return this->count( keyword ) > 0;
}

std::vector<double>& Solution::data(const std::string& keyword) {
    return this->at( keyword ).data;
}

const std::vector<double>& Solution::data(const std::string& keyword) const {
    return this->at( keyword ).data;
}

std::pair< Solution::iterator, bool > Solution::insert( std::string name,
                                                        UnitSystem::measure m,
                                                        std::vector< double > xs,
                                                        TargetType type ) {

    return this->emplace( name, CellData{ m, std::move( xs ), type } );
}

void data::Solution::convertToSI( const UnitSystem& units ) {
    if (this->si) return;

    for( auto& elm : *this ) {
        UnitSystem::measure dim = elm.second.dim;
        if (dim != UnitSystem::measure::identity)
            units.to_si( dim , elm.second.data );
    }

    this->si = true;
}

void data::Solution::convertFromSI( const UnitSystem& units ) {
    if (!this->si) return;

    for (auto& elm : *this ) {
        UnitSystem::measure dim = elm.second.dim;
        if (dim != UnitSystem::measure::identity)
            units.from_si( dim , elm.second.data );
    }

    this->si = false;
}

}
}

