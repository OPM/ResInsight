// $Id: regularsurface.hpp 1196 2013-09-09 12:01:04Z perroe $

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
#ifndef NRLIB_REGULARSURFACE_HPP
#define NRLIB_REGULARSURFACE_HPP

#include <algorithm>
#include <cmath>
#include <iostream>

#include "surface.hpp"
#include "surfaceio.hpp"
#include "../grid/grid2d.hpp"
#include "../iotools/stringtools.hpp"

namespace NRLib {

/// Surface represented as a regular grid, where each of the grid cells
/// are modelled as bilinear surfaces.

template <class A>
class RegularSurface : public Grid2D<A>, public Surface<A> {
public:
  RegularSurface();

  RegularSurface(double x0, double y0, double lx, double ly, size_t nx, size_t ny,
                 const A& value = A());

  RegularSurface(double x0, double y0, double lx, double ly, Grid2D<A> grid);

  /// \brief Read surface file on given format.
  /// \param filename  File name.
  /// \param format    File format. If SURF_UNKNOWN we try to determine the format.
  /// \throws IOError         If we failed to open the file
  /// \throws FileFormatError If we can not determine the file format, or the contents
  ///                         of the file does not match the file format.
  RegularSurface(const std::string & filename,
                 SurfaceFileFormat   format = SURF_UNKNOWN);

  Surface<A>* Clone() const
  { return new RegularSurface<A>(*this); }

  /// Return z. Returns missing if we are outside the grid.
  /// Returns also values when not all grid cell corners are present,
  /// e.g. when (x,y) is just outside the grid.
  A GetZ(double x, double y) const;

  /// Return z. Returns missing if not all grid cell corners are present,
  /// e.g. when (x,y) is just outside the grid.
  A GetZInside(double x, double y) const;

  /// Checks if point is inside definition area for surface.
  bool IsInsideSurface(double x, double y) const;

  bool EnclosesRectangle(double x_min, double x_max,
                         double y_min, double y_max) const;

  /// Sets all values on the surface to a constant value.
  void Assign(A c) {
    typename Grid2D<A>::iterator i;
    for (i=this->begin();i<this->end();i++)
      *i = c;
  }

  void Add(A c) {
    typename Grid2D<A>::iterator i;
    for (i=this->begin();i<this->end();i++)
      if (*i != missing_val_)
        *i += c;
  }

  void Subtract(A c) {
    typename Grid2D<A>::iterator i;
    for (i=this->begin();i<this->end();i++)
      if (*i != missing_val_)
        *i -= c;
  }

  void Multiply(A c) {
    typename Grid2D<A>::iterator i;
    for (i=this->begin();i<this->end();i++)
      if (*i != missing_val_)
        *i *= c;
  }

  ///The following routines are for binary operations with non-conform grids.
  ///Missing areas will shrink.
  ///Also works for identical definitions without missing, but is inefficient.
  bool AddNonConform(const Surface<A> * s2);
  bool SubtractNonConform(const Surface<A> * s2);
  bool MultiplyNonConform(const Surface<A> * s2);
  bool DivideNonConform(const Surface<A> *s2);

  A Min() const;
  A MinNode(size_t &i, size_t &j) const;
  A Max() const;
  A MaxNode(size_t &i, size_t &j) const;
  A Avg() const;
  // n_nodes is number of nodes with non-missing values.
  A Avg(int& n_nodes) const;

  /// Returns vector with the four corners around the point (x,y).
  /// Fails if failOutside is false and either ni or nj is greater than 2^31.
  /// \param[out] corners The corners of the cell containing (x,y) or missing_val_ if
  ///                     the given corner is outside the grid, or does not have a valid
  ///                     value.
  inline void GetCorners(double x,
                         double y,
                         A      corners[4]) const;

  /// Returns the arithmetic average of the up to 8 (non-missing)-values from neighbouring grid cells.
  /// If no neighborud cells have non-missing values, missing is returned.
  bool CreateNeighbourAvg(size_t i, size_t j);

  /// Gets the index to left of x.
  /// The found index corresponds to the cell index.
  inline size_t FindI(double x) const;

  /// Gets the index to left of x.
  /// The found index corresponds to the cell index.
  inline size_t FindJ(double y) const;

  /// Gets the index for the nearest point up, and to the left of
  /// (x,y), the corner point with the lowest i and j values.
  /// The found index corresponds to the cell index.
  inline void FindIndex(double x, double y, size_t& i, size_t& j) const;

  // Find continuous index
  void FindContIndex(double x, double y, double& i, double& j) const;

  /// Gets the index for the nearest point up, and to the left of
  /// (x,y), the corner point with the lowest i and j values.
  /// May give values outside the grid.
  /// Fails if either ni or nj is greater than 2^31.
  /// The found index corresponds to the cell index.
  inline void FindGeneralIndex(double x, double y, int& i, int& j) const;

  /// Find nearest node.
  /// Similar to FindIndex, but finds index of nearest node instead of cell index.
  inline void FindNearestNodeIndex(double x, double y, size_t& i, size_t& j) const;

  double GetX(size_t i) const
  { return x_min_ + i*dx_; }

  double GetY(size_t j) const
  { return y_min_ + j*dy_; }

  void GetXY(size_t i, size_t j, double & x, double & y) const {
    x = x_min_ + i*dx_;
    y = y_min_ + j*dy_;
  }

  void GetXY(size_t index, double & x, double & y) const {
    size_t j = index / this->GetNI();
    size_t i = index - j*this->GetNI();
    GetXY(i, j, x, y);
  }

  void GetNode(size_t index, double& x, double& y, double& z) const {
    size_t i, j;
    this->GetIJ(index, i, j);
    GetNode(i, j, x, y, z);
  }

  void GetNode(size_t i, size_t j, double& x, double& y, double& z) const {
    GetXY(i, j, x, y);
    z = (*this)(i, j);
  }

  double GetXMin()    const { return x_min_; }
  double GetYMin()    const { return y_min_; }
  double GetXMax()    const { return x_min_ + lx_; }
  double GetYMax()    const { return y_min_ + ly_; }
  double GetDX()      const { return dx_; }
  double GetDY()      const { return dy_; }
  double GetLengthX() const { return lx_; }
  double GetLengthY() const { return ly_; }

  void SetDimensions(double x_min, double y_min,
                     double lx, double ly);

  /// Resize grid. Overrides Grid2D's resize.
  void Resize(size_t ni, size_t nj, const A& val = A());

  /// Check if grid value is missing
  bool IsMissing(A val) const {
    return val == missing_val_;
  }

  // Change names:
  // IsMissing(A)       ==>   EqualsMissingValue(A) ??
  // IsMissingAt(i,j)   ==>   GetMissingAt(i,j)
  // SetMissing(i,j)    ==>   SetMissingAt(i,j)

  /// Check if grid value is missing
  bool IsMissingAt(size_t i, size_t j) const {
    return (*this)(i, j) == missing_val_;
  }

  /// Set missing
  void SetMissing(size_t i, size_t j) {
    (*this)(i, j) = missing_val_;
  }

  A GetMissingValue() const { return missing_val_ ;}
  void SetMissingValue(A missing_val) { missing_val_ = missing_val; }

  const std::string& GetName() const                  { return name_; }
  void               SetName(const std::string& name) { name_ = name; }

  /// \brief Read surface file on given format.
  /// \param filename  File name.
  /// \param format    File format. If SURF_UNKNOWN we try to determine the format.
  /// \throws IOError         If we failed to open the file
  /// \throws FileFormatError If we can not determine the file format, or the contents
  ///                         of the file does not match the file format.
  void ReadFromFile(const std::string& filename,
                    SurfaceFileFormat  format = SURF_UNKNOWN);

  void WriteToFile(const std::string& filename,
                   SurfaceFileFormat  format = SURF_STORM_BINARY) const;

  void Swap(RegularSurface<A>& other);

private:
  double CellRelX(double x) const;
  double CellRelY(double y) const;

  double x_min_;
  double y_min_;
  double lx_;
  double ly_;
  double dx_;
  double dy_;
 // double rotation_;

  /// Fault name. Usually obtained from filename.
  std::string name_;

  /// Missing value
  A missing_val_;

  inline static A GetBilinearZ(A corners[4], double u, double v);
  /// Bilinear interpolation with missing corner.
  inline A GetBilinearZMissingCorners(A corners[4], double u, double v) const;
};

// ==================== TEMPLATE IMPLEMENTATIONS ====================

template <class A>
RegularSurface<A>::RegularSurface()
  : Grid2D<A>(0, 0),
    x_min_(0),
    y_min_(0),
    lx_(0),
    ly_(0),
    dx_(0),
    dy_(0),
    missing_val_(static_cast<A>(-999.0))
{}


template <class A>
RegularSurface<A>::RegularSurface(double x_min, double y_min,
                                  double lx, double ly,
                                  size_t nx, size_t ny,
                                  const A& value)
  : Grid2D<A>(nx, ny, value),
    x_min_(x_min),
    y_min_(y_min),
    lx_(lx),
    ly_(ly),
    missing_val_(static_cast<A>(-999.0))
{
  dx_ = (nx > 1) ? lx / (nx-1) : 1.0;
  dy_ = (ny > 1) ? ly / (ny-1) : 1.0;
}


template <class A>
RegularSurface<A>::RegularSurface(double x_min, double  y_min,
                                  double lx, double ly,
                                  Grid2D<A> grid)
  : Grid2D<A>(grid),
    x_min_(x_min),
    y_min_(y_min),
    lx_(lx),
    ly_(ly),
    missing_val_(static_cast<A>(-999.0))
{
  size_t ni = grid.GetNI();
  size_t nj = grid.GetNJ();
  dx_ = (ni > 1) ? lx / (ni-1) : 1.0;
  dy_ = (nj > 1) ? ly / (nj-1) : 1.0;
}


template <class A>
RegularSurface<A>::RegularSurface(const std::string& filename,
                                  SurfaceFileFormat  format)
  : lx_(0.0),
    ly_(0.0),
    missing_val_(static_cast<A>(-999.0))
{
  ReadFromFile(filename, format);
}


/// \todo Move to a more suitable location.
template <class A>
A RegularSurface<A>::GetBilinearZ(A corners[4], double u, double v)
{
  A a = corners[0];
  A b = corners[1] - a;
  A c = corners[2] - a;
  A d = corners[3] - a - b - c;

  return static_cast<A>(a + b*u + c*v + d*u*v);
}


template <class A>
A RegularSurface<A>::GetBilinearZMissingCorners(A corners[4], double u, double v) const
{
  double weights[4];

  weights[0] = (1.0-u) * (1.0-v);
  weights[1] = u * (1.0 - v);
  weights[2] = (1.0-u) * v;
  weights[3] = u * v;

  double sum = 0.0;
  double sum_weights = 0.0;

  for (size_t i = 0; i < 4; ++i) {
    if (!IsMissing(corners[i])) {
      sum += weights[i] * corners[i];
      sum_weights += weights[i];
    }
  }

  if (sum_weights == 0.0) {
    return GetMissingValue();
  }
  return static_cast<A>(sum / sum_weights);
}


template <class A>
A RegularSurface<A>::GetZ(double x, double y) const
{
  A corner[4];
  GetCorners(x, y, corner);

  int n_missing = 0;

  for (int pix = 0; pix < 4; pix++)
  {
    if (IsMissing(corner[pix]))
      n_missing++;
  }

  if (n_missing == 4) {
    return(missing_val_);
  }
  else {
    double x1 = CellRelX(x);
    double y1 = CellRelY(y);

    if (n_missing == 0) {
      return GetBilinearZ(corner, x1, y1);
    }
    else {
      return GetBilinearZMissingCorners(corner, x1, y1);
    }
  }
}


template <class A>
A RegularSurface<A>::GetZInside(double x, double y) const
{
  A corner[4];
  GetCorners(x, y, corner);

  bool missing = false;

  int pix = 0;
  while (pix < 4 && !missing) {
    if (IsMissing(corner[pix]))
      missing = true;
    pix++;
  }

  if (missing)
    return(missing_val_);
  else {
    double x1 = CellRelX(x);
    double y1 = CellRelY(y);
    return GetBilinearZ(corner, x1, y1);
  }
}


template <class A>
bool RegularSurface<A>::IsInsideSurface(double x, double y) const
{
  if (x < GetXMin() || x > GetXMax() || y < GetYMin() || y > GetYMax())
    return false;
  return true;
}


template <class A>
bool RegularSurface<A>::AddNonConform(const Surface<A> * s2)
{
  for (size_t i = 0; i < this->GetN(); i++) {
    if ((*this)(i) != missing_val_) {
      double x, y, value;
      GetXY(i, x, y);
      value = s2->GetZ(x, y);
      if (s2->IsMissing(value) == false)
        (*this)(i) += value;
      else
        (*this)(i) = missing_val_;
    }
  }
  return(true);
}


template <class A>
bool RegularSurface<A>::SubtractNonConform(const Surface<A> * s2)
{
  for (size_t i = 0; i < this->GetN(); i++) {
    if ((*this)(i) != missing_val_) {
      double x, y, value;
      GetXY(i, x, y);
      value = s2->GetZ(x, y);
      if (s2->IsMissing(value) == false)
        (*this)(i) -= value;
      else
        (*this)(i) = missing_val_;
    }
  }
  return(true);
}

template <class A>
bool RegularSurface<A>::MultiplyNonConform(const Surface<A> * s2)
{
  for (size_t i = 0; i < this->GetN(); i++) {
    if ((*this)(i) != missing_val_) {
      double x, y, value;
      GetXY(i, x, y);
      value = s2->GetZ(x, y);
      if (s2->IsMissing(value) == false)
        (*this)(i) *= value;
      else
        (*this)(i) = missing_val_;
    }
  }
  return(true);
}

template <class A>
bool RegularSurface<A>::DivideNonConform(const Surface<A> * s2)
{
  for (size_t i = 0; i < this->GetN(); i++) {
    if ((*this)(i) != missing_val_) {
      double x, y, value;
      GetXY(i, x, y);
      value = s2->GetZ(x, y);
      if (s2->IsMissing(value) == false)
        (*this)(i) /= value;
      else
        (*this)(i) = missing_val_;
    }
  }
  return(true);
}


template <class A>
A RegularSurface<A>::Min() const
{
  A minVal = (*this)(0);
  typename std::vector<A>::const_iterator i;
  for (i = this->begin(); i < this->end(); i++) {
    if ((IsMissing(minVal) || (*i) < minVal) && IsMissing(*i) == false)
      minVal = *i;
  }
  return minVal;
}

template <class A>
A RegularSurface<A>::MinNode(size_t &i, size_t &j) const
{
  A minVal = (*this)(0);
  typename std::vector<A>::const_iterator it;
  size_t min_index = 0;
  for (it = this->begin(); it < this->end(); it++) {
    if ((IsMissing(minVal) || (*it) < minVal) && IsMissing(*it) == false) {
      minVal = *it;
      min_index = static_cast<size_t> (it - this->begin());
    }
  }
  this->GetIJ(min_index, i, j);
  return minVal;
}

template <class A>
A RegularSurface<A>::Max() const
{
  A maxVal = (*this)(0);
  typename std::vector<A>::const_iterator i;
  for (i = this->begin(); i < this->end(); i++) {
    if ((IsMissing(maxVal) || (*i) > maxVal) && IsMissing(*i) == false)
      maxVal = *i;
  }
  return maxVal;
}

template <class A>
A RegularSurface<A>::MaxNode(size_t &i, size_t &j) const
{
  A maxVal = (*this)(0);
  typename std::vector<A>::const_iterator it;
  size_t max_index = 0;
  for (it = this->begin(); it < this->end(); it++) {
    if ((IsMissing(maxVal) || (*it) > maxVal) && IsMissing(*it) == false) {
      maxVal = *it;
      max_index = static_cast<size_t> (it - this->begin());
    }
  }
  this->GetIJ(max_index, i, j);
  return maxVal;
}

template <class A>
A RegularSurface<A>::Avg() const
{
  int n;
  return Avg(n);
}


template <class A>
A RegularSurface<A>::Avg(int& n_nodes) const
{
  A sum = static_cast<A>(0);
  int count = 0;
  typename std::vector<A>::const_iterator i;
  for (i = this->begin(); i < this->end(); i++) {
    if (!IsMissing(*i)) {
      sum += *i;
      count++;
    }
  }
  double avg;
  if (count > 0)
    avg = sum / count;
  else
    avg = missing_val_;

  n_nodes = count;
  return avg;
}


template <class A>
bool RegularSurface<A>::EnclosesRectangle(double x_min, double x_max,
                                          double y_min, double y_max) const
{
  if (x_min < GetXMin() || x_max > GetXMax() ||
      y_min < GetYMin() || y_max > GetYMax()) {
    return false;
  }

  return true;
}


template <class A>
size_t RegularSurface<A>::FindI(double x) const
{
  assert (x >= GetXMin() && x <= GetXMax());
  return static_cast<size_t>( (x-GetXMin()) / GetDX() );
}


template <class A>
size_t RegularSurface<A>::FindJ(double y) const
{
  assert (y >= GetYMin() && y <= GetYMax());
  return static_cast<size_t>( (y-GetYMin()) / GetDY() );
}


template <class A>
void RegularSurface<A>::FindIndex(double x, double y, size_t& i, size_t& j) const
{
  assert(x >= GetXMin() && x <= GetXMax() &&
         y >= GetYMin() && y <= GetYMax());

  i = static_cast<size_t>( (x-GetXMin()) / GetDX() );
  j = static_cast<size_t>( (y-GetYMin()) / GetDY() );
}


template <class A>
void RegularSurface<A>::FindContIndex(double x, double y, double& i, double& j) const
{
  assert(x >= GetXMin() && x <= GetXMax() &&
         y >= GetYMin() && y <= GetYMax());

  i = (x-GetXMin()) / GetDX() ;
  j = (y-GetYMin()) / GetDY() ;
}


template <class A>
void RegularSurface<A>::FindGeneralIndex(double x, double y, int& i, int& j) const
{
  i = static_cast<int>(std::floor( (x-GetXMin()) / GetDX() ));
  j = static_cast<int>(std::floor( (y-GetYMin()) / GetDY() ));
}


template <class A>
void RegularSurface<A>::FindNearestNodeIndex(double x, double y, size_t& i, size_t& j) const
{
  int tmp_i, tmp_j;
  int ni = static_cast<int>(this->GetNI());
  int nj = static_cast<int>(this->GetNJ());
  FindGeneralIndex(x + 0.5*dx_, y + 0.5*dy_, tmp_i, tmp_j);
  if (tmp_i < 0)
    tmp_i = 0;
  else if (tmp_i >= ni)
    tmp_i = ni - 1;

  if (tmp_j < 0)
    tmp_j = 0;
  else if (tmp_j >= nj)
    tmp_j = nj - 1;

  i = static_cast<size_t>(tmp_i);
  j = static_cast<size_t>(tmp_j);
}


template <class A>
void RegularSurface<A>::GetCorners(double x,
                                   double y,
                                   A      corners[4]) const
{
  int i, j;
  int ni = static_cast<int>(this->GetNI());
  int nj = static_cast<int>(this->GetNJ());
  FindGeneralIndex(x, y, i , j);

  if (i+1 >= 0 && i < ni && j+1 >= 0 && j < nj) {
    if (i >= 0 && j >= 0)
      corners[0] = (*this)(i, j);
    else
      corners[0] = missing_val_;
    if (i+1 < ni && j >= 0)
      corners[1] = (*this)(i+1, j);
    else
      corners[1] = missing_val_;
    if (i >= 0 && j+1 < nj)
      corners[2] = (*this)(i, j+1);
    else
      corners[2] = missing_val_;
    if (i+1 < ni && j+1 < nj)
      corners[3] = (*this)(i+1, j+1);
    else
      corners[3] = missing_val_;
  }
  else {
    corners[0] = missing_val_;
    corners[1] = missing_val_;
    corners[2] = missing_val_;
    corners[3] = missing_val_;
  }
}


template <class A>
bool RegularSurface<A>::CreateNeighbourAvg(size_t i, size_t j)
{
  double sum = 0.0;
  int n = 0;
  if (i > 0) {
    if (j > 0) {
      if (!IsMissingAt(i-1,j-1)) {
        sum += (*this)(i-1,j-1);
        n++;
      }
    }
    if (j < this->GetNJ() - 1) {
      if (!IsMissingAt(i-1,j+1)) {
        sum += (*this)(i-1,j+1);
        n++;
      }
    }
    if (!IsMissingAt(i-1,j)) {
      sum += (*this)(i-1,j);
      n++;
    }
  }
  if (i < this->GetNI() - 1) {
    if (j > 0) {
      if (!IsMissingAt(i+1,j-1)) {
        sum += (*this)(i+1,j-1);
        n++;
      }
    }
    if (j < this->GetNJ() - 1) {
      if (!IsMissingAt(i+1,j+1)) {
        sum += (*this)(i+1,j+1);
        n++;
      }
    }
    if (!IsMissingAt(i+1,j)) {
      sum += (*this)(i+1,j);
      n++;
    }
  }

  if (j > 0) {
    if (!IsMissingAt(i,j-1)) {
      sum += (*this)(i,j-1);
      n++;
    }
  }
  if (j < this->GetNJ() - 1) {
    if (!IsMissingAt(i,j+1)) {
      sum += (*this)(i,j+1);
      n++;
    }
  }

  double avg = missing_val_;
  bool non_missing = false;
  if (n > 0) {
    avg = sum / n;
    non_missing = true;
  }

  (*this)(i,j) = avg;
  return (non_missing);
}


template <class A>
void RegularSurface<A>::SetDimensions(double x_min, double y_min,
                                      double lx, double ly)
{
  x_min_ = x_min;
  y_min_ = y_min;
  lx_ = lx;
  ly_ = ly;
  size_t ni = Grid2D<A>::GetNI();
  size_t nj = Grid2D<A>::GetNJ();
  dx_ = (ni > 1) ? lx / (ni - 1) : 1.0;
  dy_ = (nj > 1) ? ly / (nj - 1) : 1.0;
}


template <class A>
void RegularSurface<A>::Resize(size_t nx, size_t ny, const A& value)
{
  Grid2D<A>::Resize(nx, ny, value);
  dx_ = (nx > 1) ? lx_ / (nx-1) : 1.0;
  dy_ = (ny > 1) ? ly_ / (ny-1) : 1.0;
}


template <class A>
void RegularSurface<A>::ReadFromFile(const std::string& filename,
                                     SurfaceFileFormat  format)
{
  if (format == SURF_UNKNOWN) {
    format = FindSurfaceFileType(filename);
    if (format == SURF_UNKNOWN) {
      throw FileFormatError("Failed to determine file format for surface file: " + filename);
    }
  }

  double angle = 0.0;

  switch (format) {
    case SURF_IRAP_CLASSIC_ASCII:
      ReadIrapClassicAsciiSurf(filename, *this, angle);
      break;
    case SURF_STORM_BINARY:
      ReadStormBinarySurf(filename, *this);
      break;
    case SURF_SGRI:
      ReadSgriSurf(filename, *this, angle);
      break;
    default:
      throw FileFormatError("Reading of file " + filename + " on format " + ToString(format)
                             + " as non-rotated grid is currently not supported.");
  }

  if (angle != 0.0) {
    throw FileFormatError("Error reading regular surface: " + filename
                          + ". Rotated grids are not supported.");
  }
}


template <class A>
void RegularSurface<A>::WriteToFile(const std::string& filename,
                                    SurfaceFileFormat  format) const
{
  switch (format) {
    case SURF_IRAP_CLASSIC_ASCII:
      WriteIrapClassicAsciiSurf(*this, 0.0, filename);
      break;
    case SURF_STORM_BINARY:
      WriteStormBinarySurf(*this, filename);
      break;
    default:
      throw FileFormatError("Writing of surface to file " + filename + " on format "
                            + ToString(format) + " is currently not supported.");
  }
}


template <class A>
double RegularSurface<A>::CellRelX(double x) const
{
  double i;
  return std::modf((x-GetXMin())/GetDX(), &i);
}


template <class A>
double RegularSurface<A>::CellRelY(double y) const
{
  double j;
  return std::modf((y-GetYMin())/GetDY(), &j);
}


template <class A>
void RegularSurface<A>::Swap(NRLib::RegularSurface<A> &other)
{
  Grid2D<A>::Swap(other);
  std::swap(x_min_, other.x_min_);
  std::swap(y_min_, other.y_min_);
  std::swap(lx_, other.lx_);
  std::swap(ly_, other.ly_);
  std::swap(dx_, other.dx_);
  std::swap(dy_, other.dy_);
  std::swap(name_, other.name_);
  std::swap(missing_val_, other.missing_val_);
}


} // namespace NRLib

#endif // NRLIB_REGULARSURFACE_HPP
