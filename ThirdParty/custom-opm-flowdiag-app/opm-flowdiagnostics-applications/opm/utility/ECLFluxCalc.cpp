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

#include <opm/utility/ECLFluxCalc.hpp>

namespace Opm
{

    ECLFluxCalc::ECLFluxCalc(const ECLGraph& graph)
        : graph_(graph)
        , neighbours_(graph.neighbours())
        , transmissibility_(graph.transmissibility())
    {
    }





    std::vector<double> ECLFluxCalc::flux(const PhaseIndex /* phase */) const
    {
        // Obtain pressure vector.
        std::vector<double> pressure = graph_.linearisedCellData("PRESSURE", &ECLUnits::UnitSystem::pressure);

        // Compute fluxes per connection.
        const int num_conn = transmissibility_.size();
        std::vector<double> fluxvec(num_conn);
        for (int conn = 0; conn < num_conn; ++conn) {
            fluxvec[conn] = singleFlux(conn, pressure);
        }
        return fluxvec;
    }





    double ECLFluxCalc::singleFlux(const int connection,
                                   const std::vector<double>& pressure) const
    {
        const int c1 = neighbours_[2*connection];
        const int c2 = neighbours_[2*connection + 1];
        const double t = transmissibility_[connection];
        return t * (pressure[c1] - pressure[c2]);
    }


} // namespace Opm
