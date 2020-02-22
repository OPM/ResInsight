/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cvfBase.h"
#include "cvfVector3.h"

namespace caf
{
template< typename S>
class Tensor3
{
    S m_tensor[6]; // SXX, SYY, SZZ, SXY, SYZ, SZX
public:
    Tensor3();
    Tensor3(S sxx, S syy, S szz, S sxy, S syz, S szx); 
    Tensor3(const Tensor3& other);
    template<typename T>
    explicit Tensor3(const Tensor3<T>& other);

    static Tensor3      invalid();

    inline Tensor3&     operator=(const Tensor3& rhs);
    inline Tensor3      operator+(const Tensor3& rhs) const;
    inline Tensor3      operator*(S scale) const;

    bool                equals(const Tensor3& mat) const;
    bool                operator==(const Tensor3& rhs) const;
    bool                operator!=(const Tensor3& rhs) const;

    enum TensorComponentEnum { SXX, SYY, SZZ, SXY, SYZ, SZX };
    inline S&           operator[](TensorComponentEnum comp);
    inline S            operator[](TensorComponentEnum comp) const;

    void                setFromInternalLayout(S* tensorData); 
    void                setFromAbaqusLayout(S* tensorData);

    cvf::Vec3f          calculatePrincipals(cvf::Vec3f principalDirections[3]) const;
    float               calculateVonMises() const;

    Tensor3             rotated(const cvf::Matrix3<S>& rotMx) const;
};

typedef Tensor3<float> Ten3f;
typedef Tensor3<double> Ten3d;

}

#include "cafTensor3.inl"
