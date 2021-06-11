/*
  Copyright 2018 Statoil ASA.

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
#include <stdexcept>
#include <algorithm>

#include <cassert>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellTestConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellTestState.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>

namespace Opm {

    void WellTestState::closeWell(const std::string& well_name, WellTestConfig::Reason reason, double sim_time) {

        WTestWell* well_ptr = getWell(well_name, reason);

        if (well_ptr) {
            if (well_ptr->closed) {
                throw std::runtime_error( " Well " + well_name + " is closed with reason "
                                        + WellTestConfig::reasonToString(reason)
                                        + ", we are trying to close it again with same reason!");
            }
            // the well exists already, we just update it with action of closing
            well_ptr->closed = true;
            well_ptr->last_test = sim_time;
        } else {
            // by default, we use -1 if there is no WTEST request for this well
            // it will be updated when checking for WellTestConfig
            this->wells.push_back({well_name, reason, true, sim_time, 0, -1});
        }
    }


    void WellTestState::openWell(const std::string& well_name, WellTestConfig::Reason reason) {

        WTestWell* well_ptr = getWell(well_name, reason);

        if (well_ptr)
            well_ptr->closed = false;
        else
            throw std::runtime_error("No well named " + well_name + " with close reason "
                                    + WellTestConfig::reasonToString(reason)
                                    + " found in WellTestState.");
    }


    void WellTestState::openWell(const std::string& well_name) {
        for (auto& well : wells)
            if (well.name == well_name)
                well.closed = false;
    }

    bool WellTestState::hasWellClosed(const std::string& well_name) const {
        for (const auto& well : wells)
            if (well.name == well_name && well.closed)
                return true;

        return false;
    }


    bool WellTestState::hasWellClosed(const std::string& well_name, WellTestConfig::Reason reason) const {
        const auto well_iter = std::find_if(wells.begin(),
                                            wells.end(),
                                            [&well_name, &reason](const WTestWell& well)
                                            {
                                                return (reason == well.reason && well.name == well_name && well.closed);
                                            });
        return (well_iter != wells.end());
    }


    WellTestState::WTestWell* WellTestState::getWell(const std::string& well_name, WellTestConfig::Reason reason)
    {
        const auto well_iter = std::find_if(wells.begin(), wells.end(), [&well_name, &reason](const WTestWell& well) {
            return (reason == well.reason && well.name == well_name);
        });

        return (well_iter == wells.end() ? nullptr : std::addressof(*well_iter) );
    }


    size_t WellTestState::sizeWells() const {
        return this->wells.size();
    }

    std::vector<std::pair<std::string, WellTestConfig::Reason>>
    WellTestState::updateWells(const WellTestConfig& config,
                               const std::vector<Well>& wells_ecl,
                               double sim_time) {
        std::vector<std::pair<std::string, WellTestConfig::Reason>> output;

        updateForNewWTEST(config);

        for (auto& well : this->wells) {
            auto well_ecl = std::find_if(wells_ecl.begin(), wells_ecl.end(),
                    [&well](const Well& w)
                    {
                        return w.name() == well.name;
                    });

            if (well_ecl == wells_ecl.end())
                throw std::runtime_error("No well named " + well.name + " is found in wells_ecl.");

            // if the well is SHUT, we do not consider to test it
            if (well_ecl->getStatus() != Well::Status::OPEN)
                continue;

            if (well.closed && config.has(well.name, well.reason)) {
                const auto& well_config = config.get(well.name, well.reason);
                double elapsed = sim_time - well.last_test;

                if (elapsed >= well_config.test_interval)
                    if (well_config.num_test == 0 || (well.num_attempt < well_config.num_test)) {
                        well.last_test = sim_time;
                        well.num_attempt ++;
                        output.emplace_back(std::make_pair(well.name, well.reason));
                        if ( (well_config.num_test != 0) && (well.num_attempt >= well_config.num_test) ) {
                            OpmLog::info(well.name + " will be tested for " + WellTestConfig::reasonToString(well.reason)
                                        + " reason for the last time! " );
                        }
                    }
            }
        }
        return output;
    }


    void WellTestState::addClosedCompletion(const std::string& well_name, int complnum, double sim_time) {
        if (this->hasCompletion(well_name, complnum))
            return;

        this->completions.push_back( {well_name, complnum, sim_time, 0} );
    }


    void WellTestState::dropCompletion(const std::string& well_name, int complnum) {
        completions.erase(std::remove_if(completions.begin(),
                                         completions.end(),
                                         [&well_name, complnum](const ClosedCompletion& completion) { return (completion.wellName == well_name && completion.complnum == complnum); }),
                          completions.end());
    }


    bool WellTestState::hasCompletion(const std::string& well_name, const int complnum) const {
        const auto completion_iter = std::find_if(completions.begin(),
                                                  completions.end(),
                                                  [&well_name, &complnum](const ClosedCompletion& completion)
                                                  {
                                                    return (complnum == completion.complnum && completion.wellName == well_name);
                                                  });
        return (completion_iter != completions.end());
    }

    size_t WellTestState::sizeCompletions() const {
        return this->completions.size();
    }

    std::vector<std::pair<std::string, int>> WellTestState::updateCompletion(const WellTestConfig& config, double sim_time) {
        std::vector<std::pair<std::string, int>> output;
        for (auto& closed_completion : this->completions) {
            if (config.has(closed_completion.wellName, WellTestConfig::Reason::COMPLETION)) {
                const auto& well_config = config.get(closed_completion.wellName, WellTestConfig::Reason::COMPLETION);
                double elapsed = sim_time - closed_completion.last_test;

                if (elapsed >= well_config.test_interval)
                    if (well_config.num_test == 0 || (closed_completion.num_attempt < well_config.num_test)) {
                        closed_completion.last_test = sim_time;
                        closed_completion.num_attempt += 1;
                        output.push_back(std::make_pair(closed_completion.wellName, closed_completion.complnum));
                    }
            }
        }
        return output;
    }

    double WellTestState::lastTestTime(const std::string& well_name) const {
        const auto well_iter = std::find_if(wells.begin(),
                                            wells.end(),
                                            [&well_name](const WTestWell& well)
                                            {
                                                return (well.name == well_name);
                                            });
        if (well_iter == wells.end())
            throw std::runtime_error("No well named " + well_name + " is found in WellTestState.");

        return well_iter->last_test;
    }

    void WellTestState::updateForNewWTEST(const Opm::WellTestConfig& config)
    {
        // check whether to reset based on the new WTEST request
        for (auto& well: this->wells) {
            if (config.has(well.name, well.reason)) {
                const auto& well_config = config.get(well.name, well.reason);
                if (well_config.begin_report_step > well.wtest_report_step) {
                    // there is a new WTEST input, we should reset the counting
                    well.num_attempt = 0;
                    well.wtest_report_step = well_config.begin_report_step;
                }
                if (well_config.begin_report_step != well.wtest_report_step)
                    throw std::logic_error(" Bug in OPM/flow when using WTEST information related to well " + well.name);

            } else {
                // If there is WTEST step, due to new WTEST input, which does not specify any testing closure cause,
                // there is no WTEST request anymore.
                // If there is no WTEST step, then everything stay the same.
                if (well.wtest_report_step >= 0) {
                    well.wtest_report_step = -1;
                    well.num_attempt = 0;
                }
                if (well.wtest_report_step != -1 || well.num_attempt != 0)
                    throw std::logic_error(" Bugs in OPM/flow when there is WTEST request for well " + well.name);

            }
        }
    }

}


