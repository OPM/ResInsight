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

#ifndef OPM_OUTPUT_SUMMARY_HPP
#define OPM_OUTPUT_SUMMARY_HPP

#include <opm/parser/eclipse/EclipseState/Schedule/Group/Group.hpp>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Opm {
    class EclipseGrid;
    class EclipseState;
    class Schedule;
    class SummaryConfig;
    class SummaryState;
} // namespace Opm

namespace Opm { namespace data {
    class WellRates;
    class GroupValues;
}} // namespace Opm::data

namespace Opm { namespace out {

class Summary {
public:
    using GlobalProcessParameters = std::map<std::string, double>;
    using RegionParameters = std::map<std::string, std::vector<double>>;
    using BlockValues = std::map<std::pair<std::string, int>, double>;

    Summary(const EclipseState&  es,
            const SummaryConfig& sumcfg,
            const EclipseGrid&   grid,
            const Schedule&      sched,
            const std::string&   basename = "");

    ~Summary();

    void add_timestep(const SummaryState& st, const int report_step);

    void eval(SummaryState&                  summary_state,
              const int                      report_step,
              const double                   secs_elapsed,
              const EclipseState&            es,
              const Schedule&                schedule,
              const data::WellRates&         well_solution,
              const data::GroupValues&       group_solution,
              const GlobalProcessParameters& single_values,
              const RegionParameters&        region_values = {},
              const BlockValues&             block_values  = {}) const;

    void write() const;

    
private:
    class SummaryImplementation;
    std::unique_ptr<SummaryImplementation> pImpl_;
};

}} // namespace Opm::out

#endif //OPM_OUTPUT_SUMMARY_HPP
