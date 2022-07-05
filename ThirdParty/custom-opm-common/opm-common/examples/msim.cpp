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

#include <opm/output/eclipse/EclipseIO.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/msim/msim.hpp>


int main(int /* argc */, char** argv) {
    std::string deck_file = argv[1];
    Opm::Parser parser;
    Opm::ParseContext parse_context;
    Opm::ErrorGuard error_guard;
    auto python = std::make_shared<Opm::Python>();

    Opm::OpmLog::setupSimpleDefaultLogging();
    Opm::Deck deck = parser.parseFile(deck_file, parse_context, error_guard);
    Opm::EclipseState state(deck);
    Opm::Schedule schedule(deck, state, parse_context, error_guard, python);
    Opm::SummaryConfig summary_config(deck, schedule, state.fieldProps(), state.aquifer(),
                                      parse_context, error_guard);

    if (error_guard) {
        error_guard.dump();
        error_guard.terminate();
    }

    Opm::msim msim(state, schedule);
    Opm::EclipseIO io(state, state.getInputGrid(), schedule, summary_config);
    msim.run(io, false);
}

