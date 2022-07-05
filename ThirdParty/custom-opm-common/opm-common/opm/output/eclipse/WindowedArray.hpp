/*
  Copyright (c) 2018 Statoil ASA

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

#ifndef OPM_WINDOWED_ARRAY_HPP
#define OPM_WINDOWED_ARRAY_HPP

#include <cassert>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <boost/range/iterator_range.hpp>

/// \file
///
/// Provide facilities to simplify constructing restart vectors
/// such as IWEL or RSEG.

namespace Opm { namespace RestartIO { namespace Helpers {

    /// Provide read-only and read/write access to constantly sized
    /// portions/windows of a linearised buffer with an implied
    /// 1D array structure.
    ///
    /// Intended as backing store for vectors that have a constant
    /// number of items per entity (e.g., N integer data items for
    /// each active group at a report step).
    ///
    /// \tparam T Element type for underlying data items.
    template <typename T>
    class WindowedArray
    {
    public:
        /// Read/write access.
        using WriteWindow = boost::iterator_range<
            typename std::vector<T>::iterator>;

        /// Read-only access.
        using ReadWindow = boost::iterator_range<
            typename std::vector<T>::const_iterator>;

        using Idx = typename std::vector<T>::size_type;

        /// Distinct compile-time type for number of windows in
        /// underlying storage.
        struct NumWindows { Idx value; };

        /// Distinct compile-time type for size of windows
        /// (number of data items per window.)
        struct WindowSize { Idx value; };

        /// Constructor.
        ///
        /// \param[in] n Number of windows.
        /// \param[in] sz Number of data items per window.
        explicit WindowedArray(const NumWindows n, const WindowSize sz)
            : x_         (n.value * sz.value)
            , windowSize_(sz.value)
        {
            if (sz.value == 0)
                throw std::invalid_argument("Window array with windowsize==0 is not permitted");
        }

        WindowedArray(const WindowedArray& rhs) = default;
        WindowedArray(WindowedArray&& rhs) = default;
        WindowedArray& operator=(const WindowedArray& rhs) = delete;
        WindowedArray& operator=(WindowedArray&& rhs) = default;

        /// Retrieve number of windows allocated for this array.
        Idx numWindows() const
        {
            return this->x_.size() / this->windowSize_;
        }

        /// Retrieve number of data items per windows.
        Idx windowSize() const
        {
            return this->windowSize_;
        }

        /// Request read/write access to individual window.
        ///
        /// \param[in] window Numeric ID of particular read/write window.
        ///   Must be in range \code [0 .. numWindows()-1] \endcode.
        WriteWindow operator[](const Idx window)
        {
            assert ((window < this->numWindows()) &&
                    "Window ID Out of Bounds");

            auto b = std::begin(this->x_) + window*this->windowSize_;
            auto e = b + this->windowSize_;

            return { b, e };
        }

        /// Request read-only access to individual window.
        ///
        /// \param[in] window Numeric ID of particular read-only window.
        ///   Must be in range \code [0 .. numWindows()-1] \endcode.
        ReadWindow operator[](const Idx window) const
        {
            assert ((window < this->numWindows()) &&
                    "Window ID Out of Bounds");

            auto b = std::begin(this->x_) + window*this->windowSize_;
            auto e = b + this->windowSize_;

            return { b, e };
        }

        /// Get read-only access to full, linearised data items for
        /// all windows.
        const std::vector<T>& data() const
        {
            return this->x_;
        }

        /// Extract full, linearised data items for all windows.
        ///
        /// Destroys the internal state of the \c WindowedArray.
        std::vector<T> getDataDestructively()
        {
            return std::move(this->x_);
        }

    private:
        std::vector<T> x_;

        Idx windowSize_;
    };


    /// Provide read-only and read/write access to constantly sized
    /// portions/windows of a linearised buffer with an implied
    /// row/column matrix (2D array) structure.
    ///
    /// Intended as backing store for vectors that have a constant
    /// number of items per sub-entity of a fixed number of containing
    /// entities (e.g., K double precision data items for each of N
    /// maximum well connections for each of M maximum active wells at
    /// a particular report step).
    ///
    /// \tparam T Element type for underlying data items.
    template <typename T>
    class WindowedMatrix
    {
    private:
        using NumWindows  = typename WindowedArray<T>::NumWindows;

    public:
        using WriteWindow = typename WindowedArray<T>::WriteWindow;
        using ReadWindow  = typename WindowedArray<T>::ReadWindow;
        using WindowSize  = typename WindowedArray<T>::WindowSize;
        using Idx         = typename WindowedArray<T>::Idx;

        /// Distinct compile-time type for number of matrix rows
        /// in underlying storage.
        struct NumRows { Idx value; };

        /// Distinct compile-time type for number of matrix columns
        /// in underlying storage.
        struct NumCols { Idx value; };

        /// Constructor.
        ///
        /// \param[in] nRows Number of rows.
        /// \param[in] nCols Number of columns.
        /// \param[in] sz Number of data items per (row,column) window.
        explicit WindowedMatrix(const NumRows& nRows,
                                const NumCols& nCols,
                                const WindowSize& sz)
            : data_   (NumWindows{ nRows.value * nCols.value }, sz)
            , numCols_(nCols.value)
        {
            if (nCols.value == 0)
                throw std::invalid_argument("Window matrix with columns==0 is not permitted");
        }

        /// Retrieve number of columns allocated for this matrix.
        Idx numCols() const
        {
            return this->numCols_;
        }

        /// Retrieve number of rows allocated for this matrix.
        Idx numRows() const
        {
            return this->data_.numWindows() / this->numCols();
        }

        /// Retrieve number of data items per windows.
        Idx windowSize() const
        {
            return this->data_.windowSize();
        }

        /// Request read/write access to individual window.
        ///
        /// \param[in] row Numeric ID of particular row in matrix.
        ///   Must be in range \code [0 .. numRows()-1] \endcode.
        ///
        /// \param[in] col Numeric ID of particular column in matrix.
        ///   Must be in range \code [0 .. numCols()-1] \endcode.
        ///
        /// \return Read/write window at position \code (row,col) \endcode.
        WriteWindow operator()(const Idx row, const Idx col)
        {
            return this->data_[ this->i(row, col) ];
        }

        /// Request read-only access to individual window.
        ///
        /// \param[in] row Numeric ID of particular row in matrix.
        ///   Must be in range \code [0 .. numRows()-1] \endcode.
        ///
        /// \param[in] col Numeric ID of particular column in matrix.
        ///   Must be in range \code [0 .. numCols()-1] \endcode.
        ///
        /// \return Read-only window at position \code (row,col) \endcode.
        ReadWindow operator()(const Idx row, const Idx col) const
        {
            return this->data_[ this->i(row, col) ];
        }

        /// Get read-only access to full, linearised data items for
        /// all windows.
        auto data() const
            -> decltype(std::declval<const WindowedArray<T>>().data())
        {
            return this->data_.data();
        }

        /// Extract full, linearised data items for all windows.
        ///
        /// Destroys the internal state of the \c WindowedMatrix.
        auto getDataDestructively()
            -> decltype(std::declval<WindowedArray<T>>()
                        .getDataDestructively())
        {
            return this->data_.getDataDestructively();
        }

    private:
        WindowedArray<T> data_;

        Idx numCols_;

        /// Row major (C) order.
        Idx i(const Idx row, const Idx col) const
        {
            return row*this->numCols() + col;
        }
    };

}}} // Opm::RestartIO::Helpers

#endif // OPM_WINDOW_ARRAY_HPP
