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

#ifndef RST_HEADER
#define RST_HEADER

#include <vector>
#include <ctime>
#include <cstddef>

#include <opm/input/eclipse/EclipseState/Runspec.hpp>

namespace Opm {
class UnitSystem;

namespace RestartIO {

struct RstHeader {
    RstHeader(const Runspec& runspec, const UnitSystem& unit_system, const std::vector<int>& intehead, const std::vector<bool>& logihead, const std::vector<double>& doubhead);

    Runspec runspec;
    int nx;
    int ny;
    int nz;
    int nactive;
    int num_wells;
    int ncwmax;
    int max_wells_in_group;
    int max_groups_in_field;
    int max_wells_in_field;
    int year;
    int month;
    int mday;
    int hour;
    int minute;
    int microsecond;
    int phase_sum;
    int niwelz;
    int nswelz;
    int nxwelz;
    int nzwelz;
    int niconz;
    int nsconz;
    int nxconz;
    int nigrpz;
    int nsgrpz;
    int nxgrpz;
    int nzgrpz;
    int ncamax;
    int niaaqz;
    int nsaaqz;
    int nxaaqz;
    int nicaqz;
    int nscaqz;
    int nacaqz;
    int tstep;
    int report_step;
    int newtmx;
    int newtmn;
    int litmax;
    int litmin;
    int mxwsit;
    int mxwpit;
    int version;
    int iprog;
    int nsegwl;
    int nswlmx;
    int nsegmx;
    int nlbrmx;
    int nisegz;
    int nrsegz;
    int nilbrz;
    int ntfip ;
    int nmfipr;
    int ngroup;
    int nwgmax;
    int nwell_udq;
    int ngroup_udq;
    int nfield_udq;
    int num_action;
    int guide_rate_nominated_phase;
    int max_wlist;

    bool e300_radial;
    bool e100_radial;
    bool enable_hysteris;
    bool enable_msw;
    bool is_live_oil;
    bool is_wet_gas;
    bool const_comp_oil;
    bool dir_relperm;
    bool reversible_relperm;
    bool endscale;
    bool dir_eps;
    bool reversible_eps;
    bool alt_eps;
    bool group_control_active;
    bool glift_all_nupcol;

    double next_timestep1;
    double next_timestep2;
    double max_timestep;
    double guide_rate_a;
    double guide_rate_b;
    double guide_rate_c;
    double guide_rate_d;
    double guide_rate_e;
    double guide_rate_f;
    double guide_rate_delay;
    double guide_rate_damping;
    double udq_range;
    double udq_undefined;
    double udq_eps;
    double glift_min_wait;
    double glift_rate_delta;
    double glift_min_eco_grad;


    std::time_t sim_time() const;
    std::pair<std::time_t, std::size_t> restart_info() const;
    int num_udq() const;
};


}
}




#endif
