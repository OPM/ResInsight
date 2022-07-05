/*
  Copyright 2018 Equinor ASA.

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

#include <iostream>
#include <utility>

#include <opm/output/eclipse/Inplace.hpp>
#include <opm/output/eclipse/EclipseIO.hpp>
#include <opm/output/eclipse/RestartValue.hpp>
#include <opm/output/eclipse/Summary.hpp>
#include <opm/output/data/Solution.hpp>
#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Groups.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/Schedule/Action/ActionContext.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/msim/msim.hpp>

namespace Opm {

msim::msim(const EclipseState& state_arg, const Schedule& schedule_arg)
    : state(state_arg)
    , schedule(schedule_arg)
    , st(TimeService::from_time_t(this->schedule.getStartTime()))
{}

void msim::run(EclipseIO& io, bool report_only) {
    const double week = 7 * 86400;
    data::Solution sol;
    UDQState udq_state(this->schedule.getUDQConfig(0).params().undefinedValue());
    WellTestState wtest_state;
    Python python;

    io.writeInitial();
    for (size_t report_step = 1; report_step < schedule.size(); report_step++) {
        data::Wells well_data;
        data::GroupAndNetworkValues group_nwrk_data;
        if (report_only)
            run_step(wtest_state, udq_state, sol, well_data, group_nwrk_data, report_step, io);
        else {
            double time_step = std::min(week, 0.5*schedule.stepLength(report_step - 1));
            run_step(wtest_state, udq_state, sol, well_data, group_nwrk_data, report_step, time_step, io);
        }
        auto sim_time = TimeService::from_time_t( schedule.simTime(report_step) );
        post_step(sol, well_data, group_nwrk_data, report_step, sim_time);
        const auto& exit_status = schedule.exitStatus();
        if (exit_status.has_value())
            return;
    }
}

UDAValue msim::uda_val() {
 return UDAValue();
}


void msim::post_step(data::Solution& /* sol */, data::Wells& /* well_data */, data::GroupAndNetworkValues& /* grp_nwrk_data */, size_t report_step, const time_point& sim_time) {
    const auto& actions = this->schedule[report_step].actions.get();
    if (actions.empty())
        return;

    Action::Context context( this->st , this->schedule[report_step].wlist_manager.get());

    for (const auto& action : actions.pending(this->action_state, std::chrono::system_clock::to_time_t(sim_time))) {
        auto result = action->eval(context);
        if (result)
            this->schedule.applyAction(report_step, *action, result.wells(), {});
    }

    for (const auto& pyaction : actions.pending_python(this->action_state))
        this->schedule.runPyAction(report_step, *pyaction, this->action_state, this->state, this->st);
}



void msim::run_step(WellTestState& wtest_state, UDQState& udq_state, data::Solution& sol, data::Wells& well_data, data::GroupAndNetworkValues& grp_nwrk_data, size_t report_step, EclipseIO& io) {
    this->run_step(wtest_state, udq_state, sol, well_data, grp_nwrk_data, report_step, schedule.stepLength(report_step - 1), io);
}


void msim::run_step(WellTestState& wtest_state, UDQState& udq_state, data::Solution& sol, data::Wells& well_data, data::GroupAndNetworkValues& group_nwrk_data, size_t report_step, double dt, EclipseIO& io) {
    double start_time = this->schedule.seconds(report_step - 1);
    double end_time = this->schedule.seconds(report_step);
    double seconds_elapsed = start_time;

    while (seconds_elapsed < end_time) {
        double time_step = dt;
        if ((seconds_elapsed + time_step) > end_time)
            time_step = end_time - seconds_elapsed;

        this->simulate(sol, well_data, group_nwrk_data, report_step, seconds_elapsed, time_step);

        seconds_elapsed += time_step;

        io.summary().eval(this->st,
                          report_step,
                          seconds_elapsed,
                          well_data,
                          group_nwrk_data,
                          {},
                          {},
                          {},
                          {});

        this->schedule.getUDQConfig( report_step ).eval(report_step, schedule.wellMatcher(report_step), this->st, udq_state);

        this->output(wtest_state,
                     udq_state,
                     report_step,
                     (seconds_elapsed < end_time),
                     seconds_elapsed,
                     sol,
                     well_data,
                     group_nwrk_data,
                     io);
    }
}



void msim::output(WellTestState& wtest_state, const UDQState& udq_state, size_t report_step, bool substep, double seconds_elapsed, const data::Solution& sol, const data::Wells& well_data, const data::GroupAndNetworkValues& group_nwrk_data, EclipseIO& io) {
    RestartValue value(sol, well_data, group_nwrk_data, {});
    io.writeTimeStep(this->action_state,
                     wtest_state,
                     this->st,
                     udq_state,
                     report_step,
                     substep,
                     seconds_elapsed,
                     std::move(value));
}


void msim::simulate(data::Solution& sol, data::Wells& well_data, data::GroupAndNetworkValues& /* group_nwrk_data */, size_t report_step, double seconds_elapsed, double time_step) {
    for (const auto& sol_pair : this->solutions) {
        auto func = sol_pair.second;
        func(this->state, this->schedule, sol, report_step, seconds_elapsed + time_step);
    }

    for (const auto& well_pair : this->well_rates) {
        const std::string& well_name = well_pair.first;
        const auto& sched_well = this->schedule.getWell(well_name, report_step);
        bool well_open = (sched_well.getStatus() == Well::Status::OPEN);

        data::Well& well = well_data[well_name];
        for (const auto& rate_pair : well_pair.second) {
            auto rate = rate_pair.first;
            auto func = rate_pair.second;

            if (well_open)
                well.rates.set(rate, func(this->state, this->schedule, this->st, sol, report_step, seconds_elapsed + time_step));
            else
                well.rates.set(rate, 0.0);
        }

        // This is complete bogus; a temporary fix to pass an assert() in the
        // the restart output.
        well.connections.resize(100);
    }
}


void msim::well_rate(const std::string& well, data::Rates::opt rate, std::function<well_rate_function> func) {
    this->well_rates[well][rate] = func;
}


void msim::solution(const std::string& field, std::function<solution_function> func) {
    this->solutions[field] = func;
}


}


