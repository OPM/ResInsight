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

#include "cvfMath.h"

#include <limits>

namespace cvf {


//==================================================================================================
//
// Vector class for 2D vectors
//
//==================================================================================================
template<typename S>
class Vector2
{
public:
    Vector2();
    Vector2(S x, S y);
    Vector2(const Vector2& other);

    template<typename T>
    explicit Vector2(const T& other);

    inline Vector2&         operator=(const Vector2& rhs);

    inline bool             equals(const Vector2& other) const;
    inline bool             operator==(const Vector2& rhs) const;
    inline bool             operator!=(const Vector2& rhs) const;
    
    inline void             add(const Vector2& other);
    inline void             subtract(const Vector2& other);
    inline const Vector2    operator+(const Vector2& rhs) const;
    inline const Vector2    operator-(const Vector2& rhs) const;
    inline Vector2&         operator+=(const Vector2& rhs);
    inline Vector2&         operator-=(const Vector2& rhs);
    inline const Vector2    operator-() const;

    inline void             scale(S scalar);
    inline const Vector2    operator*(S scalar) const;
    inline const Vector2    operator/(S scalar) const;
    inline Vector2&         operator*=(S scalar);
    inline Vector2&         operator/=(S scalar);

    inline S                dot(const Vector2& other) const;
    inline S                operator*(const Vector2& rhs) const;    // Dot product

    template<typename T> 
    void                    set(const T& other);
    inline void             set(S x, S y);
    inline void             setZero();
    inline const S&         x() const   { return m_v[0]; }          ///< Get the X element of the vector
    inline const S&         y() const   { return m_v[1]; }          ///< Get the Y element of the vector
    inline S&               x()         { return m_v[0]; }          ///< Get a reference to the X element of the vector. E.g. x() = 1;
    inline S&               y()         { return m_v[1]; }          ///< Get a reference to the Y element of the vector. E.g. y() = 2;

    inline const S&         operator[](int index) const;            // Get component 0 or 1. E.g. x = v[0];
    inline S&               operator[](int index);                  // Set component 0 or 1. E.g. v[0] = x;

    inline S*               ptr()       { return m_v; }             ///< Get a raw pointer to the internal c array of type S.
    inline const S*         ptr() const { return m_v; }             ///< Get a const raw pointer to the internal c array of type S.

    inline bool     isZero() const;
    inline bool     isUndefined() const;

    bool            normalize();
    const Vector2   getNormalized(bool* normalizationOK = nullptr) const;

    inline S        length() const;
    inline S        lengthSquared() const;
    bool            setLength(S newLength);

    const Vector2   perpendicularVector() const;

public:
    static const Vector2 X_AXIS;    ///< X axis vector <1, 0>
    static const Vector2 Y_AXIS;    ///< Y axis vector <0, 1>
    static const Vector2 ZERO;      ///< Null vector <0, 0>
    static const Vector2 UNDEFINED; ///< Undefined vector

private:
    template<typename T> 
    friend const Vector2<T> operator*(T scalar, const Vector2<T>& rhs);

private:
    S m_v[2];
};


typedef Vector2<float>  Vec2f;  ///< A vector with float components
typedef Vector2<double> Vec2d;  ///< A vector with double components
typedef Vector2<int>    Vec2i;  ///< A vector with int components
typedef Vector2<uint>   Vec2ui;  ///< A vector with uint components
typedef Vector2<ushort> Vec2us;  ///< A vector with ushort components

}

#include "cvfVector2.inl"
