// $Id: grid2d.hpp 882 2011-09-23 13:10:16Z perroe $

// Copyright (c)  2011, Norwegian Computing Center
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// •  Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// •  Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef NRLIB_GRID2D_HPP
#define NRLIB_GRID2D_HPP

#include <cassert>
#include <sstream>
#include <vector>

namespace NRLib {

template<class A>
class Grid2D {
public:
  typedef typename std::vector<A>::iterator        iterator;
  typedef typename std::vector<A>::const_iterator  const_iterator;
  typedef typename std::vector<A>::reference       reference;
  typedef typename std::vector<A>::const_reference const_reference;

  Grid2D();
    /// \param val Initial cell value.
  Grid2D(size_t ni, size_t nj, const A& val = A());
  virtual ~Grid2D();

    /// All values in the grid are erased when the grid is
    /// resized.
    /// \param val Initial cell value.
  virtual void           Resize(size_t ni, size_t nj, const A& val = A());

  inline reference       operator()(size_t i, size_t j);
  inline reference       operator()(size_t index);

  inline const_reference operator()(size_t i, size_t j) const;
  inline const_reference operator()(size_t index) const;

  iterator               begin()       { return data_.begin(); }
  iterator               end()         { return data_.end(); }

  const_iterator         begin() const { return data_.begin(); }
  const_iterator         end()   const { return data_.end(); }

  size_t                 GetNI() const { return ni_; }
  size_t                 GetNJ() const { return nj_; }
  size_t                 GetN()  const { return data_.size(); }

  inline size_t          GetIndex(size_t i, size_t j) const;
  void                   GetIJ(size_t index, size_t &i, size_t &j) const;

  bool                   IsValidIndex(int i, int j) const;

  void                   Swap(Grid2D<A>& other);

  A                      FindMin(A missingValue) const;
  A                      FindMax(A missingValue) const;

private:
  size_t ni_;
  size_t nj_;
  /// The grid data, column-major ordering.
  std::vector<A> data_;
};

template<class A>
Grid2D<A>::Grid2D()
  : ni_(0),
    nj_(0),
    data_()
{}

template<class A>
Grid2D<A>::Grid2D(size_t ni, size_t nj, const A& val)
  : ni_(ni),
    nj_(nj),
    data_(ni*nj, val)
{}

template<class A>
Grid2D<A>::~Grid2D()
{}

template<class A>
void Grid2D<A>::Resize(size_t ni, size_t nj, const A& val)
{
  ni_ = ni;
  nj_ = nj;

  data_.resize(0); //To avoid copying of elements
  data_.resize(ni_ * nj_, val);
}


template<class A>
typename Grid2D<A>::reference Grid2D<A>::operator()(size_t i, size_t j)
{
  return(data_[GetIndex(i, j)]);
}


template<class A>
typename Grid2D<A>::reference Grid2D<A>::operator()(size_t index)
{
  assert(index < GetN());

  return(data_[index]);
}


template<class A>
typename Grid2D<A>::const_reference Grid2D<A>::operator()(size_t i, size_t j) const
{
  return(data_[GetIndex(i, j)]);
}


template<class A>
typename Grid2D<A>::const_reference Grid2D<A>::operator()(size_t index) const
{
  assert(index < GetN());

  return(data_[index]);
}


template<class A>
size_t Grid2D<A>::GetIndex(size_t i, size_t j) const
{
  assert(i < ni_);
  assert(j < nj_);

  return(i+j*ni_);
}

template<class A>
void Grid2D<A>::GetIJ(size_t index, size_t &i, size_t &j) const
{
  assert (index < GetN());

  i = (index % ni_);
  j = ((index-i)/ni_ % nj_);
}


template<class A>
bool Grid2D<A>::IsValidIndex(int i, int j) const
{
  if (i >= 0 && static_cast<size_t>(i) < ni_ &&
      j >= 0 && static_cast<size_t>(j) < nj_)
    return true;

  return false;
}


template<class A>
void Grid2D<A>::Swap(NRLib::Grid2D<A> &other)
{
  std::swap(ni_, other.ni_);
  std::swap(nj_, other.nj_);
  data_.swap(other.data_);
}

template<class A>
A Grid2D<A>::FindMin(A missingValue) const
{
  A minVal = (*this)(0);
  typename std::vector<A>::const_iterator i;
  for (i = this->begin(); i < this->end(); i++) {
    if ((minVal == missingValue || (*i) < minVal) && (*i) != missingValue)
      minVal = *i;
  }
  return minVal;
}

template<class A>
A Grid2D<A>::FindMax(A missingValue) const
{
  A maxVal = (*this)(0);
  typename std::vector<A>::const_iterator i;
  for (i = this->begin(); i < this->end(); i++) {
    if ((maxVal == missingValue || (*i) > maxVal) && (*i) != missingValue)
      maxVal = *i;
  }
  return maxVal;
}

} // namespace NRLib

#endif // NRLIB_GRID2D_HPP
