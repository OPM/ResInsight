/*
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

#ifndef OPM_WRITE_RESTART_HELPERS_HPP
#define OPM_WRITE_RESTART_HELPERS_HPP

#include <vector>

// Forward declarations

namespace Opm {

    class Runspec;
    class EclipseGrid;
    class EclipseState;
    class Schedule;
    class Well;
    class UnitSystem;
    class UDQActive;
    class Actdims;

} // Opm

namespace Opm { namespace RestartIO { namespace Helpers {

    std::vector<double>
    createDoubHead(const EclipseState& es,
                   const Schedule&     sched,
                   const std::size_t   sim_step,
                   const std::size_t   report_step,
                   const double        simTime,
                   const double        nextTimeStep);

    std::vector<int>
    createInteHead(const EclipseState& es,
                   const EclipseGrid&  grid,
                   const Schedule&     sched,
                   const double        simTime,
                   const int           num_solver_steps,
                   const int           report_step,
                   const int           lookup_step);

    std::vector<bool>
    createLogiHead(const EclipseState& es);

    std::vector<int>
    createUdqDims(const Schedule&     		sched,
                  const std::size_t       lookup_step,
                  const std::vector<int>& inteHead);

    std::size_t
    entriesPerSACT();

    std::size_t
    entriesPerIACT();

    std::size_t
    entriesPerZACT();

    std::size_t
    entriesPerZACN(const Opm::Actdims& actdims);

    std::size_t
    entriesPerIACN(const Opm::Actdims& actdims);

    std::size_t
    entriesPerSACN(const Opm::Actdims& actdims);

    std::vector<int>
    createActionRSTDims(const Schedule&     sched,
                        const std::size_t   simStep);

}}} // Opm::RestartIO::Helpers

#endif  // OPM_WRITE_RESTART_HELPERS_HPP
