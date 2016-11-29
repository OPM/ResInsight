/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/flowdiagnostics/ConnectionValues.hpp>

#include <exception>
#include <stdexcept>

namespace Opm
{
namespace FlowDiagnostics
{


ConnectionValues::
ConnectionValues(const NumConnections& nconn,
                 const NumPhases&      nphase)
    : nconn_ (nconn)
    , nphase_(nphase)
    , data_  (nconn_.total * nphase_.total, 0.0)
{}

std::vector<double>::size_type
ConnectionValues::numConnections() const
{
    return nconn_.total;
}

std::size_t
ConnectionValues::numPhases() const
{
    return nphase_.total;
}

double
ConnectionValues::operator()(const ConnID&  conn,
                                  const PhaseID& phase) const
{
    if ((conn .id >= nconn_ .total) ||
        (phase.id >= nphase_.total))
    {
        throw std::logic_error("(Connection,Phase) pair out of range");
    }

    return data_[conn.id*nphase_.total + phase.id];
}

double&
ConnectionValues::operator()(const ConnID&  conn,
                                  const PhaseID& phase)
{
    if ((conn .id >= nconn_ .total) ||
        (phase.id >= nphase_.total))
    {
        throw std::logic_error("(Connection,Phase) pair out of range");
    }

    return data_[conn.id*nphase_.total + phase.id];
}

} // namespace FlowDiagnostics
} // namespace Opm
