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

#ifndef OPM_CONNECTIONVALUES_HEADER_INCLUDED
#define OPM_CONNECTIONVALUES_HEADER_INCLUDED

#include <cstddef>
#include <vector>

namespace Opm
{
namespace FlowDiagnostics
{


    class ConnectionValues
    {
    public:
        struct NumConnections
        {
            std::vector<double>::size_type total;
        };

        struct NumPhases
        {
            std::size_t total;
        };

        struct ConnID
        {
            std::vector<double>::size_type id;
        };

        struct PhaseID
        {
            std::size_t id;
        };

        ConnectionValues(const NumConnections& nconn,
                         const NumPhases&      nphase);

        std::vector<double>::size_type numConnections() const;
        std::size_t                    numPhases()      const;

        double operator()(const ConnID&  connection,
                          const PhaseID& phase) const;

        double& operator()(const ConnID&  connection,
                           const PhaseID& phase);

    private:
        NumConnections      nconn_;
        NumPhases           nphase_;

        std::vector<double> data_;
    };

} // namespace FlowDiagnostics
} // namespace Opm

#endif // OPM_CONNECTIONVALUES_HEADER_INCLUDED
