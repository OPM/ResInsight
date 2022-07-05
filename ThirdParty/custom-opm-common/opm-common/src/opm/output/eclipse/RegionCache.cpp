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

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Well/Connection.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <opm/output/eclipse/RegionCache.hpp>

namespace Opm {
namespace out {

RegionCache::RegionCache(const std::set<std::string>& fip_regions, const FieldPropsManager& fp, const EclipseGrid& grid, const Schedule& schedule) {
    for (const auto& fip_name : fip_regions) {
        const auto& fip_region = fp.get_int(fip_name);

        const auto& wells = schedule.getWellsatEnd();
        for (const auto& well : wells) {
            const auto& connections = well.getConnections( );
            if (connections.empty())
                continue;

            for (const auto& c : connections) {
                if (grid.cellActive(c.getI(), c.getJ(), c.getK())) {
                    size_t active_index = grid.activeIndex(c.getI(), c.getJ(), c.getK());
                    int region_id = fip_region[active_index];
                    auto key = std::make_pair(fip_name, region_id);
                    auto& well_index_list = this->connection_map[ key ];
                    well_index_list.push_back( { well.name() , active_index } );
                }
            }

            const auto& conn0 = connections[0];
            auto region_id = fip_region[grid.activeIndex(conn0.global_index())];
            auto key = std::make_pair(fip_name, region_id);
            this->well_map[ key ].push_back(well.name());
        }
    }
}


    const std::vector<std::pair<std::string,size_t>>& RegionCache::connections( const std::string& region_name, int region_id ) const {
        auto key = std::make_pair(region_name, region_id);
        const auto iter = this->connection_map.find( key );
        if (iter == this->connection_map.end())
            return this->connections_empty;
        else
            return iter->second;
    }


    std::vector<std::string> RegionCache::wells(const std::string& region_name, int region_id) const {
        auto key = std::make_pair(region_name, region_id);
        const auto iter = this->well_map.find( key );
        if (iter == this->well_map.end())
            return {};
        else
            return iter->second;
    }

}
}

