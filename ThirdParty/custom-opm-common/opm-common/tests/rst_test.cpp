/*
  Copyright 2020 Equinor ASA

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

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/RestartFileView.hpp>
#include <opm/io/eclipse/rst/state.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/StreamLog.hpp>

void initLogging() {
    std::shared_ptr<Opm::StreamLog> cout_log = std::make_shared<Opm::StreamLog>(std::cout, Opm::Log::DefaultMessageTypes);
    Opm::OpmLog::addBackend( "COUT" , cout_log);
}

/*
  This is a small test application which can be used to check that the Schedule
  object is correctly initialized from a restart file. The program can take
  either one or two commandline arguments:

     rst_test  RESTART_CASE.DATA

  We just verify that the Schedule object can be initialized from
  RESTART_CASE.DATA.

      rst_test CASE.DATA RESTART_CASE.DATA

  The Schedule object initialized from the restart file and the Schedule object
  initialized from the normal case are compared. The restart time configured in
  the second .DATA file must be within the time range covered by the first .DATA
  file.

  In both cases the actual restart file pointed to by the RESTART_CASE.DATA file
  must also be present.
*/


std::pair<Opm::EclipseState, Opm::Schedule> load_schedule(std::shared_ptr<const Opm::Python> python, const std::string& fname, int& report_step) {
    Opm::Parser parser;
    auto deck = parser.parseFile(fname);
    Opm::EclipseState state(deck);

    const auto& init_config = state.getInitConfig();
    if (init_config.restartRequested()) {
        report_step = init_config.getRestartStep();
        const auto& rst_filename = state.getIOConfig().getRestartFileName( init_config.getRestartRootName(), report_step, false );
        auto rst_file = std::make_shared<Opm::EclIO::ERst>(rst_filename);
        auto rst_view = std::make_shared<Opm::EclIO::RestartFileView>(std::move(rst_file), report_step);

        const auto rst = Opm::RestartIO::RstState::load(std::move(rst_view), state.runspec(), parser);
        return {
            std::piecewise_construct,
            std::forward_as_tuple(state),
            std::forward_as_tuple(deck, state, python, std::nullopt, &rst)
        };
    } else
        return {
            std::piecewise_construct,
            std::forward_as_tuple(state),
            std::forward_as_tuple(deck, state, python)
        };
}

std::pair<Opm::EclipseState, Opm::Schedule> load_schedule(std::shared_ptr<const Opm::Python> python, const std::string& fname) {
    int report_step;
    return load_schedule(python, fname, report_step);
}



int main(int argc, char ** argv) {
    auto python = std::make_shared<Opm::Python>();
    initLogging();
    if (argc == 2)
        load_schedule(python, argv[1]);
    else {
        bool equal = true;
        int report_step;
        const auto& [state, sched] = load_schedule(python, argv[1]);
        const auto& [rst_state, rst_sched] = load_schedule(python, argv[2], report_step);


        if (Opm::EclipseState::rst_cmp(state, rst_state))
            std::cout << "EclipseState objects were equal!" << std::endl;
        else {
            std::cout << "EclipseState objects were different!" << std::endl;
            equal = false;
        }

        if (Opm::Schedule::cmp(sched, rst_sched, static_cast<std::size_t>(report_step)) )
            std::cout << "Schedule objects were equal!" << std::endl;
        else {
            std::cout << "Differences were encountered between the Schedule objects" << std::endl;
            equal = false;
        }

        if (!equal)
            std::exit( EXIT_FAILURE );
    }
}
