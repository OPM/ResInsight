// $Id: grid.hpp 882 2011-09-23 13:10:16Z perroe $

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

#ifndef NRLIB_GRID_HPP
#define NRLIB_GRID_HPP

#include <cassert>
#include <sstream>
#include <vector>
#include <limits>

//#include "../../../src/definitions.h"

namespace NRLib {

template<class A>
class Grid {
public:
  typedef typename std::vector<A>::iterator iterator;
  typedef typename std::vector<A>::const_iterator const_iterator;
  typedef typename std::vector<A>::reference reference;
  typedef typename std::vector<A>::const_reference const_reference;

  Grid();
    /// \param val Initial cell value.
  Grid(size_t ni, size_t nj, size_t nk, const A& val = A());
  virtual ~Grid();

    /// All values in the grid are erased when the grid is
    /// resized.
    /// \param val Initial cell value.
  void Resize(size_t ni, size_t nj, size_t nk, const A& val = A());
  void GetAvgMinMax(A& avg, A& min, A& max);
  void GetAvgMinMaxWithMissing(A& avg, A& min, A& max, A missing);
  void LogTransform(A missing);

  inline reference operator()(size_t i, size_t j, size_t k);
  inline reference operator()(size_t index);
  inline reference GetValue(size_t i, size_t j, size_t k);

  inline const_reference operator()(size_t i, size_t j, size_t k) const;
  inline const_reference operator()(size_t index) const;
  inline const_reference GetValue(size_t i, size_t j, size_t k) const;

  iterator       begin() {return( data_.begin()); }
  iterator       end()   {return( data_.end()); }

  const_iterator begin() const { return(data_.begin()); }
  const_iterator end()   const { return(data_.end()); }

  size_t         GetNI() const { return(ni_); }
  size_t         GetNJ() const { return(nj_); }
  size_t         GetNK() const { return(nk_); }
  size_t         GetN()  const { return data_.size(); }

  inline size_t  GetIndex(size_t i, size_t j, size_t k) const;
  void           GetIJK(size_t index, size_t &i, size_t &j, size_t &k) const;
  void           SetValue(size_t i, size_t j, size_t k, const A& value);

  void           Swap(Grid<A>& other);

private:
  size_t ni_;
  size_t nj_;
  size_t nk_;

  /// The grid data, column-major ordering.
  std::vector<A> data_;
};

template<class A>
Grid<A>::Grid()
  : ni_(0),
    nj_(0),
    nk_(0),
    data_()
{}

template<class A>
Grid<A>::Grid(size_t ni, size_t nj, size_t nk, const A& val)
  : ni_(ni),
    nj_(nj),
    nk_(nk),
    data_(ni*nj*nk, val)
{}

template<class A>
Grid<A>::~Grid()
{}

template<class A>
void Grid<A>::Resize(size_t ni, size_t nj, size_t nk, const A& val)
{
  ni_ = ni;
  nj_ = nj;
  nk_ = nk;

  data_.resize(0); //To avoid copying of elements
  data_.resize(ni_ * nj_ * nk_, val);
}

template<class A>
void Grid<A>::GetAvgMinMax(A& avg, A& min, A& max)
{
  A sum   = 0.0;
  A value = 0.0;
  max = -std::numeric_limits<A>::infinity();
  min = +std::numeric_limits<A>::infinity();

  for(unsigned int i = 0; i < ni_; i++) {
    for(unsigned int j = 0; j < nj_; j++) {
      for(unsigned int k = 0; k < nk_; k++) {
        value = data_[GetIndex(i, j, k)];
        sum += value;

        if(value > max)
          max = value;

        if(value < min)
          min = value;

      }
    }
  }
  avg = sum /= GetN();
}

template<class A>
void Grid<A>::GetAvgMinMaxWithMissing(A& avg, A& min, A& max, A missing)
{
  A sum   = 0.0;
  A value = 0.0;
  max = -std::numeric_limits<A>::infinity();
  min = +std::numeric_limits<A>::infinity();

  unsigned int n = 0;
  for(unsigned int i = 0; i < ni_; i++) {
    for(unsigned int j = 0; j < nj_; j++) {
      for(unsigned int k = 0; k < nk_; k++) {
        value = data_[GetIndex(i, j, k)];
        if(value != missing) {
          sum += value;
          n++;
          if(value > max)
            max = value;

          if(value < min)
            min = value;
        }
      }
    }
  }
  avg = sum/static_cast<A>(n);
}

template<class A>
void Grid<A>::LogTransform(A missing)
{
  for(size_t i = 0; i < ni_; i++) {
    for(size_t j = 0; j < nj_; j++) {
      for(size_t k = 0; k < nk_; k++) {
        A value = data_[GetIndex(i, j, k)];
        if(value == missing || value <= 0.0) //First RMISSING
          data_[GetIndex(i, j, k)] = 0.0;
        else
          data_[GetIndex(i, j, k)] = log(value);
      }
    }
  }
}

template<class A>
typename Grid<A>::reference Grid<A>::operator()(size_t i, size_t j, size_t k)
{
  return data_[GetIndex(i, j, k)];
}

template<class A>
typename Grid<A>::reference Grid<A>::operator()(size_t index)
{
  assert(index < GetN());

  return data_[index];
}

template<class A>
typename Grid<A>::reference Grid<A>::GetValue(size_t i, size_t j, size_t k)
{
  return data_[GetIndex(i, j, k)];
}

template<class A>
typename Grid<A>::const_reference Grid<A>::operator()(size_t i, size_t j, size_t k) const
{
  return data_[GetIndex(i, j, k)];
}

template<class A>
typename Grid<A>::const_reference Grid<A>::operator()(size_t index) const
{
  assert(index < GetN());

  return data_[index];
}

template<class A>
typename Grid<A>::const_reference Grid<A>::GetValue(size_t i, size_t j, size_t k) const
{
  return data_[GetIndex(i, j, k)];
}

template<class A>
size_t Grid<A>::GetIndex(size_t i, size_t j, size_t k) const
{
  assert(i < GetNI() && j < GetNJ() && k < GetNK());

  return i + j*ni_ + k*ni_*nj_;
}

template<class A>
void Grid<A>::GetIJK(size_t index, size_t &i, size_t &j, size_t &k) const
{
  assert(index < GetN());

  i = index % ni_;
  j = (index-i)/ni_ % nj_;
  k = (index - j*ni_ - i)/ni_/nj_;
}

template<class A>
void Grid<A>::SetValue(size_t i, size_t j, size_t k, const A& value)
{
  data_[GetIndex(i, j, k)] = value;
}

template<class A>
void Grid<A>::Swap(NRLib::Grid<A> &other)
{
  std::swap(ni_, other.ni_);
  std::swap(nj_, other.nj_);
  std::swap(nk_, other.nk_);
  data_.swap(other.data_);
}

}
#endif







