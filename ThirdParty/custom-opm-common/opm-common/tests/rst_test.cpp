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
#include <iostream>
#include <unordered_map>
#include <vector>

#include <opm/io/eclipse/rst/state.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/StreamLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>

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


Opm::Schedule load_schedule(std::shared_ptr<const Opm::Python> python, const std::string& fname, int& report_step) {
    Opm::Parser parser;
    auto deck = parser.parseFile(fname);
    Opm::EclipseState state(deck);

    const auto& init_config = state.getInitConfig();
    if (init_config.restartRequested()) {
        report_step = init_config.getRestartStep();
        const auto& rst_filename = state.getIOConfig().getRestartFileName( init_config.getRestartRootName(), report_step, false );
        Opm::EclIO::ERst rst_file(rst_filename);

        const auto& rst = Opm::RestartIO::RstState::load(rst_file, report_step);
        return Opm::Schedule(deck, state, python, &rst);
    } else
        return Opm::Schedule(deck, state, python);
}

Opm::Schedule load_schedule(std::shared_ptr<const Opm::Python> python, const std::string& fname) {
    int report_step;
    return load_schedule(python, fname, report_step);
}



int main(int argc, char ** argv) {
    auto python = std::make_shared<Opm::Python>();
    initLogging();
    if (argc == 2)
        load_schedule(python, argv[1]);
    else {
        int report_step;
        const auto& sched = load_schedule(python, argv[1]);
        const auto& rst_sched = load_schedule(python, argv[2], report_step);

        if (Opm::Schedule::cmp(sched, rst_sched, report_step) ) {
            std::cout << "Schedule objects were equal!" << std::endl;
            std::exit( EXIT_SUCCESS );
        } else {
            std::cout << "Differences were encountered between the Schedule objects" << std::endl;
            std::exit( EXIT_FAILURE );
        }
    }
}
