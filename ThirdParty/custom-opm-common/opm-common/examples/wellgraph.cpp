/*
  Copyright 2013, 2020 Equinor ASA.

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

#include <fstream>
#include <iomanip>
#include <chrono>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/StreamLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/common/utility/FileSystem.hpp>


inline void createDot(const Opm::Schedule& schedule, const std::string& casename)
{
    std::cout << "Writing " << casename << ".gv .... ";  std::cout.flush();
    std::ofstream os(casename + ".gv");
    os << "// This file was written by the 'wellgraph' utility from OPM.\n";
    os << "// Find the source code at github.com/OPM.\n";
    os << "// Convert output to PDF with 'dot -Tpdf " << casename << ".gv > " << casename << ".pdf'\n";
    os << "strict digraph \"" << casename << "\"\n{\n";
    const auto groupnames = schedule.groupNames();
    const std::size_t last = schedule.getTimeMap().last();
    // Group -> Group relations.
    for (const auto& gn : groupnames) {
        const auto& g = schedule.getGroup(gn, last);
        const auto& children = g.groups();
        if (!children.empty()) {
            os << "    \"" << gn << "\" -> {";
            for (const auto& child : children) {
                os << " \"" << child << '"';
            }
            os << " }\n";
        }
    }
    // Group -> Well relations.
    os << "    node [shape=box]\n";
    for (const auto& gn : groupnames) {
        const auto& g = schedule.getGroup(gn, last);
        const auto& children = g.wells();
        if (!children.empty()) {
            os << "    \"" << gn << "\" -> {";
            for (const auto& child : children) {
                os << " \"" << child << '"';
            }
            os << " }\n";
        }
    }
    // Color wells by injector or producer.
    for (const auto& w : schedule.getWellsatEnd()) {
        os << "    \"" << w.name() << '"';
        if (w.isProducer() && w.isInjector()) {
            os << " [color=purple]\n";
        } else if (w.isProducer()) {
            os << " [color=red]\n";
        } else {
            os << " [color=blue]\n";
        }
    }
    os << "}\n";
    std::cout << "complete." << std::endl;
}


inline Opm::Schedule loadSchedule(const std::string& deck_file)
{
    Opm::ParseContext parseContext({{Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputError::IGNORE},
                                    {Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputError::WARN},
                                    {Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputError::WARN},
                                    {Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputError::WARN}});
    Opm::ErrorGuard errors;
    Opm::Parser parser;
    auto python = std::make_shared<Opm::Python>();

    std::cout << "Loading and parsing deck: " << deck_file << " ..... "; std::cout.flush();
    auto deck = parser.parseFile(deck_file, parseContext, errors);
    std::cout << "complete.\n";

    std::cout << "Creating EclipseState .... ";  std::cout.flush();
    Opm::EclipseState state( deck );
    std::cout << "complete.\n";

    std::cout << "Creating Schedule .... ";  std::cout.flush();
    Opm::Schedule schedule( deck, state, python);
    std::cout << "complete." << std::endl;

    return schedule;
}


int main(int argc, char** argv)
{
    std::ostringstream os;
    std::shared_ptr<Opm::StreamLog> string_log = std::make_shared<Opm::StreamLog>(os, Opm::Log::DefaultMessageTypes);
    Opm::OpmLog::addBackend( "STRING" , string_log);
    try {
        for (int iarg = 1; iarg < argc; iarg++) {
            const std::string filename = argv[iarg];
            const auto sched = loadSchedule(filename);
            const auto casename = Opm::filesystem::path(filename).stem();
            createDot(sched, casename);
        }
    } catch (const std::exception& e) {
        std::cout << "\n\n***** Caught an exception: " << e.what() << std::endl;
        std::cout << "\n\n***** Printing log: "<< std::endl;
        std::cout << os.str();
        std::cout << "\n\n***** Exiting due to errors." << std::endl;
    }
}

