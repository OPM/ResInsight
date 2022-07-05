/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.
  Copyright 2022 Equinor ASA

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

#ifndef OPM_OUTPUT_DATA_INTERREGFLOWMAP_HPP
#define OPM_OUTPUT_DATA_INTERREGFLOWMAP_HPP

#include <opm/output/data/InterRegFlow.hpp>

#include <cstddef>
#include <optional>
#include <utility>
#include <type_traits>
#include <vector>

/// \file
///
/// Facility for converting collection of region ID pairs into a sparse
/// (CSR) adjacency matrix representation of a graph.  Supports O(nnz)
/// compression and, if applicable, accumulation of weight values for
/// repeated entity pairs.

namespace Opm { namespace data {

    /// Form CSR adjacency matrix representation of inter-region flow rate
    /// graph provided as a list of connections between regions.
    class InterRegFlowMap
    {
    private:
        /// Representation of neighbouring regions.
        using Neighbours = std::vector<int>;

        /// Offset into neighbour array.
        using Offset = Neighbours::size_type;

        /// CSR start pointers.
        using Start = std::vector<Offset>;

        /// Linear flow rate buffer.
        using RateBuffer = std::vector<float>;

        /// Internal view of flows between regions.
        using Window = InterRegFlow<RateBuffer::iterator>;

    public:
        /// Client view of flows between specified region pair.
        using ReadOnlyWindow = InterRegFlow<std::vector<float>::const_iterator>;

        /// Client type through which to define a single inter-region connection.
        using FlowRates = Window::FlowRates;

        /// Client type through which to identify a component flow of a
        /// single inter-region connection.
        using Component = Window::Component;

        /// Add flow rate connection between regions.
        ///
        /// \param[in] r1 Primary (source) zero-based region index.  Used as
        ///    row index.
        ///
        /// \param[in] r2 Secondary (sink) zero-based region index.  Used as
        ///   column index.
        ///
        /// \param[in] rates Flow rates associated to single connection.
        ///
        /// If both region IDs are the same then this function does nothing.
        void addConnection(const int r1, const int r2, const FlowRates& rates);

        /// Form CSR adjacency matrix representation of input graph from
        /// connections established in previous calls to addConnection().
        ///
        /// \param[in] numRegions Number of rows in resulting CSR matrix.
        ///     If prior calls to addConnection() supply source entity IDs
        ///     (row indices) greater than or equal to \p numRows, then
        ///     method compress() will throw \code std::invalid_argument
        ///     \endcode.
        void compress(const std::size_t numRegions);

        /// Retrieve number of rows (source entities) in input graph.
        /// Corresponds to value of argument passed to compress().  Valid
        /// only after calling compress().
        Offset numRegions() const;

        /// Retrieve accumulated inter-region flow rates for identified pair
        /// of regions.
        ///
        /// \param[in] r1 Primary (source) zero-based region index.  Used as
        ///    row index.
        ///
        /// \param[in] r2 Secondary (sink) zero-based region index.  Used as
        ///    column index.
        ///
        /// \return View of accumulated inter-region flow rates and
        ///    associated flow direction sign.  \code std::nullopt \endcode
        ///    if no such rates exist.
        std::optional<std::pair<ReadOnlyWindow, ReadOnlyWindow::ElmT>>
        getInterRegFlows(const int r1, const int r2) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const
        {
            this->csr_.write(buffer);
        }

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void read(MessageBufferType& buffer)
        {
            auto other = CSR{};
            other.read(buffer);

            this->uncompressed_
                .add(other.maxRowIdx(),
                     other.maxColIdx(),
                     other.coordinateFormatRowIndices(),
                     other.columnIndices(),
                     other.values());
        }

        /// Clear all internal buffers, but preserve allocated capacity.
        void clear();

    private:
        /// Coordinate format representation of individual contributions to
        /// inter-region flows.
        class Connections
        {
        public:
            /// Add contributions from a single inter-region connection.
            ///
            /// \param[in] r1 Source region.  Zero-based region index/ID.
            ///
            /// \param[in] r2 Destination region.  Zero-based region index.
            ///
            /// \param[in] rates Flow rates of single inter-region
            ///    connection.
            void add(const int r1, const int r2, const FlowRates& rates);

            /// Add contributions from multiple inter-region connections.
            ///
            /// \param[in] maxRowIdx Maximum row (source region) index
            ///    across all new inter-region connection contributions.
            ///
            /// \param[in] maxColIdx Maximum column (destination region)
            ///    index across all new inter-region contributions.
            ///
            /// \param[in] rows Source region indices for all new
            ///    inter-region connection contributions.
            ///
            /// \param[in] cols Destination region indices for all new
            ///    inter-region connection contributions.
            ///
            /// \param[in] rates Flow rate values for all new inter-region
            ///    connection contributions.
            void add(const int maxRowIdx,
                     const int maxColIdx,
                     const Neighbours& rows,
                     const Neighbours& cols,
                     const RateBuffer& rates);

            /// Clear internal tables.  Preserve allocated capacity.
            void clear();

            /// Predicate.
            ///
            /// \return Whether or not internal tables are empty.
            bool empty() const;

            /// Whether or not internal tables meet size consistency
            /// requirements.
            bool isValid() const;

            /// Maximum zero-based row (source region) index.
            int maxRow() const;

            /// Maximum zero-based column (destination region) index.
            int maxCol() const;

            /// Number of uncompressed contributions in internal tables.
            Neighbours::size_type numContributions() const;

            /// Read-only access to uncompressed row indices.
            const Neighbours& rowIndices() const;

            /// Read-only access to uncompressed column indices.
            const Neighbours& columnIndices() const;

            /// Read-only access to uncompressed flow rate values.
            const RateBuffer& values() const;

        private:
            /// Zero-based row/source region indices.
            Neighbours i_{};

            /// Zero-based column/destination region indices.
            Neighbours j_{};

            /// Uncompressed flow rate values.  Window::bufferSize() entries
            /// per connection.
            RateBuffer v_{};

            /// Maximum row index in \code this->i_ \endcode.
            int max_i_{ -1 };

            /// Maximum column index in \code this->j_ \endcode.
            int max_j_{ -1 };
        };

        /// Compressed sparse row representation of inter-region flow rates
        ///
        /// Row and column indices are zero-based region IDs.  Column
        /// indices ascendingly sorted per row.  Value type is window,
        /// backed by a pair of iterators, of aggregate flow rates per
        /// region pair.
        class CSR
        {
        public:
            /// Merge coordinate format into existing CSR map.
            ///
            /// \param[in] conns Coordinate representation of new
            ///    contributions.
            ///
            /// \param[in] numRegions Maximum number of regions in this
            ///    region set.  Common values/settings are
            ///
            ///      -# Maximum one-based region ID on local MPI rank
            ///      -# Maximum one-based region ID across all MPI ranks
            ///      -# Maximum *possible* one-based region ID in model
            ///         ("NTFIP"), from TABDIMS(5) and/or REGDIMS(1).
            ///
            ///    If this value is smaller than the maximum one-based
            ///    region ID on the local MPI rank, then it will be ignored
            ///    and the local rank's maximum one-based region ID will be
            ///    used instead.
            void merge(const Connections& conns,
                       const Offset       numRegions);

            /// Read-only access to flow rates of given region ID pair.
            ///
            /// \param[in] i Source region.  Zero-based region ID.
            ///
            /// \param[in] j Destination region.  Zero-based region ID.
            ///
            /// \return Flow rates of region ID pair.  Nullopt if no such
            ///    pair exists.
            std::optional<ReadOnlyWindow> getWindow(const int i, const int j) const;

            /// Total number of rows in compressed map structure.
            Offset numRows() const;

            /// Maximum zero-based row index encountered mapped structure.
            int maxRowIdx() const;

            /// Maximum zero-based column index encountered mapped structure.
            int maxColIdx() const;

            /// Read-only access to compressed structure's start pointers.
            const Start& startPointers() const;

            /// Read-only access to compressed structure's column indices,
            /// ascendingly sorted per rwo.
            const Neighbours& columnIndices() const;

            /// Read-only access to compressed, unique, linearised flow rate
            /// values.  \code Window::bufferSize() \endcode entries per
            /// non-zero element.
            const RateBuffer& values() const;

            /// Coordinate format row index vector.  Expanded from \code
            /// startPointers() \endcode.
            Neighbours coordinateFormatRowIndices() const;

            // MessageBufferType API should be similar to Dune::MessageBufferIF
            template <class MessageBufferType>
            void write(MessageBufferType& buffer) const
            {
                this->writeVector(this->ia_, buffer);
                this->writeVector(this->ja_, buffer);
                this->writeVector(this->sa_, buffer);
                this->writeVector(this->compressedIdx_, buffer);

                buffer.write(this->numRows_);
                buffer.write(this->numCols_);
            }

            // MessageBufferType API should be similar to Dune::MessageBufferIF
            template <class MessageBufferType>
            void read(MessageBufferType& buffer)
            {
                this->readVector(buffer, this->ia_);
                this->readVector(buffer, this->ja_);
                this->readVector(buffer, this->sa_);
                this->readVector(buffer, this->compressedIdx_);

                buffer.read(this->numRows_);
                buffer.read(this->numCols_);
            }

            /// Clear internal tables.  Preserve allocated capacity.
            void clear();

        private:
            /// Start pointers.
            Start ia_{};

            /// Column indices.  Ascendingly sorted per row once structure
            /// is fully established.
            Neighbours ja_{};

            /// Compressed, unique, linearised flow rate values.  \code
            /// Window::bufferSize() \endcode entries per non-zero map
            /// element.
            RateBuffer sa_{};

            /// Destination index in compressed representation.  Size NNZ.
            Start compressedIdx_{};

            /// Number of active rows in compressed map structure.
            int numRows_{ 0 };

            /// Number of active columns in compressed map structure.
            /// Tracked as the maximum column index plus one.
            int numCols_{ 0 };

            // ---------------------------------------------------------
            // Implementation of read()/write()
            // ---------------------------------------------------------

            template <typename T, class A, class MessageBufferType>
            void writeVector(const std::vector<T,A>& vec,
                             MessageBufferType&      buffer) const
            {
                const auto n = vec.size();
                buffer.write(n);

                for (const auto& x : vec) {
                    buffer.write(x);
                }
            }

            template <class MessageBufferType, typename T, class A>
            void readVector(MessageBufferType& buffer,
                            std::vector<T,A>&  vec)
            {
                auto n = 0 * vec.size();
                buffer.read(n);

                vec.resize(n);

                for (auto& x : vec) {
                    buffer.read(x);
                }
            }

            // ---------------------------------------------------------
            // Implementation of merge()
            // ---------------------------------------------------------

            /// Incorporate new, coordinate format contributions into
            /// existing, possibly empty, CSR mapping structure.
            ///
            /// On exit the ia_ array holds the proper start pointers while
            /// ja_ holds the corresponding column indices albeit possibly
            /// repeated and unsorted.
            ///
            /// \param[in] rows Row indices of all, possibly repeated,
            ///    coordinate format input contributions.  Start pointers \c
            ///    ia_ updated to account for new entries.
            ///
            /// \param[in] cols Column index of coordinate format intput
            ///    structure.  Inserted into \c ja_ according to its
            ///    corresponding row index.
            ///
            /// \param[in] maxRowIdx Maximum index in \p rows.  Needed to
            ///    ensure proper size of \c ia_.
            ///
            /// \param[in] maxColIdx Maximum index in \p cols.
            void assemble(const Neighbours& rows,
                          const Neighbours& cols,
                          const int         maxRowIdx,
                          const int         maxColIdx);


            /// Sort column indices per row and compress repeated column
            /// indices down to a single unique element per row.  Sum
            /// repeated values
            ///
            /// On exit the \c ia_, \c ja_, and \c sa_ arrays all have their
            /// expected, canonical structure.
            ///
            /// \param[in] numRegions Maximum number of regions supported by
            ///    final compressed mapping structure.  Ignored if less than
            ///    active number of rows.
            ///
            /// \param[in] rates Uncompressed flow rate values from
            ///    coordinate format contributions.
            void compress(const Offset      numRegions,
                          const RateBuffer& rates);

            /// Sort column indices within each mapped row.
            ///
            /// On exit \c ja_ has ascendingly sorted column indices, albeit
            /// possibly with repeated entries.  This function also updates
            /// \c compressedIdx_ to account for the new locations of the
            /// non-zero elements in the grouped structure.
            void sortColumnIndicesPerRow();

            /// Condense repeated column indices per row down to a single
            /// unique entry for each.
            ///
            /// Assumes that each row has ascendingly sorted column indices
            /// in \c ja_ and must therefore be called after member function
            /// sortColumnIndicesPerRow().  On exit, \c ja_ has its final
            /// canonical structure and \c compressedIdx_ knows the final
            /// location of each non-zero contribution in the input
            /// coordinate format.
            void condenseDuplicates();

            /// Sum coordinate format flow rates into compressed map
            /// structure.
            ///
            /// Repeated (row,column) index pairs in the input coordinate
            /// format add to the same compressed map element.  This
            /// function assumes that \c compressedIdx_ knows the final
            /// compressed location of each non-zero contribution in the
            /// input coordinate format and must therefore be called after
            /// member function condenseDuplicates().  On exit \c sa_ has
            /// incorporated all entries from the input coordinate
            /// structure.
            ///
            /// \param[in] v Uncompressed flow rate values from coordinate
            ///    format contributions.
            void accumulateFlowRates(const RateBuffer& v);

            // ---------------------------------------------------------
            // Implementation of assemble()
            // ---------------------------------------------------------

            /// Position end pointers at start of row to prepare for column
            /// index grouping by corresponding row index.
            ///
            /// Also counts total number of non-zero elements, possibly
            /// including repetitions, in \code this->ia_[0] \endcode.
            ///
            /// \param[in] Number of rows in final compressed structure.
            ///    Used to allocate \code this->ia_ \endcode.
            ///
            /// \param[in] Row indices of all, possibly repeated, coordinate
            ///    format input contributions.  Needed to count the number
            ///    of possibly repeated column index entries per row.
            void preparePushbackRowGrouping(const int         numRows,
                                            const Neighbours& rowIdx);

            /// Group column indices by corresponding row index and track
            /// grouped location of original coordinate format element
            ///
            /// Appends grouped location to \c compressedIdx_.
            ///
            /// \param[in] rowIdx Row index of coordinate format input
            ///    structure.  Used as grouping key.
            ///
            /// \param[in] colIdx Column index of coordinate format intput
            ///    structure.  Inserted into \c ja_ according to its
            ///    corresponding row index.
            void groupAndTrackColumnIndicesByRow(const Neighbours& rowIdx,
                                                 const Neighbours& colIdx);

            // ---------------------------------------------------------
            // General utilities
            // ---------------------------------------------------------

            /// Transpose connectivity structure.
            ///
            /// Essentially swaps the roles of rows and columns.  Also used
            /// as a basic building block for sortColumnIndicesPerRow().
            void transpose();

            /// Condense sequences of repeated column indices in a single
            /// map row down to a single copy of each unique column index.
            ///
            /// Appends new unique column indices to \code ja_ \endcode
            ///
            /// Assumes that the map row has ascendingly sorted column
            /// indices and therefore has the same requirements as
            /// std::unique.  Will also update the internal compressedIdx_
            /// mapping to record new compressed locations for the current,
            /// uncompressed, non-zero map elements.
            ///
            /// \param[in] begin Start of map row that contains possibly
            ///    repeated column indices.
            ///
            /// \param[in] end One-past-end of map row that contains
            ///    possibly repeated column indices.
            void condenseAndTrackUniqueColumnsForSingleRow(Neighbours::const_iterator begin,
                                                           Neighbours::const_iterator end);

            /// Update \c compressedIdx_ mapping to account for column index
            /// reshuffling.
            ///
            /// \param[in] compressedIdx New compressed index locations of
            ///   the non-zero map entries.
            void remapCompressedIndex(Start&& compressedIdx);
        };

        /// Accumulated coordinate format contributions that have not yet
        /// been added to the final CSR structure.
        Connections uncompressed_;

        /// Canonical representation of unique inter-region flow rates.
        CSR csr_;
    };

}} // namespace Opm::data

#endif // OPM_OUTPUT_DATA_INTERREGFLOWMAP_HPP
