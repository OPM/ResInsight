/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016, 2017 Statoil ASA.

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

#ifndef OPM_ECLWELLSOLUTION_HEADER_INCLUDED
#define OPM_ECLWELLSOLUTION_HEADER_INCLUDED

#include <array>
#include <string>
#include <utility>
#include <vector>

namespace Opm
{

    class ECLRestartData;

    class ECLWellSolution
    {
    public:
        /// Constructor.
        /// \param[in] rate_threshold      a well will be ignored if its total RESV rate is less than this (m^3/s)
        /// \param[in] disallow_crossflow  if true, injecting perforations of production wells and vice versa will be ignored
        explicit ECLWellSolution(const double rate_threshold = 1e-14,
                                 const bool disallow_crossflow = true);

        /// Contains the well data extracted from the restart file.
        struct WellData
        {
            std::string name;
            bool is_injector_well;

            double qOs;   // Well oil surface volume rate.
            double qWs;   // Well water surface volume rate.
            double qGs;   // Well gas surface volume rate.
            double lrat;  // Well liquid (oil + water) surface volume rate.
            double bhp;   // Well bottom hole pressure.
            double qr;    // Well total reservoir volume rate.

            struct Completion
            {
                std::string gridName;          // Empty for main grid, otherwise LGR grid.
                std::array<int, 3> ijk;        // Cartesian location in grid.
                double reservoir_inflow_rate;  // Total reservoir volume fluid rate in SI (m^3/s).
                double qOs;                    // Completion oil surface volute rate.
                double qWs;                    // Completion water surface volute rate.
                double qGs;                    // Completion gas surface volute rate.

            };
            std::vector<Completion> completions;
        };

        /// Return well solution for pre-selected report step
        ///
        /// Will throw if required data is not available for the
        /// requested step.
        std::vector<WellData> solution(const ECLRestartData& restart,
                                       const std::vector<std::string>& grids) const;

    private:
        // Data members.
        double rate_threshold_;
        bool disallow_crossflow_;

        // Methods.
        std::vector<WellData> readWellData(const ECLRestartData& restart,
                                           const std::string& gridName) const;
    };


} // namespace Opm

#endif // OPM_ECLWELLSOLUTION_HEADER_INCLUDED
