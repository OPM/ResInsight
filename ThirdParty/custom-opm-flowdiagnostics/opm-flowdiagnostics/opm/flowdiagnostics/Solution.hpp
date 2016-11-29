/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

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

#ifndef OPM_SOLUTION_HEADER_INCLUDED
#define OPM_SOLUTION_HEADER_INCLUDED


#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/flowdiagnostics/CellSetValues.hpp>
#include <memory>
#include <vector>


namespace Opm
{
namespace FlowDiagnostics
{

    /// Results from diagnostics computations.
    class Solution
    {
    public:
        /// Default constructor.
        Solution();

        /// Destructor.
        ~Solution();

        /// Copy constructor.
        Solution(const Solution& rhs);

        /// Move constructor.
        Solution(Solution&& rhs);

        // ------ Interface for querying of solution below ------

        /// Ids of stored tracer solutions.
        std::vector<CellSetID> startPoints() const;

        /// Time-of-flight field from all start points.
        const std::vector<double>& timeOfFlight() const;

        /// Time-of-flight field restricted to single tracer region.
        CellSetValues timeOfFlight(const CellSetID& tracer) const;

        /// The computed tracer field corresponding to a single tracer.
        ///
        /// The \c tracer must correspond to an id passed in
        /// computeX...Diagnostics().
        CellSetValues concentration(const CellSetID& tracer) const;

        // ------ Interface for modification of solution below ------

        struct TimeOfFlight
        {
            CellSetValues data;
        };

        struct TracerConcentration
        {
            CellSetValues data;
        };

        void assignGlobalToF(std::vector<double>&& global_tof);
        void assign(const CellSetID& id, TimeOfFlight&& tof);
        void assign(const CellSetID& id, TracerConcentration&& conc);

    private:
        class Impl;

        explicit Solution(std::unique_ptr<Impl> pImpl);

        std::unique_ptr<Impl> pImpl_;
    };

} // namespace FlowDiagnostics
} // namespace Opm

#endif // OPM_SOLUTION_HEADER_INCLUDED
