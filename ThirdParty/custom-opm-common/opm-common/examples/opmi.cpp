/*
  Copyright 2013 Statoil ASA.

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
#include <iomanip>
#include <chrono>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/StreamLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/common/utility/TimeService.hpp>


void initLogging() {
    std::shared_ptr<Opm::StreamLog> cout_log = std::make_shared<Opm::StreamLog>(std::cout, Opm::Log::DefaultMessageTypes);
    Opm::OpmLog::addBackend( "COUT" , cout_log);
}

inline void loadDeck( const char * deck_file) {
    Opm::ParseContext parseContext;
    Opm::ErrorGuard errors;
    Opm::Parser parser;
    auto python = std::make_shared<Opm::Python>();

    std::cout << "Loading deck: " << deck_file << " ..... "; std::cout.flush();

    Opm::time_point start;

    start = Opm::TimeService::now();
    auto deck = parser.parseFile(deck_file, parseContext, errors);
    auto deck_time = Opm::TimeService::now() - start;

    std::cout << "parse complete - creating EclipseState .... ";  std::cout.flush();

    start = Opm::TimeService::now();
    Opm::EclipseState state( deck );
    auto state_time = Opm::TimeService::now() - start;

    std::cout << "creating Schedule .... ";  std::cout.flush();

    start = Opm::TimeService::now();
    Opm::Schedule schedule( deck, state, python);
    auto schedule_time = Opm::TimeService::now() - start;

    std::cout << "creating SummaryConfig .... ";  std::cout.flush();

    start = Opm::TimeService::now();
    Opm::SummaryConfig summary( deck, schedule, state.fieldProps(), state.aquifer(),
                                parseContext, errors );
    auto summary_time = Opm::TimeService::now() - start;

    std::cout << "complete." << std::endl << std::endl;
    std::cout << "Time: " << std::endl;
    std::cout << "   deck.....: "  << std::chrono::duration<double>(deck_time).count()  << " seconds" << std::endl;
    std::cout << "   state....: "  << std::chrono::duration<double>(state_time).count()  << " seconds" << std::endl;
    std::cout << "   schedule.: "  << std::chrono::duration<double>(schedule_time).count()  << " seconds" << std::endl;
    std::cout << "   summary..: "  << std::chrono::duration<double>(summary_time).count()  << " seconds" << std::endl;
}


int main(int argc, char** argv) {
    initLogging();
    for (int iarg = 1; iarg < argc; iarg++)
        loadDeck( argv[iarg] );
}

