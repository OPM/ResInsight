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

#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group/Group.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ArrayDimChecker.hpp>


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
                std::ostringstream os;
                os << "Run uses " << nWells << " wells, but allocates at "
                   << "most " << wdims.maxWellsInField() << " in RUNSPEC "
                   << "section.  Increase item 1 of WELLDIMS accordingly.";

                ctxt.handleError(Opm::ParseContext::RUNSPEC_NUMWELLS_TOO_LARGE,
                                 os.str(), guard);
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
                std::ostringstream os;
                os << "Run has well with " << nconn << " reservoir connections, "
                   << "but allocates at most " << wdims.maxConnPerWell()
                   << " connections per well in RUNSPEC section.  Increase item "
                   << "2 of WELLDIMS accordingly.";

                ctxt.handleError(Opm::ParseContext::RUNSPEC_CONNS_PER_WELL_TOO_LARGE,
                                 os.str(), guard);
            }
        }

        void checkNumGroups(const Opm::Welldims&     wdims,
                            const Opm::Schedule&     sched,
                            const Opm::ParseContext& ctxt,
                            Opm::ErrorGuard&         guard)
        {
            const auto nGroups = sched.numGroups();

            // Note: "1 +" to account for FIELD group being in 'sched.numGroups()'
            //   but excluded from WELLDIMS(3).
            if (nGroups > 1U + wdims.maxGroupsInField())
            {
                std::ostringstream os;
                os << "Run uses " << (nGroups - 1) << " non-FIELD groups, but "
                   << "allocates at most " << wdims.maxGroupsInField() 
                   << " in RUNSPEC section.  Increase item 3 of WELLDIMS "
                   << "accordingly.";

                ctxt.handleError(Opm::ParseContext::RUNSPEC_NUMGROUPS_TOO_LARGE,
                                 os.str(), guard);
            }
        }

        void checkGroupSize(const Opm::Welldims&     wdims,
                            const Opm::Schedule&     sched,
                            const Opm::ParseContext& ctxt,
                            Opm::ErrorGuard&         guard)
        {
            const auto numSteps = sched.getTimeMap().numTimesteps();

            auto size = std::size_t{0};
            for (auto step = 0*numSteps; step < numSteps; ++step) {
                const auto nwgmax = maxGroupSize(sched, step);

                size = std::max(size, static_cast<std::size_t>(nwgmax));
            }

            if (size > static_cast<decltype(size)>(wdims.maxWellsPerGroup()))
            {
                std::ostringstream os;
                os << "Run uses maximum group size of " << size << ", but "
                   << "allocates at most " << wdims.maxWellsPerGroup()
                   << " in RUNSPEC section.  Increase item 4 of WELLDIMS "
                   << "accordingly.";

                ctxt.handleError(Opm::ParseContext::RUNSPEC_GROUPSIZE_TOO_LARGE,
                                 os.str(), guard);
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
