// $Id: surface.hpp 882 2011-09-23 13:10:16Z perroe $

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

#ifndef NRLIB_SURFACE_HPP
#define NRLIB_SURFACE_HPP

#include <limits>

namespace NRLib {
  template <class A>
  class Surface {
  public:
    virtual ~Surface();

    /// \brief Generate a copy of the underlying object.
    virtual Surface * Clone() const = 0;

    virtual A         GetZ(double x, double y) const = 0;

    virtual bool      EnclosesRectangle(double x_min, double x_max,
                                        double y_min, double y_max) const = 0;

    virtual bool      IsMissing(A)          const { return(false) ;}

    virtual bool      IsInsideSurface(double /*x*/, double /*y*/) const = 0;

    virtual void      Add(A c) = 0;

    virtual void      Multiply(A c) = 0;

    virtual A         Min() const = 0;
    virtual A         Max() const = 0;

    virtual double    GetXMin() const = 0;
    virtual double    GetYMin() const = 0;
    virtual double    GetXMax() const = 0;
    virtual double    GetYMax() const = 0;

  };

  template <class A>
  class ConstantSurface : public Surface<A> {
  public:
    ConstantSurface(A z);

    Surface<A>* Clone() const
    { return new ConstantSurface(*this); }

    A GetZ() const {
      return z_;
    }

    bool IsInsideSurface(double /*x*/, double /*y*/) const {return(true);}
    A GetZ(double /*x*/, double /*y*/) const
    { return z_; }

    bool EnclosesRectangle(double /*x_min*/, double /*x_max*/,
                           double /*y_min*/, double /*y_max*/) const
    { return true; }

    void Add(A c) {
      z_ += c;
    }

    void Multiply(A c) {
      z_ *= c;
    }

    A Min() const {return(z_);}
    A Max() const {return(z_);}

    double GetXMin() const
    {
      if ( std::numeric_limits<double>::has_infinity )
        return -std::numeric_limits<double>::infinity();
      else
        return -std::numeric_limits<double>::max();
    }

    double GetYMin() const
    {
      if ( std::numeric_limits<double>::has_infinity )
        return -std::numeric_limits<double>::infinity();
      else
        return -std::numeric_limits<double>::max();
    }

    double GetXMax() const
    {
      if ( std::numeric_limits<double>::has_infinity )
        return(std::numeric_limits<double>::infinity());
      else
        return std::numeric_limits<double>::max();
    }

    double GetYMax() const
    {
      if ( std::numeric_limits<double>::has_infinity )
        return(std::numeric_limits<double>::infinity());
      else
        return std::numeric_limits<double>::max();
    }

  private:
    A z_;
  };

  template <class A>
  Surface<A>::~Surface()
  {}

  template <class A>
  ConstantSurface<A>::ConstantSurface(A z)
    : z_(z)
  {}
} // namespace NRLib

#endif // NRLIB_SURFACE_HPP
