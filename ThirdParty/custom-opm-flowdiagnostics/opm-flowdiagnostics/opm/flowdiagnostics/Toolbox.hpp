/*
  Copyright 2016 Statoil ASA.
  Copyright 2016 SINTEF ICT, Applied Mathematics.

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

#ifndef OPM_FLOWDIAGNOSTICS_TOOLBOX_HEADER_INCLUDED
#define OPM_FLOWDIAGNOSTICS_TOOLBOX_HEADER_INCLUDED

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/flowdiagnostics/CellSetValues.hpp>
#include <opm/flowdiagnostics/ConnectivityGraph.hpp>
#include <opm/flowdiagnostics/ConnectionValues.hpp>
#include <opm/flowdiagnostics/Solution.hpp>

#include <memory>
#include <vector>

namespace Opm
{
namespace FlowDiagnostics
{

    /// Toolbox for running flow diagnostics.
    class Toolbox
    {
    public:
        /// Construct from known neighbourship relation.
        explicit Toolbox(const ConnectivityGraph& connectivity);

        /// Destructor.
        ~Toolbox();

        /// Move constructor.
        Toolbox(Toolbox&& rhs);

        /// Move assignment.
        Toolbox& operator=(Toolbox&& rhs);

        /// Assign pore volumes associated with each active cell.
        void assignPoreVolume(const std::vector<double>& pv);

        /// Assign fluxes associated with each connection.
        void assignConnectionFlux(const ConnectionValues& flux);

        /// Assign inflow fluxes, typically from wells.
        ///
        /// Inflow fluxes (injection) should be positive, outflow
        /// fluxes (production) should be negative, both should be
        /// given in the inflow_flux argument passed to this method.
        /// Values from a single well should typically be associated with
        /// a single CellSetID and be a single CellSetValues object.
        void assignInflowFlux(const std::map<CellSetID, CellSetValues>& inflow_flux);

        struct Forward
        {
            const Solution fd;
        };

        struct Reverse
        {
            const Solution fd;
        };

        /// Compute forward time-of-flight and tracer solutions.
        ///
        /// An element of \code start_sets \endcode provides a set of
        /// starting locations for a single tracer.
        ///
        /// Forward time-of-flight is the time needed for a neutral fluid
        /// particle to flow from the nearest fluid source to an arbitrary
        /// point in the model.  The tracer solutions identify cells that
        /// are flooded by injectors.
        ///
        /// You must have called assignPoreVolume() and assignConnectionFlux()
        /// before calling this method.
        ///
        /// The IDs of the \code start_sets \endcode must be unique.
        Forward computeInjectionDiagnostics(const std::vector<CellSet>& start_sets);

        /// Compute reverse time-of-flight and tracer solutions.
        ///
        /// An element of \code start_sets \endcode provides a set of
        /// starting locations for a single tracer.
        ///
        /// Reverse time-of-flight is the time needed for a neutral fluid
        /// particle to flow from an arbitrary point to the nearest fluid
        /// sink in the model.  The tracer solutions identify cells that are
        /// drained by producers.
        ///
        /// You must have called assignPoreVolume() and assignConnectionFlux()
        /// before calling this method.
        ///
        /// The IDs of the \code start_sets \endcode must be unique.
        Reverse computeProductionDiagnostics(const std::vector<CellSet>& start_sets);

    private:
        class Impl;

        std::unique_ptr<Impl> pImpl_;
    };

} // namespace FlowDiagnostics
} // namespace Opm

#endif // OPM_FLOWDIAGNOSTICS_TOOLBOX_HEADER_INCLUDED
