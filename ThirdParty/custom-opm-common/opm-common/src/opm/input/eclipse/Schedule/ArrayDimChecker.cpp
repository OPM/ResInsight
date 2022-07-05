/*
  Copyright (c) 2019 Equinor ASA

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
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <sstream>

#include <fmt/format.h>

#include <opm/common/OpmLog/KeywordLocation.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Schedule/ArrayDimChecker.hpp>


namespace {
    namespace WellDims {
        void checkNumWells(const Opm::Welldims&     wdims,
                           const Opm::Schedule&     sched,
                           const Opm::ParseContext& ctxt,
                           Opm::ErrorGuard&         guard)
        {
            const auto nWells = sched.numWells();

            if (nWells > std::size_t(wdims.maxWellsInField()))
            {
                const auto& location = wdims.location();
                if (location) {
                    std::string fmt_message = fmt::format("Problem with keyword {{keyword}}\n"
                                                          "In {{file}} line {{line}}\n"
                                                          "The case has {0} wells - but only {1} are specified in WELLDIMS.\n"
                                                          "Please increase item 1 in WELLDIMS to at least {1}", nWells, wdims.maxWellsInField());

                    ctxt.handleError(Opm::ParseContext::RUNSPEC_NUMWELLS_TOO_LARGE,
                                     fmt_message, *location, guard);
                } else {
                    std::string msg = fmt::format("The case does not have a WELLDIMS keyword.\n"
                                                  "Please add a WELLDIMS keyword in the RUNSPEC section specifying at least {} wells in item 1.", nWells);
                    ctxt.handleError(Opm::ParseContext::RUNSPEC_NUMWELLS_TOO_LARGE, msg, {}, guard);
                }
            }
        }

        void checkConnPerWell(const Opm::Welldims&     wdims,
                              const Opm::Schedule&     sched,
                              const Opm::ParseContext& ctxt,
                              Opm::ErrorGuard&         guard)
        {
            auto nconn = std::size_t{0};
            for (const auto& well_name : sched.wellNames()) {
                const auto& well = sched.getWellatEnd(well_name);
                nconn = std::max(nconn, well.getConnections().size());
            }

            if (nconn > static_cast<decltype(nconn)>(wdims.maxConnPerWell()))
            {
                const auto& location = wdims.location();
                if (location) {
                    std::string fmt_message = fmt::format("Problem with keyword {{keyword}}\n"
                                                          "In {{file}} line {{line}}\n"
                                                          "The case has a well with {0} connections, but {1} is specified as maxmimum in WELLDIMS.\n"
                                                          "Please increase item 2 in WELLDIMS to at least {0}", nconn, wdims.maxConnPerWell());

                    ctxt.handleError(Opm::ParseContext::RUNSPEC_CONNS_PER_WELL_TOO_LARGE,
                                     fmt_message, *location, guard);
                } else {
                    std::string msg = fmt::format("The case does not have a WELLDIMS keyword.\n"
                                                  "Please add a WELLDIMS keyword in the RUNSPEC section specifying at least {} connections in item 2.", nconn);
                    ctxt.handleError(Opm::ParseContext::RUNSPEC_CONNS_PER_WELL_TOO_LARGE, msg, {}, guard);
                }
            }
        }

        void checkNumGroups(const Opm::Welldims&     wdims,
                            const Opm::Schedule&     sched,
                            const Opm::ParseContext& ctxt,
                            Opm::ErrorGuard&         guard)
        {
            const auto nGroups = sched.back().groups.size();

            // Note: "1 +" to account for FIELD group being in 'sched.numGroups()'
            //   but excluded from WELLDIMS(3).
            if (nGroups > 1U + wdims.maxGroupsInField())
            {
                const auto& location = wdims.location();
                if (location) {
                    std::string fmt_message = fmt::format("Problem with keyword {{keyword}}\n"
                                                          "In {{file}} line {{line}}\n"
                                                          "The case has {0} non-FIELD groups, the WELLDIMS keyword specifies {1}\n."
                                                          "Please increase item 3 in WELLDIMS to at least {0}", nGroups - 1, wdims.maxGroupsInField());

                    ctxt.handleError(Opm::ParseContext::RUNSPEC_NUMGROUPS_TOO_LARGE,
                                     fmt_message, *location, guard);
                } else {
                    std::string msg = fmt::format("The case does not have a WELLDIMS keyword.\n"
                                                  "Please add a WELLDIMS keyword in the RUNSPEC section specifying at least {} groups in item 3.", nGroups - 1);
                    ctxt.handleError(Opm::ParseContext::RUNSPEC_NUMGROUPS_TOO_LARGE,
                                     msg , {}, guard);
                }
            }
        }

        void checkGroupSize(const Opm::Welldims&     wdims,
                            const Opm::Schedule&     sched,
                            const Opm::ParseContext& ctxt,
                            Opm::ErrorGuard&         guard)
        {
            const auto numSteps = sched.size() - 1;

            auto size = std::size_t{0};
            for (auto step = 0*numSteps; step < numSteps; ++step) {
                const auto nwgmax = maxGroupSize(sched, step);

                size = std::max(size, static_cast<std::size_t>(nwgmax));
            }

            if (size > static_cast<decltype(size)>(wdims.maxWellsPerGroup()))
            {
                const auto& location = wdims.location();
                if (location) {
                    std::string fmt_message = fmt::format("Problem with keyword {{keyword}}\n"
                                                          "In {{file}} line {{line}}\n"
                                                          "The case has a maximum group size of {0} wells/groups, the WELLDIMS keyword specifies {1}.\n"
                                                          "Please increase item 4 in WELLDIMS to at least {0}", size, wdims.maxWellsPerGroup());

                    ctxt.handleError(Opm::ParseContext::RUNSPEC_GROUPSIZE_TOO_LARGE,
                                     fmt_message, *location, guard);
                } else {
                    std::string msg = fmt::format("The case does not have a WELLDIMS keyword.\n"
                                                  "Please add a WELLDIMS keyword in the RUNSPEC section specifying at least {} as max groupsize in item 4.", size);
                    ctxt.handleError(Opm::ParseContext::RUNSPEC_GROUPSIZE_TOO_LARGE,
                                     msg, Opm::KeywordLocation{}, guard);
                }
            }
        }
    } // WellDims

    void consistentWellDims(const Opm::Welldims&     wdims,
                            const Opm::Schedule&     sched,
                            const Opm::ParseContext& ctxt,
                            Opm::ErrorGuard&         guard)
    {
        WellDims::checkNumWells   (wdims, sched, ctxt, guard);
        WellDims::checkConnPerWell(wdims, sched, ctxt, guard);
        WellDims::checkNumGroups  (wdims, sched, ctxt, guard);
        WellDims::checkGroupSize  (wdims, sched, ctxt, guard);
    }
} // Anonymous

void
Opm::checkConsistentArrayDimensions(const EclipseState& es,
                                    const Schedule&     sched,
                                    const ParseContext& ctxt,
                                    ErrorGuard&         guard)
{
    consistentWellDims(es.runspec().wellDimensions(), sched, ctxt, guard);
}




int
Opm::maxGroupSize(const Opm::Schedule& sched,
                  const std::size_t    step)
{
    int nwgmax = 0;

    for (const auto& gnm : sched.groupNames(step)) {
        const auto& grp = sched.getGroup(gnm, step);
        const auto  gsz = grp.wellgroup()
            ? grp.numWells() : grp.groups().size();

        nwgmax = std::max(nwgmax, static_cast<int>(gsz));
    }

    return nwgmax;
}
