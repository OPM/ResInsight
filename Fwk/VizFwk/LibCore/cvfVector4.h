//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfVector3.h"

namespace cvf {


//==================================================================================================
//
// Vector class for 3D vectors
//
//==================================================================================================
template<typename S>
class Vector4
{
public:
    Vector4() {}
    Vector4(S x, S y, S z, S w);
    Vector4(const Vector4& other);

    Vector4(const Vector3<S>& other, S w);

    template<typename T>
    explicit Vector4(const T& other);

    inline Vector4&         operator=(const Vector4& rhs);

    inline bool             equals(const Vector4& other) const;
    inline bool             operator==(const Vector4& rhs) const;
    inline bool             operator!=(const Vector4& rhs) const;

    inline void             add(const Vector4& other);
    inline void             subtract(const Vector4& other);
    inline const Vector4    operator+(const Vector4& rhs) const;
    inline const Vector4    operator-(const Vector4& rhs) const;
    inline Vector4&         operator+=(const Vector4& rhs);
    inline Vector4&         operator-=(const Vector4& rhs);
    inline const Vector4    operator-() const;

    inline void             scale(S scalar);
    inline const Vector4    operator*(S scalar) const;
    inline const Vector4    operator/(S scalar) const;
    inline Vector4&         operator*=(S scalar);
    inline Vector4&         operator/=(S scalar);

    inline S                dot(const Vector4& other) const;
    inline S                operator*(const Vector4& rhs) const;    // Dot product

    template<typename T> 
    void                    set(const T& other);
    inline void             set(S x, S y, S z, S w);
    inline void             setZero();
    inline const S&         x() const       { return m_v[0]; }      ///< Get the X element of the vector
    inline const S&         y() const       { return m_v[1]; }      ///< Get the Y element of the vector
    inline const S&         z() const       { return m_v[2]; }      ///< Get the Z element of the vector
    inline const S&         w() const       { return m_v[3]; }      ///< Get the W element of the vector
    inline S&               x()             { return m_v[0]; }      ///< Get a reference to the X element of the vector. E.g. x() = 1;
    inline S&               y()             { return m_v[1]; }      ///< Get a reference to the Y element of the vector. E.g. y() = 2;
    inline S&               z()             { return m_v[2]; }      ///< Get a reference to the Z element of the vector. E.g. z() = 3;
    inline S&               w()             { return m_v[3]; }      ///< Get a reference to the W element of the vector. E.g. w() = 3;

    inline const S&         operator[](int index) const;            // Get component 0,1,2,3. E.g. x = v[0];
    inline S&               operator[](int index);                  // Set component 0,1,2,3. E.g. v[0] = x;

    inline S*               ptr()       { return m_v; }             ///< Get a raw pointer to the internal c array of type S.
    inline const S*         ptr() const { return m_v; }             ///< Get a const raw pointer to the internal c array of type S.

    inline bool     isZero() const;

    bool            normalize();
    const Vector4   getNormalized(bool* normalizationOK = NULL) const;

    inline S        length() const;
    inline S        lengthSquared() const;
    bool            setLength(S newLength);

public:
    static const Vector4 ZERO;      ///< Null vector <0, 0, 0>
    static const Vector4 UNDEFINED; ///< Undefined vector

private:
    S m_v[4];
};


typedef Vector4<float>  Vec4f;  ///< A vector with float components
typedef Vector4<double> Vec4d;  ///< A vector with double components
typedef Vector4<int>    Vec4i;  ///< A vector with int components

}

#include "cvfVector4.inl"
