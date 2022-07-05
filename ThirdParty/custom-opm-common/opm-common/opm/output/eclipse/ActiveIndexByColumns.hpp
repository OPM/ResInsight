/*
  Copyright (c) 2021 Equinor ASA

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

#ifndef OPM_ACTIVE_INDEX_BY_COLUMNS_HPP
#define OPM_ACTIVE_INDEX_BY_COLUMNS_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <vector>

namespace Opm {
    class EclipseGrid;
}

namespace Opm {

/// Special purpose mapping facility to handle the output code's need to
/// enumerate the active cells by columns (layer ID (K) cycling fastest,
/// followed by J, followed by I) for aquifer connections.
class ActiveIndexByColumns
{
public:
    bool operator==(const ActiveIndexByColumns& rhs) const;

    /// Create natural->columnar active cell index mapping.
    ///
    /// \param[in] numActive Number of active cells in model.
    /// \param[in] cartDims Model's Cartesian dimensions.
    /// \param[in] getIJK Call-back routine for retrieving the Cartesian
    ///    (I,J,K) tuple of an active cell index.
    explicit ActiveIndexByColumns(const std::size_t                                           numActive,
                                  const std::array<int, 3>&                                   cartDims,
                                  const std::function<std::array<int, 3>(const std::size_t)>& getIJK);

    /// Map active index in natural order to active index in columnar order.
    ///
    /// The output code needs return type \c int here, so use that instead
    /// of \code std::size_t \endcode.
    int getColumnarActiveIndex(const std::size_t naturalActiveIndex) const
    {
        assert ((naturalActiveIndex < this->natural2columnar_.size())
                && "Natural active cell index out of bounds");

        return this->natural2columnar_[naturalActiveIndex];
    }

private:
    std::vector<int> natural2columnar_;
};

/// Build natural->columnar active cell index mapping from an EclipseGrid instance.
ActiveIndexByColumns buildColumnarActiveIndexMappingTables(const EclipseGrid& grid);

} // namespace Opm


#endif // OPM_ACTIVE_INDEX_BY_COLUMNS_HPP
