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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/output/data/InterRegFlowMap.hpp>

#include <opm/output/data/InterRegFlow.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------
// Class Opm::data::InterRegFlowMap::Connections
// ---------------------------------------------------------------------

void
Opm::data::InterRegFlowMap::
Connections::add(const int r1, const int r2, const FlowRates& v)
{
    using ElmT = Window::ElmT;

    const auto one  = ElmT{1};
    const auto sign = (r1 < r2) ? one : -one;

    auto low = r1, high = r2;
    if (std::signbit(sign)) {
        std::swap(low, high);
    }

    this->i_.push_back(low);
    this->j_.push_back(high);

    this->max_i_ = std::max(this->max_i_, this->i_.back());
    this->max_j_ = std::max(this->max_j_, this->j_.back());

    const auto start = this->v_.size();
    this->v_.insert(this->v_.end(), Window::bufferSize(), ElmT{0});

    Window{ this->v_.begin() + start, this->v_.end() }.addFlow(sign, v);
}

void
Opm::data::InterRegFlowMap::
Connections::add(const int         maxRowIdx,
                 const int         maxColIdx,
                 const Neighbours& rows,
                 const Neighbours& cols,
                 const RateBuffer& rates)
{
    if (cols.size() != rows.size()) {
        throw std::invalid_argument {
            "Coordinate format column index table size does not match "
            "row index table size"
        };
    }

    if (rates.size() != Window::bufferSize() * rows.size()) {
        throw std::invalid_argument {
            "Coordinate format value table size does not match "
            "row index table size"
        };
    }

    this->i_.insert(this->i_.end(), rows .begin(), rows .end());
    this->j_.insert(this->j_.end(), cols .begin(), cols .end());
    this->v_.insert(this->v_.end(), rates.begin(), rates.end());

    this->max_i_ = std::max(this->max_i_, maxRowIdx);
    this->max_j_ = std::max(this->max_j_, maxColIdx);
}

void Opm::data::InterRegFlowMap::Connections::clear()
{
    this->v_.clear();
    this->j_.clear();
    this->i_.clear();

    this->max_i_ = -1;
    this->max_j_ = -1;
}

bool Opm::data::InterRegFlowMap::Connections::empty() const
{
    return this->i_.empty();
}

bool Opm::data::InterRegFlowMap::Connections::isValid() const
{
    return (this->i_.size() == this->j_.size())
        && (this->v_.size() == this->i_.size()*Window::bufferSize());
}

int Opm::data::InterRegFlowMap::Connections::maxRow() const
{
    return this->max_i_;
}

int Opm::data::InterRegFlowMap::Connections::maxCol() const
{
    return this->max_j_;
}

Opm::data::InterRegFlowMap::Neighbours::size_type
Opm::data::InterRegFlowMap::Connections::numContributions() const
{
    return this->i_.size();
}

const Opm::data::InterRegFlowMap::Neighbours&
Opm::data::InterRegFlowMap::Connections::rowIndices() const
{
    return this->i_;
}

const Opm::data::InterRegFlowMap::Neighbours&
Opm::data::InterRegFlowMap::Connections::columnIndices() const
{
    return this->j_;
}

const Opm::data::InterRegFlowMap::RateBuffer&
Opm::data::InterRegFlowMap::Connections::values() const
{
    return this->v_;
}

// =====================================================================

// ---------------------------------------------------------------------
// Class Opm::data::InterRegFlowMap::CSR
// ---------------------------------------------------------------------

void
Opm::data::InterRegFlowMap::
CSR::merge(const Connections& conns, const Offset numRegions)
{
    if (! conns.empty() &&
        (static_cast<Offset>(conns.maxRow()) >= numRegions))
    {
        throw std::invalid_argument {
            "Input graph contains more "
            "source regions than are "
            "implied by explicit size of "
            "adjacency matrix"
        };
    }

    this->assemble(conns.rowIndices(), conns.columnIndices(),
                   conns.maxRow(), conns.maxCol());
    this->compress(numRegions, conns.values());
}

std::optional<Opm::data::InterRegFlowMap::ReadOnlyWindow>
Opm::data::InterRegFlowMap::CSR::getWindow(const int i, const int j) const
{
    if ((i < 0) || (i >= static_cast<int>(this->numRows())) ||
        (j < 0) || (j >  this->maxColIdx()))
    {
        // Entity pair IDs out of range.
        return std::nullopt;
    }

    auto begin = this->columnIndices().begin() + this->startPointers()[i + 0];
    auto end   = this->columnIndices().begin() + this->startPointers()[i + 1];

    auto pos = std::lower_bound(begin, end, j);
    if ((pos == end) || (*pos > j)) {
        // Entity 'j' does not connect to entity 'i'.
        return std::nullopt;
    }

    // Entity 'j' connects to 'i'.  Form read-only view into sub-range
    // pertaining to this entity pair.
    const auto sz = ReadOnlyWindow::bufferSize();
    const auto windowID = pos - this->columnIndices().begin();
    auto start = this->values().begin() + windowID*sz;

    return { ReadOnlyWindow{ start, start + sz } };
}

Opm::data::InterRegFlowMap::Offset
Opm::data::InterRegFlowMap::CSR::numRows() const
{
    return this->startPointers().empty()
        ? 0 : this->startPointers().size() - 1;
}

int Opm::data::InterRegFlowMap::CSR::maxRowIdx() const
{
    return this->numRows_ - 1;
}

int Opm::data::InterRegFlowMap::CSR::maxColIdx() const
{
    return this->numCols_ - 1;
}

const Opm::data::InterRegFlowMap::Start&
Opm::data::InterRegFlowMap::CSR::startPointers() const
{
    return this->ia_;
}

const Opm::data::InterRegFlowMap::Neighbours&
Opm::data::InterRegFlowMap::CSR::columnIndices() const
{
    return this->ja_;
}

const Opm::data::InterRegFlowMap::RateBuffer&
Opm::data::InterRegFlowMap::CSR::values() const
{
    return this->sa_;
}

std::vector<int>
Opm::data::InterRegFlowMap::CSR::coordinateFormatRowIndices() const
{
    auto rowIdx = std::vector<int>{};

    if (this->ia_.empty()) {
        return rowIdx;
    }

    rowIdx.reserve(this->ia_.back());

    auto row = 0;

    const auto m = this->ia_.size() - 1;
    for (auto i = 0*m; i < m; ++i, ++row) {
        const auto n = this->ia_[i + 1] - this->ia_[i + 0];

        rowIdx.insert(rowIdx.end(), n, row);
    }

    return rowIdx;
}

void Opm::data::InterRegFlowMap::CSR::clear()
{
    this->ia_.clear();
    this->ja_.clear();
    this->sa_.clear();
    this->compressedIdx_.clear();

    this->numRows_ = 0;
    this->numCols_ = 0;
}

void
Opm::data::InterRegFlowMap::
CSR::assemble(const Neighbours& rows,
              const Neighbours& cols,
              const int         maxRowIdx,
              const int         maxColIdx)
{
    auto i = this->coordinateFormatRowIndices();
    i.insert(i.end(), rows.begin(), rows.end());

    auto j = this->ja_;
    j.insert(j.end(), cols.begin(), cols.end());

    const auto thisNumRows = std::max(this->numRows_, maxRowIdx + 1);
    const auto thisNumCols = std::max(this->numCols_, maxColIdx + 1);

    this->preparePushbackRowGrouping(thisNumRows, i);

    this->groupAndTrackColumnIndicesByRow(i, j);

    this->numRows_ = thisNumRows;
    this->numCols_ = thisNumCols;
}

void
Opm::data::InterRegFlowMap::
CSR::compress(const Offset      numRegions,
              const RateBuffer& rates)
{
    this->sortColumnIndicesPerRow();

    // Must be called *after* sortColumnIndicesPerRow().
    this->condenseDuplicates();

    {
        auto v = this->values();
        v.insert(v.end(), rates.begin(), rates.end());

        this->accumulateFlowRates(v);
    }

    const auto nRows = this->startPointers().size() - 1;
    if (nRows < numRegions) {
        this->ia_.insert(this->ia_.end(),
                         numRegions - nRows,
                         this->startPointers().back());
    }
}

void Opm::data::InterRegFlowMap::CSR::sortColumnIndicesPerRow()
{
    // Transposition is, in this context, effectively a linear time (O(nnz))
    // bucket insertion procedure.  In other words transposing the structure
    // twice creates a structure with column indices in (ascendingly) sorted
    // order.

    this->transpose();
    this->transpose();
}

void Opm::data::InterRegFlowMap::CSR::condenseDuplicates()
{
    // Note: Must be called *after* sort().

    const auto colIdx  = this->ja_;
    auto compressedIdx = this->compressedIdx_;
    auto end           = colIdx.begin();

    this->ja_.clear();
    this->compressedIdx_.clear();

    const auto numRows = this->ia_.size() - 1;
    for (auto row = 0*numRows; row < numRows; ++row) {
        auto begin = end;

        std::advance(end, this->ia_[row + 1] - this->ia_[row + 0]);

        const auto q = this->ja_.size();

        this->condenseAndTrackUniqueColumnsForSingleRow(begin, end);

        this->ia_[row + 0] = q;
    }

    this->remapCompressedIndex(std::move(compressedIdx));

    // Record final table sizes.
    this->ia_.back() = this->ja_.size();
}

void Opm::data::InterRegFlowMap::CSR::accumulateFlowRates(const RateBuffer& v)
{
    constexpr auto sz = Window::bufferSize();

    if (v.size() != this->compressedIdx_.size()*sz) {
        throw std::logic_error {
            "Flow rates must be provided for each connection"
        };
    }

    auto dst = [this](const Offset start) -> Window
    {
        auto begin = this->sa_.begin() + start*sz;

        return Window { begin, begin + sz };
    };

    auto src = [&v](const Offset start) -> ReadOnlyWindow
    {
        auto begin = v.begin() + start*sz;

        return ReadOnlyWindow { begin, begin + sz };
    };

    this->sa_.assign(this->ja_.size() * sz, Window::ElmT{0});

    const auto numRates = this->compressedIdx_.size();
    for (auto rateID = 0*numRates; rateID < numRates; ++rateID) {
        dst(this->compressedIdx_[rateID]) += src(rateID);
    }
}

void
Opm::data::InterRegFlowMap::
CSR::preparePushbackRowGrouping(const int         numRows,
                                const Neighbours& rowIdx)
{
    assert (numRows >= 0);

    this->ia_.assign(numRows + 1, 0);

    for (const auto& row : rowIdx) {
        this->ia_[row + 1] += 1;
    }

    // Note index range: 1..numRows inclusive.
    for (Start::size_type i = 1, n = numRows; i <= n; ++i) {
        this->ia_[0] += this->ia_[i];
        this->ia_[i]  = this->ia_[0] - this->ia_[i];
    }

    assert (this->ia_[0] == rowIdx.size());
}

void
Opm::data::InterRegFlowMap::
CSR::groupAndTrackColumnIndicesByRow(const Neighbours& rowIdx,
                                     const Neighbours& colIdx)
{
    assert (this->ia_[0] == rowIdx.size());

    const auto nnz = rowIdx.size();

    this->ja_.resize(nnz);

    this->compressedIdx_.clear();
    this->compressedIdx_.reserve(nnz);

    for (auto nz = 0*nnz; nz < nnz; ++nz) {
        const auto k = this->ia_[rowIdx[nz] + 1] ++;

        this->ja_[k] = colIdx[nz];
        this->compressedIdx_.push_back(k);
    }

    this->ia_[0] = 0;
}

void Opm::data::InterRegFlowMap::CSR::transpose()
{
    auto compressedIdx = this->compressedIdx_;

    {
        const auto rowIdx = this->coordinateFormatRowIndices();
        const auto colIdx = this->ja_;

        this->preparePushbackRowGrouping(this->numCols_, colIdx);

        // Note parameter order.  Transposition switches role of rows and
        // columns.
        this->groupAndTrackColumnIndicesByRow(colIdx, rowIdx);
    }

    this->remapCompressedIndex(std::move(compressedIdx));

    std::swap(this->numRows_, this->numCols_);
}

void
Opm::data::InterRegFlowMap::CSR::
condenseAndTrackUniqueColumnsForSingleRow(Neighbours::const_iterator begin,
                                          Neighbours::const_iterator end)
{
    // We assume that we're only called *after* sortColumnIndicesPerRow()
    // whence duplicate elements appear consecutively in [begin, end).
    //
    // Note: This is essentially the same as std::unique(begin, end) save
    // for the return value and the fact that we additionally record the
    // 'compressedIdx_' mapping.  That mapping enables subsequent, decoupled
    // accumulation of the 'sa_' contributions.

    while (begin != end) {
        // Note: Order of ja_ and compressedIdx_ matters here.
        this->compressedIdx_.push_back(this->ja_.size());
        this->ja_           .push_back(*begin);

        while ((++begin != end) &&
               ( *begin == this->ja_.back()))
        {
            this->compressedIdx_.push_back(this->compressedIdx_.back());
        }
    }
}

void
Opm::data::InterRegFlowMap::CSR::
remapCompressedIndex(Start&& compressedIdx)
{
    for (auto& i : compressedIdx) {
        i = this->compressedIdx_[i];
    }

    this->compressedIdx_.swap(compressedIdx);
}

// =====================================================================

// ---------------------------------------------------------------------
// Class Opm::data::InterRegFlowMap
// ---------------------------------------------------------------------

void
Opm::data::InterRegFlowMap::
addConnection(const int        r1,
              const int        r2,
              const FlowRates& rates)
{
    if ((r1 < 0) || (r2 < 0)) {
        throw std::invalid_argument {
            "Region indices must be non-negative.  Got (r1,r2) = ("
            + std::to_string(r1) + ", " + std::to_string(r2)
            + ')'
        };
    }

    if (r1 == r2) {
        // Internal to a region.  Skip.
        return;
    }

    this->uncompressed_.add(r1, r2, rates);
}

void Opm::data::InterRegFlowMap::compress(const std::size_t numRegions)
{
    if (! this->uncompressed_.isValid()) {
        throw std::logic_error {
            "Cannot compress invalid connection list"
        };
    }

    this->csr_.merge(this->uncompressed_, numRegions);

    this->uncompressed_.clear();
}

Opm::data::InterRegFlowMap::Offset
Opm::data::InterRegFlowMap::numRegions() const
{
    return this->csr_.numRows();
}

std::optional<std::pair<
    Opm::data::InterRegFlowMap::ReadOnlyWindow,
    Opm::data::InterRegFlowMap::ReadOnlyWindow::ElmT
>>
Opm::data::InterRegFlowMap::getInterRegFlows(const int r1, const int r2) const
{
    if ((r1 < 0) || (r2 < 0)) {
        throw std::invalid_argument {
            "Region indices must be non-negative.  Got (r1,r2) = ("
            + std::to_string(r1) + ", " + std::to_string(r2)
            + ')'
        };
    }

    if (r1 == r2) {
        // Internal to a region.  Skip.
        throw std::invalid_argument {
            "Region indices must be distinct.  Got (r1,r2) = ("
            + std::to_string(r1) + ", " + std::to_string(r2)
            + ')'
        };
    }

    using ElmT = ReadOnlyWindow::ElmT;

    const auto one  = ElmT{1};
    const auto sign = (r1 < r2) ? one : -one;

    auto low = r1, high = r2;
    if (std::signbit(sign)) {
        std::swap(low, high);
    }

    auto window = this->csr_.getWindow(low, high);
    if (! window.has_value()) {
        // High is not connected to low.
        return std::nullopt;
    }

    return std::make_pair(std::move(window.value()), sign);
}

void Opm::data::InterRegFlowMap::clear()
{
    this->uncompressed_.clear();
    this->csr_.clear();
}
