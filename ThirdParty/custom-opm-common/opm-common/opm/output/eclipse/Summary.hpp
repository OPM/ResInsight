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

#include <opm/output/data/Aquifer.hpp>
#include <opm/output/data/InterRegFlowMap.hpp>

#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Well/PAvgCalculatorCollection.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Opm {
    class EclipseGrid;
    class EclipseState;
    class Schedule;
    class SummaryConfig;
    class SummaryState;
    class Inplace;
} // namespace Opm

namespace Opm { namespace data {
    class Wells;
    class GroupAndNetworkValues;
    class InterRegFlowMap;
}} // namespace Opm::data

namespace Opm { namespace out {

class Summary {
public:
    using GlobalProcessParameters = std::map<std::string, double>;
    using RegionParameters = std::map<std::string, std::vector<double>>;
    using BlockValues = std::map<std::pair<std::string, int>, double>;
    using InterRegFlowValues = std::unordered_map<std::string, data::InterRegFlowMap>;

    Summary(const EclipseState&  es,
            const SummaryConfig& sumcfg,
            const EclipseGrid&   grid,
            const Schedule&      sched,
            const std::string&   basename = "",
            const bool           writeEsmry = false);

    ~Summary();

    void add_timestep(const SummaryState& st, const int report_step, bool isSubstep);

    void eval(SummaryState&                      summary_state,
              const int                          report_step,
              const double                       secs_elapsed,
              const data::Wells&                 well_solution,
              const data::GroupAndNetworkValues& group_and_nwrk_solution,
              GlobalProcessParameters            single_values,
              const Inplace&                     initial_inplace,
              const Inplace&                     inplace,
              const PAvgCalculatorCollection&    ,
              const RegionParameters&            region_values = {},
              const BlockValues&                 block_values  = {},
              const data::Aquifers&              aquifers_values = {},
              const InterRegFlowValues&          interreg_flows = {}) const;

    void write(const bool is_final_summary = false) const;

    PAvgCalculatorCollection wbp_calculators(std::size_t report_step) const;

private:
    class SummaryImplementation;
    std::unique_ptr<SummaryImplementation> pImpl_;
};

}} // namespace Opm::out

#endif //OPM_OUTPUT_SUMMARY_HPP
