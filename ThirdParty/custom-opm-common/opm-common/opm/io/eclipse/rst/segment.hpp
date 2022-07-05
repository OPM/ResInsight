/*
  Copyright 2020 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RST_SEGMENT
#define RST_SEGMENT

#include <vector>
#include <algorithm>

#include <opm/input/eclipse/Schedule/MSW/Segment.hpp>
#include <opm/input/eclipse/Schedule/MSW/icd.hpp>

namespace Opm {
class UnitSystem;

namespace RestartIO {

struct RstSegment {
    RstSegment(const ::Opm::UnitSystem& unit_system, int segment_number, const int* iseg, const double * rseg);

    int segment;
    int outlet_segment;
    int branch;
    Segment::SegmentType segment_type;
    int icd_scaling_mode;
    ICDStatus icd_status;

    double dist_outlet;
    double outlet_dz;
    double diameter;
    double roughness;
    double area;
    double volume;
    double dist_bhp_ref;
    double bhp_ref_dz;
    double total_flow;
    double water_flow_fraction;
    double gas_flow_fraction;
    double pressure;
    double valve_length;
    double valve_area;
    double valve_flow_coeff;
    double valve_max_area;
    double base_strength;
    double fluid_density;
    double fluid_viscosity;
    double critical_water_fraction;
    double transition_region_width;
    double max_emulsion_ratio;
    double max_valid_flow_rate;
    double icd_length;
    double valve_area_fraction;

    std::vector<int> inflow_segments;
};


}
}




#endif
