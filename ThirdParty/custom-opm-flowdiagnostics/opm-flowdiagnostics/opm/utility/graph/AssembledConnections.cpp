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

#include <opm/utility/graph/AssembledConnections.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <ios>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <utility>

// ---------------------------------------------------------------------
// Class Opm::AssembledConnections::Connections
// ---------------------------------------------------------------------

void
Opm::AssembledConnections::
Connections::add(const int i, const int j)
{
    i_.push_back(i);
    j_.push_back(j);

    max_i_ = std::max(max_i_, i_.back());
    max_j_ = std::max(max_j_, j_.back());
}

void
Opm::AssembledConnections::
Connections::add(const int i, const int j, const double v)
{
    this->add(i, j);

    v_.push_back(v);
}

void
Opm::AssembledConnections::Connections::clear()
{
    WeightVector().swap(v_);
    EntityVector().swap(j_);
    EntityVector().swap(i_);
}

bool
Opm::AssembledConnections::Connections::empty() const
{
    return i_.empty();
}

bool
Opm::AssembledConnections::Connections::isValid() const
{
    return (i_.size() == j_.size())
        && (v_.empty() || (v_.size() == i_.size()));
}

bool
Opm::AssembledConnections::Connections::isWeighted() const
{
    return ! v_.empty();
}

int
Opm::AssembledConnections::Connections::maxRow() const
{
    return max_i_;
}

int
Opm::AssembledConnections::Connections::maxCol() const
{
    return max_j_;
}

Opm::AssembledConnections::Connections::EntityVector::size_type
Opm::AssembledConnections::Connections::nnz() const
{
    return i_.size();
}

const Opm::AssembledConnections::Connections::EntityVector&
Opm::AssembledConnections::Connections::i() const
{
    return i_;
}

const Opm::AssembledConnections::Connections::EntityVector&
Opm::AssembledConnections::Connections::j() const
{
    return j_;
}

const Opm::AssembledConnections::Connections::WeightVector&
Opm::AssembledConnections::Connections::v() const
{
    return v_;
}

// =====================================================================

// ---------------------------------------------------------------------
// Class Opm::AssembledConnections::CSR
// ---------------------------------------------------------------------

void
Opm::AssembledConnections::
CSR::create(const Connections& conns, const Offset numrows)
{
    if (! conns.empty() &&
        (static_cast<Offset>(conns.maxRow()) >= numrows))
    {
        throw std::invalid_argument("Input graph contains more "
                                    "source vertices than are "
                                    "implied by explicit size of "
                                    "adjacency matrix");
    }

    this->assemble(conns);

    this->sort();

    // Must be called *after* sort().
    this->condenseDuplicates();

    if (conns.isWeighted()) {
        this->accumulateConnWeights(conns.v());
    }

    const auto nRows = this->ia().size() - 1;
    if (nRows < numrows) {
        this->ia_.insert(this->ia_.end(),
                         numrows - nRows,
                         this->ia().back());
    }
}

const Opm::AssembledConnections::Start&
Opm::AssembledConnections::CSR::ia() const
{
    return ia_;
}

const Opm::AssembledConnections::Neighbours&
Opm::AssembledConnections::CSR::ja() const
{
    return ja_;
}

const Opm::AssembledConnections::ConnWeight&
Opm::AssembledConnections::CSR::sa() const
{
    return sa_;
}

void
Opm::AssembledConnections::
CSR::assemble(const Connections& conns)
{
    {
        const auto numRows = conns.maxRow() + 1;

        this->accumulateRowEntries(numRows, conns.i());
    }

    this->createGraph(conns.i(), conns.j());

    this->numRows_ = conns.maxRow() + 1;
    this->numCols_ = conns.maxCol() + 1;
}

void
Opm::AssembledConnections::CSR::sort()
{
    // Transposition is, effectively, a linear time bucket insert, so
    // transposing the structure twice creates a structure with colum
    // indices in (ascendingly) sorted order.

    this->transpose();
    this->transpose();
}

void
Opm::AssembledConnections::CSR::condenseDuplicates()
{
    // Note: Must be called *after* sort().

    const auto colIdx = this->ja_;
    auto elmIdx       = this->elmIdx_;
    auto end          = colIdx.begin();

    this->ja_    .clear();
    this->elmIdx_.clear();

    for (decltype(this->ia_.size())
             row = 0, numRows = this->ia_.size() - 1;
         row < numRows; ++row)
    {
        auto begin = end;

        std::advance(end, this->ia_[row + 1] - this->ia_[row + 0]);

        const auto q = this->ja_.size();

        this->unique(begin, end);

        this->ia_[row + 0] = q;
    }

    this->remapElementIndex(std::move(elmIdx));

    // Record final table sizes.
    this->ia_.back() = this->ja_.size();
}

void
Opm::AssembledConnections::
CSR::accumulateConnWeights(const std::vector<double>& v)
{
    if (v.size() != this->elmIdx_.size()) {
        throw std::logic_error("Connection Weights must be "
                               "provided for each connection");
    }

    this->sa_.assign(this->ja_.size(), 0.0);

    for (decltype(v.size())
             conn = 0, numConn = v.size();
         conn < numConn; ++conn)
    {
        this->sa_[ this->elmIdx_[conn] ] += v[conn];
    }
}

void
Opm::AssembledConnections::
CSR::accumulateRowEntries(const int               numRows,
                          const std::vector<int>& rowIdx)
{
    assert (numRows >= 0);

    auto vecIdx = [](const int i)
    {
        return static_cast<Start::size_type>(i);
    };

    this->ia_.assign(vecIdx(numRows) + 1, vecIdx(0));

    for (const auto& row : rowIdx) {
        this->ia_[vecIdx(row) + 1] += 1;
    }

    // Note index range: 1..numRows inclusive.
    for (Start::size_type
             i = 1, n = vecIdx(numRows);
         i <= n; ++i)
    {
        this->ia_[0] += this->ia_[i];
        this->ia_[i]  = this->ia_[0] - this->ia_[i];
    }
}

void
Opm::AssembledConnections::
CSR::createGraph(const std::vector<int>& rowIdx,
                 const std::vector<int>& colIdx)
{
    assert (this->ia_[0] == rowIdx.size());

    auto vecIdx = [](const int i)
    {
        return static_cast<Start::size_type>(i);
    };

    this->ja_.resize(rowIdx.size());

    this->elmIdx_.clear();
    this->elmIdx_.reserve(rowIdx.size());

    for (decltype(rowIdx.size())
             nz = 0, nnz = rowIdx.size();
         nz < nnz; ++nz)
    {
        const auto k = ia_[vecIdx(rowIdx[nz]) + 1] ++;

        this->ja_[k] = colIdx[nz];

        this->elmIdx_.push_back(k);
    }

    this->ia_[0] = 0;
}

void
Opm::AssembledConnections::CSR::transpose()
{
    auto elmIdx = this->elmIdx_;

    {
        const auto rowIdx = this->expandStartPointers();
        const auto colIdx = this->ja_;

        this->accumulateRowEntries(this->numCols_, colIdx);

        this->createGraph(colIdx, rowIdx);
    }

    this->remapElementIndex(std::move(elmIdx));

    std::swap(this->numRows_, this->numCols_);
}

std::vector<int>
Opm::AssembledConnections::
CSR::expandStartPointers() const
{
    auto rowIdx = std::vector<int>{};
    rowIdx.reserve(this->ia_.back());

    auto row = 0;

    for (decltype(this->ia_.size())
             i = 0, m = this->ia_.size() - 1; i < m; ++i, ++row)
    {
        const auto n = this->ia_[i + 1] - this->ia_[i + 0];

        rowIdx.insert(rowIdx.end(), n, row);
    }

    return rowIdx;
}

void
Opm::AssembledConnections::
CSR::unique(Neighbours::const_iterator begin,
            Neighbours::const_iterator end)
{
    // We assume that we're only called *after* sort() whence duplicate
    // elements appear consecutively in [begin, end).
    //
    // Note: This is essentially the same as std::unique(begin, end) save
    // for the return value.  However, we also record the 'elmIdx_' mapping
    // to later have the ability to accumulate 'sa_'.

    while (begin != end) {
        // Note: Order of ja_ and elmIdx_ matters here.
        this->elmIdx_.push_back(this->ja_.size());
        this->ja_    .push_back(*begin);

        while ((++begin != end) &&
               ( *begin == this->ja_.back()))
        {
            this->elmIdx_.push_back(this->elmIdx_.back());
        }
    }
}

void
Opm::AssembledConnections::CSR::remapElementIndex(Start&& elmIdx)
{
    for (auto& i : elmIdx) {
        i = this->elmIdx_[i];
    }

    this->elmIdx_.swap(elmIdx);
}

// =====================================================================

// ---------------------------------------------------------------------
// Class Opm::AssembledConnections
// ---------------------------------------------------------------------

void
Opm::AssembledConnections::
addConnection(const int i,
              const int j)
{
    conns_.add(i, j);
}

void
Opm::AssembledConnections::
addConnection(const int    i,
              const int    j,
              const double v)
{
    conns_.add(i, j, v);
}

void
Opm::AssembledConnections::compress(const std::size_t numRows)
{
    if (! conns_.isValid()) {
        throw std::logic_error("Cannot compress invalid "
                               "connection list");
    }

    csr_.create(conns_, numRows);

    conns_.clear();
}

Opm::AssembledConnections::Offset
Opm::AssembledConnections::numRows() const
{
    return this->startPointers().size() - 1;
}

const Opm::AssembledConnections::Start&
Opm::AssembledConnections::startPointers() const
{
    return csr_.ia();
}

const Opm::AssembledConnections::Neighbours&
Opm::AssembledConnections::neighbourhood() const
{
    return csr_.ja();
}

const Opm::AssembledConnections::ConnWeight&
Opm::AssembledConnections::connectionWeight() const
{
    return csr_.sa();
}

Opm::AssembledConnections::CellNeighbours
Opm::AssembledConnections::cellNeighbourhood(const int cell) const
{
    const Offset beg = startPointers()[cell];
    const Offset end = startPointers()[cell + 1];
    const int* nb = neighbourhood().data();
    assert(connectionWeight().size() == neighbourhood().size());
    const double* w = connectionWeight().data();
    return CellNeighbours{ {nb + beg, w + beg}, {nb + end, w + end} };
}




// Note: not a member of class AssembledConnections.
std::ostream&
Opm::operator<<(std::ostream& os, const Opm::AssembledConnections& ac)
{
    // Set output stream format, store original settings.
    const auto oprec = os.precision(16);
    const auto oflags = os.setf(std::ios_base::scientific);

    // Write connections cell-by-cell.
    const int num_cells = ac.numRows();
    for (int cell = 0; cell < num_cells; ++cell) {
        const auto nb = ac.cellNeighbourhood(cell);
        for (const auto& conn : nb) {
            os << cell << ' ' << conn.neighbour << ' ' << conn.weight << '\n';
        }
    }

    // Restore original stream settings.
    os.precision(oprec);
    os.setf(oflags);

    return os;
}
