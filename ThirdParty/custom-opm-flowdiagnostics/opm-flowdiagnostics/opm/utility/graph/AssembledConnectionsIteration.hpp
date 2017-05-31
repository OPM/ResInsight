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

#ifndef OPM_ASSEMBLEDCONNECTIONSITERATION_HEADER_INCLUDED
#define OPM_ASSEMBLEDCONNECTIONSITERATION_HEADER_INCLUDED

#include <cassert>

namespace Opm {

    /// Holds data for a single connection.
    /// The neighbour will typically be a cell
    /// index, and the weight will typically be
    /// a flux.
    struct ConnectionData
    {
        int neighbour;
        double weight;
    };




    /// Iterator over the graph neighbourhood
    /// of a cell, typically used from AssembledConnections.
    class NeighbourhoodIterator
    {
    public:
        NeighbourhoodIterator(const int* neighbour_iter,
                              const double* weight_iter)
            : neighbour_iter_(neighbour_iter),
              weight_iter_(weight_iter)
        {
        }

        ConnectionData operator*()
        {
            return { *neighbour_iter_, *weight_iter_ };
        }

        bool operator!=(const NeighbourhoodIterator& other) const
        {
            assert((neighbour_iter_ != other.neighbour_iter_) == (weight_iter_ != other.weight_iter_));
            return neighbour_iter_ != other.neighbour_iter_;
        }

        NeighbourhoodIterator& operator++()
        {
            ++neighbour_iter_;
            ++weight_iter_;
            return *this;
        }

    private:
        const int* neighbour_iter_;
        const double* weight_iter_;
    };




    /// A straightforward range class whose only purpose is
    /// to easily allow range-for loops over [begin_, end_).
    template <class Iterator>
    struct SimpleIteratorRange
    {
        Iterator begin() const
        {
            return begin_;
        }

        Iterator end() const
        {
            return end_;
        }

        Iterator begin_;
        Iterator end_;
    };


} // namespace Opm

#endif // OPM_ASSEMBLEDCONNECTIONSITERATION_HEADER_INCLUDED
