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
    
#include <opm/flowdiagnostics/ConnectivityGraph.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <utility>

namespace Opm
{
namespace FlowDiagnostics
{

ConnectivityGraph::ConnectivityGraph(const int        num_cells,
                                     std::vector<int> connection_to_cell)
    : numCells_ (num_cells)
    , connCells_(std::move(connection_to_cell))
{
    if ((connCells_.size() % 2) != 0) {
        throw std::logic_error("Neighbourship must be N-by-2");
    }

    if ((! connCells_.empty()) &&
        (*std::max_element(connCells_.begin(),
                           connCells_.end())
         >= num_cells))
    {
        throw std::logic_error("Cell indices must be in range");
    }
}

ConnectivityGraph::SizeType
ConnectivityGraph::numCells() const
{
    return numCells_;
}

ConnectivityGraph::SizeType
ConnectivityGraph::numConnections() const
{
    return connCells_.size() / 2;
}

ConnectivityGraph::CellPair
ConnectivityGraph::connection(const SizeType i) const
{
    assert (i < numConnections());

    const auto* const start = &connCells_[2*i + 0];

    return { *(start + 0), *(start + 1) };
}

} // namespace FlowDiagnostics
} // namespace Opm

