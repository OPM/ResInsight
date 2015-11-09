// $Id: grid5d.hpp 1142 2013-03-18 15:13:41Z hgolsen $

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

#ifndef NRLIB_GRID5D_HPP
#define NRLIB_GRID5D_HPP

#include <cassert>
#include <sstream>
#include <vector>

namespace NRLib {

template<class A>
class Grid5D {
public:
  typedef typename std::vector<A>::iterator iterator;
  typedef typename std::vector<A>::const_iterator const_iterator;
  typedef typename std::vector<A>::reference reference;
  typedef typename std::vector<A>::const_reference const_reference;

  Grid5D();
    /// \param val Initial cell value.
  Grid5D(size_t ni, size_t nj, size_t nk, size_t nl, size_t nm, const A& val = A());
  virtual ~Grid5D();

    /// All values in the grid are erased when the grid is
    /// resized.
    /// \param val Initial cell value.
  void Resize(size_t ni, size_t nj, size_t nk, size_t nl, size_t nm, const A& val = A());

  inline reference operator()(size_t i, size_t j, size_t k, size_t l, size_t m);
  inline reference operator()(size_t index);

  inline const_reference operator()(size_t i, size_t j, size_t k, size_t l, size_t m) const;
  inline const_reference operator()(size_t index) const;

  iterator       begin() {return( data_.begin()); }
  iterator       end()   {return( data_.end()); }

  const_iterator begin() const { return(data_.begin()); }
  const_iterator end()   const { return(data_.end()); }

  size_t         GetNI() const { return(ni_); }
  size_t         GetNJ() const { return(nj_); }
  size_t         GetNK() const { return(nk_); }
  size_t         GetNL() const { return(nl_); }
  size_t         GetNM() const { return(nm_); }
  size_t         GetN()  const { return data_.size(); }

  inline size_t  GetIndex(size_t i, size_t j, size_t k, size_t l, size_t m) const;
  void           GetIJKLM(size_t index, size_t &i, size_t &j, size_t &k, size_t &l, size_t &m) const;

  void           Swap(Grid5D<A>& other);

private:
  size_t ni_;
  size_t nj_;
  size_t nk_;
  size_t nl_;
  size_t nm_;

  /// The grid data, column-major ordering.
  std::vector<A> data_;
};

template<class A>
Grid5D<A>::Grid5D()
  : ni_(0),
    nj_(0),
    nk_(0),
    nl_(0),
    nm_(0),
    data_()
{}

template<class A>
Grid5D<A>::Grid5D(size_t ni, size_t nj, size_t nk, size_t nl, size_t nm, const A& val)
  : ni_(ni),
    nj_(nj),
    nk_(nk),
    nl_(nl),
    nm_(nm),
    data_(ni*nj*nk*nl*nm, val)
{}

template<class A>
Grid5D<A>::~Grid5D()
{}

template<class A>
void Grid5D<A>::Resize(size_t ni, size_t nj, size_t nk, size_t nl, size_t nm, const A& val)
{
  ni_ = ni;
  nj_ = nj;
  nk_ = nk;
  nl_ = nl;
  nm_ = nm

  data_.resize(0); //To avoid copying of elements
  data_.resize(ni_ * nj_ * nk_ * nl * nm,  val);
}


template<class A>
typename Grid5D<A>::reference Grid5D<A>::operator()(size_t i, size_t j, size_t k, size_t l, size_t m)
{
  return data_[GetIndex(i, j, k, l, m)];
}


template<class A>
typename Grid5D<A>::reference Grid5D<A>::operator()(size_t index)
{
  assert(index < GetN());

  return data_[index];
}


template<class A>
typename Grid5D<A>::const_reference Grid5D<A>::operator()(size_t i, size_t j, size_t k, size_t l, size_t m) const
{
  return data_[GetIndex(i, j, k, l, m)];
}


template<class A>
typename Grid5D<A>::const_reference Grid5D<A>::operator()(size_t index) const
{
  assert(index < GetN());

  return data_[index];
}


template<class A>
size_t Grid5D<A>::GetIndex(size_t i, size_t j, size_t k, size_t l, size_t m) const
{
  assert(i < GetNI() && j < GetNJ() && k < GetNK() && l < GetNL() && m < GetNM());

  return i + j*ni_ + k*ni_*nj_ + l*ni_*nj_*nk_ + m*ni_*nj_*nk_*nl_;
}

template<class A>
void Grid5D<A>::GetIJKLM(size_t index, size_t &i, size_t &j, size_t &k, size_t &l, size_t &m) const
{
  assert(index < GetN());

  i = index % ni_;
  j = (index-i)/ni_ % nj_;
  k = (index - j*ni_ - i)/ni_/nj_;
  l = (index - k*ni_*nj_)/ni_/nj_/nk_; //?
  m = (index - m*ni_*nj_*nk_)/ni_/nj_/nk_/nl_; //?
}

template<class A>
void Grid5D<A>::Swap(NRLib::Grid5D<A> &other)
{
  std::swap(ni_, other.ni_);
  std::swap(nj_, other.nj_);
  std::swap(nk_, other.nk_);
  std::swap(nl_, other.nl_);
  std::swap(nm_, other.nm_);
  data_.swap(other.data_);
}

}
#endif







