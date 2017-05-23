/*
  Copyright 2017 Statoil ASA.

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

#ifndef OPM_ECLFLUXCALC_HEADER_INCLUDED
#define OPM_ECLFLUXCALC_HEADER_INCLUDED

#include <opm/utility/ECLGraph.hpp>
#include <vector>

namespace Opm
{
    class ECLRestartData;

    /// Class for computing connection fluxes in the absence of flux output.
    class ECLFluxCalc
    {
    public:
        /// Construct from ECLGraph.
        ///
        /// \param[in] graph Connectivity data, as well as providing a means to read data from the restart file.
        explicit ECLFluxCalc(const ECLGraph& graph);

        using PhaseIndex = ECLGraph::PhaseIndex;

        /// Retrive phase flux on all connections defined by \code
        /// graph.neighbours() \endcode.
        ///
        /// \param[in] phase Canonical phase for which to retrive flux.
        ///
        /// \return Flux values corresponding to selected phase.
        ///         Empty if required data is missing.
        ///         Numerical values in SI units (rm^3/s).
        std::vector<double>
        flux(const ECLRestartData& rstrt,
             const PhaseIndex      phase) const;

    private:
        struct DynamicData
        {
            std::vector<double> pressure;
        };

        double singleFlux(const int connection,
                          const DynamicData& dyn_data) const;

        const ECLGraph& graph_;
        std::vector<int> neighbours_;
        std::vector<double> transmissibility_;
    };

} // namespace Opm

#endif // OPM_ECLFLUXCALC_HEADER_INCLUDED
