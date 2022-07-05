//===========================================================================
//
// File: SparseVector.hpp
//
// Created: Mon Jun 29 15:28:59 2009
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//            Bård Skaflestad     <bard.skaflestad@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

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


#ifndef OPM_SPARSEVECTOR_HEADER
#define OPM_SPARSEVECTOR_HEADER

#include <vector>
#include <numeric>
#include <algorithm>
#include <opm/common/ErrorMacros.hpp>

namespace Opm
{

    /// A SparseVector stores a vector with possibly many empty elements
    /// as efficiently as possible.
    /// It is supposed to behave similarly to a standard vector, but since
    /// direct indexing is a O(log n) operation instead of O(1), we do not
    /// supply it as operator[].
    template <typename T>
    class SparseVector
    {
    public:
	/// Default constructor. Yields an empty SparseVector.
	SparseVector()
	    : size_(0), default_elem_()
	{
	}

	/// Constructs a SparseVector with a given size, but no nonzero
	/// elements.
	explicit SparseVector(int sz)
	    : size_(sz), default_elem_()
	{
	}

	/// A constructor taking all the element data for the vector and their indices.
	/// \param data_beg The start of the element data.
	/// \param data_end One-beyond-end of the element data.
	/// \param rowsize_beg The start of the index data.
	/// \param rowsize_end One beyond the end of the index data.
	template <typename DataIter, typename IntegerIter>
	SparseVector(int sz,
		     DataIter data_beg, DataIter data_end,
		     IntegerIter index_beg, IntegerIter index_end)
	    : size_(sz), data_(data_beg, data_end), indices_(index_beg, index_end),
	      default_elem_()
	{
#ifndef NDEBUG
	    OPM_ERROR_IF(sz < 0, "The size of a SparseVector must be non-negative");
	    OPM_ERROR_IF(indices_.size() != data_.size(), "The number of indices of a SparseVector must equal to the number of entries");
	    int last_index = -1;
	    int num_ind = indices_.size();
	    for (int i = 0; i < num_ind; ++i) {
		int index = indices_[i];
		if (index <= last_index || index >= sz) {
		    OPM_THROW(std::logic_error, "Error in SparseVector construction, index is nonincreasing or out of range.");
		}
		last_index = index;
	    }
#endif
	}


	/// Appends an element to the vector. Note that this function does not
	/// increase the size() of the vector, it just adds another nonzero element.
	/// Elements must be added in index order.
	void addElement(const T& elem, int index)
	{
	    assert(indices_.empty() || index > indices_.back());
	    assert(index < size_);
	    data_.push_back(elem);
	    indices_.push_back(index);
	}

	/// \return true if the vector has size 0.
	bool empty() const
	{
	    return size_ == 0;
	}

	/// Returns the size of the vector.
	/// Recall that most or all of the vector may be default/zero.
	int size() const
	{
	    return size_;
	}

	/// Returns the number of nonzero data elements.
	int nonzeroSize() const
	{
	    return data_.size();
	}

	/// Makes the vector empty().
	void clear()
	{
	    data_.clear();
	    indices_.clear();
	    size_ = 0;
	}

	/// Equality.
	bool operator==(const SparseVector& other) const
	{
	    return size_ == other.size_ && data_ == other.data_ && indices_ == other.indices_;
	}

	/// O(log n) element access.
	/// \param index the proper vector index
	/// \return the element with the given index, or the default element if no element in
	/// the vector has the given index.
	const T& element(int index) const
	{
#ifndef NDEBUG
	    OPM_ERROR_IF(index < 0, "The index of a SparseVector must be non-negative (is " << index << ")");
	    OPM_ERROR_IF(index >= size_, "The index of a SparseVector must be smaller than the maximum value (is " << index << ", max value: " << size_ <<")");
#endif
	    std::vector<int>::const_iterator lb = std::lower_bound(indices_.begin(), indices_.end(), index);
	    if (lb != indices_.end() && *lb == index) {
		return data_[lb - indices_.begin()];
	    } else {
		return default_elem_;
	    }
	}

	/// O(1) element access.
	/// \param nzindex an index counting only nonzero elements.
	/// \return the nzindex'th nonzero element.
	const T& nonzeroElement(int nzindex) const
	{
#ifndef NDEBUG
	    OPM_ERROR_IF(nzindex < 0, "The index of a SparseVector must be non-negative (is " << nzindex << ")");
	    OPM_ERROR_IF(nzindex >= nonzeroSize(), "The index of a SparseVector must be smaller than the maximum value (is " << nzindex << ", max value: " << nonzeroSize() <<")");
#endif
	    return data_[nzindex];
	}

	/// O(1) index access.
	/// \param nzindex an index counting only nonzero elements.
	/// \return the index of the nzindex'th nonzero element.
	int nonzeroIndex(int nzindex) const
	{
	    assert(nzindex >= 0);
	    assert(nzindex < nonzeroSize());
	    return indices_[nzindex];
	}

    private:
	// The vectors data_ and indices_ are always the same size.
	// The indices are supposed to be stored in increasing order,
	// to be unique, and to be in [0, size_ - 1].
	// default_elem_ is returned when a default element is requested.
	int size_;
	std::vector<T> data_;
	std::vector<int> indices_;
	T default_elem_;
    };

} // namespace Opm




#endif // OPM_SPARSEVECTOR_HEADER
