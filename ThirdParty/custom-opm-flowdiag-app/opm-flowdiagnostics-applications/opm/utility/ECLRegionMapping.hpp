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

#ifndef OPM_ECLREGIONMAPPING_HEADER_INCLUDED
#define OPM_ECLREGIONMAPPING_HEADER_INCLUDED

#include <opm/utility/graph/AssembledConnections.hpp>
#include <opm/utility/graph/AssembledConnectionsIteration.hpp>

#include <unordered_map>
#include <vector>

namespace Opm {
    /// Mapping of region IDs to subsets of explicit cell ID collection.
    ///
    /// The typical client code is
    /// \code
    ///    const auto map = ECLRegionMapping(/* ... */);
    ///
    ///    /// ...
    ///
    ///    const auto& subset = map.regionSubset();
    ///
    ///    for (const auto& regID : map.activeRegions()) {
    ///        for (const auto& ix : map.getRegionIndices(regID)) {
    ///            use(regID, subset[ix]);
    ///        }
    ///    }
    /// \endcode
    class ECLRegionMapping
    {
    public:
        /// Constructor.
        ///
        /// \param[in] region Container of region IDs.  Typically one of the
        ///    explicit region ID vectors in an ECL result set such as
        ///    SATNUM or PVTNUM.  Assumed to be defined on all active cells
        ///    of an ECL result set.
        ///
        /// \param[in] regSubset Index subset of region IDs.  This object
        ///    instance partitions the linear indices in
        ///    \code
        ///      [ 0 .. regSubset.size()-1 ]
        ///    \endcode
        ///    according to the corresponding unique region ID in the set
        ///    \code
        ///      { region[ i ] }_{i \in regSubset}
        ///    \endcode
        ///
        ///    If empty or defaulted, \p regSubset is treated as if it were
        ///    specified as \code [ 0 .. region.size() - 1] \endcode.  The
        ///    common use case for this is working on the entire set of
        ///    active cells implied by the region ID vector.
        ///
        ///    The typical use case of an explicit region subset is when
        ///    sampling PVT or saturation function curves for graphical
        ///    representation.
        ECLRegionMapping(const std::vector<int>& region,
                         const std::vector<int>& regSubset
                             = std::vector<int>());

        /// Retrieve index subset.
        ///
        /// Identical to constructor argument \c regSubset if supplied,
        /// otherwise the vector \code [ 0 .. region.size()-1 ] \endcode.
        const std::vector<int>& regionSubset() const;

        /// Retrieve sorted list of unique region IDs in the subset defined
        /// by \code { region[i] }_{i \in regionSubset()} \endcode.
        std::vector<int> activeRegions() const;

        /// Convenience alias to simplify declaring return type of member
        /// function \code getRegionIndices() \endcode.
        using IndexView = SimpleIteratorRange<
            AssembledConnections::Neighbours::const_iterator>;

        /// Retrive linear indices into \code regionSubset() \endcode that
        /// correspond to particular region ID.
        ///
        /// \param[in] region Numeric region ID.  Must be one of the unique
        ///    region IDs implied by \code activeRegions() \endcode.  If it
        ///    is not one of those IDs, function \code getRegionIndices()
        ///    \endcode throws an instance of \code std::logic_error
        ///    \endcode.
        ///
        /// \return Linear index view into \code regionSubset() \endcode.
        IndexView getRegionIndices(const int region) const;

    private:
        /// Offset from which to start assigning linear, dense active IDs.
        int start_{1};

        /// Next, unassigned linear ID.
        int next_{1};

        /// Subset of full region ID mapping.
        std::vector<int> regSubset_;

        /// Sparse mapping of region IDs to dense active IDs.
        std::unordered_map<int, int> activeID_;

        /// Map active IDs to subsets of the initial index subset.
        AssembledConnections regionSubsetIndex_;

        /// Translate region ID to dense linear active ID.  Mutable version.
        int activeID(const int regID);

        /// Translate region ID to dense linear active ID.  Immutable
        /// version.
        int activeID(const int regID) const;
    };
} // Opm

#endif // OPM_ECLREGIONMAPPING_HEADER_INCLUDED
