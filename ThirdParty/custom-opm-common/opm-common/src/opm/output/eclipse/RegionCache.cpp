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

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Connection.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <opm/output/eclipse/RegionCache.hpp>

namespace Opm {
namespace out {

RegionCache::RegionCache(const std::vector<int>& fipnum, const EclipseGrid& grid, const Schedule& schedule) {

    const auto& wells = schedule.getWellsatEnd();
    for (const auto& well : wells) {
        const auto& connections = well.getConnections( );
        for (const auto& c : connections) {
            if (grid.cellActive(c.getI(), c.getJ(), c.getK())) {
                size_t active_index = grid.activeIndex(c.getI(), c.getJ(), c.getK());
                int region_id = fipnum[active_index];
                auto& well_index_list = this->connection_map[ region_id ];
                well_index_list.push_back( { well.name() , active_index } );
            }
        }
    }
}


    const std::vector<std::pair<std::string,size_t>>& RegionCache::connections( int region_id ) const {
        const auto iter = this->connection_map.find( region_id );
        if (iter == this->connection_map.end())
            return this->connections_empty;
        else
            return iter->second;
    }

}
}

