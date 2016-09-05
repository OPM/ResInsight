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

#ifndef OPM_CONNECTIVITYGRAPH_HEADER_INCLUDED
#define OPM_CONNECTIVITYGRAPH_HEADER_INCLUDED

#include <vector>

namespace Opm
{
namespace FlowDiagnostics
{
    class ConnectivityGraph
    {
    public:
        /// Construct from explicit neighbourship table.
        ///
        /// The @p connection_to_cell must have size equal to @code 2 * E
        /// @endcode in which @c E is the number of connections.  Connection
        /// @c k connects cells @code connection_to_cell[2*k + 0] @endcode
        /// and @code connection_to_cell[2*k + 1] @endcode.
        ConnectivityGraph(const int        num_cells,
                          std::vector<int> connection_to_cell);

        struct CellPair
        {
            int first;
            int second;
        };

        using SizeType = std::vector<int>::size_type;

        SizeType numCells()      const;
        SizeType numConnections() const;

        CellPair connection(const SizeType i) const;

    private:
        SizeType         numCells_;
        std::vector<int> connCells_;
    };

} // namespace FlowDiagnostics
} // namespace Opm

#endif // OPM_CONNECTIVITYGRAPH_HEADER_INCLUDED
