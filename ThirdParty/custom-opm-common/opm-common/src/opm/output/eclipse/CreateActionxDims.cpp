/*
  Copyright (c) 2018 Equinor ASA
  Copyright (c) 2018 Statoil ASA

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

#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/DoubHEAD.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionX.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#include <chrono>
#include <cstddef>
#include <vector>

namespace {

// The number of actions (no of Actionx)
std::size_t noOfActions(const Opm::Action::Actions& acts)
{
    std::size_t no_entries = acts.size();
    return no_entries;
}
    
    
std::size_t entriesPerIACT()
{
    std::size_t no_entries = 9;
    return no_entries;
}

std::size_t entriesPerSACT()
{
    std::size_t no_entries = 5;
    return no_entries;
}

std::size_t entriesPerZACT()
{
    std::size_t no_entries = 4;
    return no_entries;
}

// (The max number of characters in an action statement / (8 - chars pr string)) * (max over actions of number of lines pr ACTIONX)
std::size_t entriesPerZLACT(const Opm::Runspec& rspec, const Opm::Action::Actions& acts)
{
    int max_char_pr_line = rspec.actdims().max_characters();
    std::size_t no_entries_pr_line = ((max_char_pr_line % 8) == 0) ? max_char_pr_line / 8 : (max_char_pr_line / 8) + 1;
    std::size_t no_entries = no_entries_pr_line * acts.max_input_lines();

    return no_entries;
}

// The max number of characters in an action statement * (max over actions of number of lines pr ACTIONX) / (8 - chars pr string)
std::size_t entriesPerLine(const Opm::Runspec& rspec)
{
    int max_char_pr_line = rspec.actdims().max_characters();
    std::size_t no_entries_pr_line = ((max_char_pr_line % 8) == 0) ? max_char_pr_line / 8 : (max_char_pr_line / 8) + 1;

    return no_entries_pr_line;
}


std::size_t entriesPerZACN(const Opm::Runspec& rspec)
{
    //(Max number of conditions pr ACTIONX) * ((max no characters pr line = 104) / (8 - characters pr string)(104/8 = 13)
    std::size_t no_entries = rspec.actdims().max_conditions() * 13;
    return no_entries;
}

std::size_t entriesPerIACN(const Opm::Runspec& rspec)
{
    //26*Max number of conditions pr ACTIONX 
    std::size_t no_entries = 26 * rspec.actdims().max_conditions();
    return no_entries;
}

std::size_t entriesPerSACN(const Opm::Runspec& rspec)
{
    //16 
    std::size_t no_entries = 16 * rspec.actdims().max_conditions();
    return no_entries;
}



} // Anonymous

// #####################################################################
// Public Interface (createUdqDims()) Below Separator
// ---------------------------------------------------------------------

std::vector<int>
Opm::RestartIO::Helpers::
createActionxDims(  const Runspec& rspec,
                    const Schedule&     sched,
                    const std::size_t       simStep)
{
    const auto& acts = sched.actions(simStep);
    std::vector<int> actDims(9);
    
    //No of Actionx keywords
    actDims[0] = noOfActions(acts);
    actDims[1] = entriesPerIACT();
    actDims[2] = entriesPerSACT();
    actDims[3] = entriesPerZACT();
    actDims[4] = entriesPerZLACT(rspec, acts);
    actDims[5] = entriesPerZACN(rspec);
    actDims[6] = entriesPerIACN(rspec);
    actDims[7] = entriesPerSACN(rspec);
    actDims[8] = entriesPerLine(rspec);
    
    return actDims;
}
