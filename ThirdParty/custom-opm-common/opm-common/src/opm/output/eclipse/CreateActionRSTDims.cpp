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
#include <opm/output/eclipse/VectorItems/action.hpp>

#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/Actdims.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>

#include <chrono>
#include <cstddef>
#include <vector>

namespace {


// (The max number of characters in an action statement / (8 - chars pr string)) * (max over actions of number of lines pr ACTIONX)
std::size_t entriesPerZLACT(const Opm::Actdims& actdims, const Opm::Action::Actions& acts)
{
    int max_char_pr_line = actdims.max_characters();
    std::size_t no_entries_pr_line = ((max_char_pr_line % 8) == 0) ? max_char_pr_line / 8 : (max_char_pr_line / 8) + 1;
    std::size_t no_entries = no_entries_pr_line * acts.max_input_lines();

    return no_entries;
}

// The max number of characters in an action statement * (max over actions of number of lines pr ACTIONX) / (8 - chars pr string)
std::size_t entriesPerLine(const Opm::Actdims& actdims)
{
    int max_char_pr_line = actdims.max_characters();
    std::size_t no_entries_pr_line = ((max_char_pr_line % 8) == 0) ? max_char_pr_line / 8 : (max_char_pr_line / 8) + 1;

    return no_entries_pr_line;
}






} // Anonymous

// #####################################################################
// Public Interface (createUdqDims()) Below Separator
// ---------------------------------------------------------------------

std::size_t Opm::RestartIO::Helpers::entriesPerZACT()
{
    std::size_t no_entries = 4;
    return no_entries;
}

std::size_t Opm::RestartIO::Helpers::entriesPerZACN(const Opm::Actdims& actdims)
{
    return Opm::RestartIO::Helpers::VectorItems::ZACN::ConditionSize * actdims.max_conditions();
}

std::size_t Opm::RestartIO::Helpers::entriesPerIACN(const Opm::Actdims& actdims)
{
    return Opm::RestartIO::Helpers::VectorItems::IACN::ConditionSize * actdims.max_conditions();
}

std::size_t Opm::RestartIO::Helpers::entriesPerSACN(const Opm::Actdims& actdims)
{
    return Opm::RestartIO::Helpers::VectorItems::SACN::ConditionSize * actdims.max_conditions();
}

std::size_t Opm::RestartIO::Helpers::entriesPerIACT()
{
    std::size_t no_entries = 9;
    return no_entries;
}

std::size_t Opm::RestartIO::Helpers::entriesPerSACT()
{
    std::size_t no_entries = 5;
    return no_entries;
}



std::vector<int>
Opm::RestartIO::Helpers::
createActionRSTDims(const Schedule&     sched,
                    const std::size_t   simStep)
{
    const auto& acts = sched[simStep].actions();
    const auto& actdims = sched.runspec().actdims();
    std::vector<int> action_rst_dims(9);

    //No of Actionx keywords
    action_rst_dims[0] = acts.ecl_size();
    action_rst_dims[1] = entriesPerIACT();
    action_rst_dims[2] = entriesPerSACT();
    action_rst_dims[3] = entriesPerZACT();
    action_rst_dims[4] = entriesPerZLACT(actdims, acts);
    action_rst_dims[5] = entriesPerZACN(actdims);
    action_rst_dims[6] = entriesPerIACN(actdims);
    action_rst_dims[7] = entriesPerSACN(actdims);
    action_rst_dims[8] = entriesPerLine(actdims);

    return action_rst_dims;
}
