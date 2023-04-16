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

#include "cvfVector2.h"

namespace cvf {


// Forward declaration of templated matrix class
template<typename S>
class Matrix4;

template<typename S>
class Matrix3;


//==================================================================================================
//
// Vector class for 3D vectors
//
//==================================================================================================
template<typename S>
class Vector3
{
public:
    Vector3();
    Vector3(S x, S y, S z);
    Vector3(const Vector3& other);

    explicit Vector3(const Vector2<S>& other);
    Vector3(const Vector2<S>& other, S z);

    template<typename T>
    explicit Vector3(const T& other);

    inline Vector3&         operator=(const Vector3& rhs);

    inline bool             equals(const Vector3& other) const;
    inline bool             operator==(const Vector3& rhs) const;
    inline bool             operator!=(const Vector3& rhs) const;
    
    inline void             add(const Vector3& other);
    inline void             subtract(const Vector3& other);
    inline const Vector3    operator+(const Vector3& rhs) const;
    inline const Vector3    operator-(const Vector3& rhs) const;
    inline Vector3&         operator+=(const Vector3& rhs);
    inline Vector3&         operator-=(const Vector3& rhs);
    inline const Vector3    operator-() const;

    inline void             scale(S scalar);
    inline const Vector3    operator*(S scalar) const;
    inline const Vector3    operator/(S scalar) const;
    inline Vector3&         operator*=(S scalar);
    inline Vector3&         operator/=(S scalar);

    inline S                dot(const Vector3& other) const;
    inline S                operator*(const Vector3& rhs) const;    // Dot product

    inline void             cross(const Vector3& v1, const Vector3& v2);
    inline const Vector3    operator^(const Vector3& rhs) const;    // Cross product

    template<typename T> 
    void                    set(const T& other);
    inline void             set(S x, S y, S z);
    inline void             setZero();
    inline const S&         x() const       { return m_v[0]; }      ///< Get the X element of the vector
    inline const S&         y() const       { return m_v[1]; }      ///< Get the Y element of the vector
    inline const S&         z() const       { return m_v[2]; }      ///< Get the Z element of the vector
    inline S&               x()             { return m_v[0]; }      ///< Get a reference to the X element of the vector. E.g. x() = 1;
    inline S&               y()             { return m_v[1]; }      ///< Get a reference to the Y element of the vector. E.g. y() = 2;
    inline S&               z()             { return m_v[2]; }      ///< Get a reference to the Z element of the vector. E.g. z() = 3;
    inline const S&         operator[](int index) const;            // Get component 0,1,2. E.g. x = v[0];
    inline S&               operator[](int index);                  // Set component 0,1,2. E.g. v[0] = x;

    inline S*               ptr()           { return m_v; }         ///< Get a raw pointer to the internal c array of type S.
    inline const S*         ptr() const     { return m_v; }         ///< Get a const raw pointer to the internal c array of type S.

    inline bool     isZero() const;
    inline bool     isUndefined() const;

    bool            normalize();
    const Vector3   getNormalized(bool* normalizationOK = NULL) const;

    inline S        length() const;
    inline S        lengthSquared() const;
    bool            setLength(S newLength);

    inline S        pointDistance(const Vector3& otherPoint) const;
    inline S        pointDistanceSquared(const Vector3& otherPoint) const;

    void            transformPoint(const Matrix4<S>& m);    
    const Vector3   getTransformedPoint(const Matrix4<S>& m) const;

    void            transformVector(const Matrix4<S>& m);
    const Vector3   getTransformedVector(const Matrix4<S>& m) const;
    void            transformVector(const Matrix3<S>& m);
    const Vector3   getTransformedVector(const Matrix3<S>& m) const;

    bool            createOrthonormalBasis(int mapToAxis, Vector3<S>* uAxis, Vector3<S>* vAxis, Vector3<S>* wAxis) const;
    const Vector3   perpendicularVector(bool* perpendicularOK = NULL) const;

public:
    static const Vector3 X_AXIS;    ///< X axis vector <1, 0, 0>
    static const Vector3 Y_AXIS;    ///< Y axis vector <0, 1, 0>
    static const Vector3 Z_AXIS;    ///< Z axis vector <0, 0, 1>
    static const Vector3 ZERO;      ///< Null vector <0, 0, 0>
    static const Vector3 UNDEFINED; ///< Undefined vector

private:
    template<typename T> 
    friend const Vector3<T> operator*(T scalar, const Vector3<T>& rhs);

private:
    S m_v[3];
};


typedef Vector3<float>  Vec3f;  ///< A vector with float components
typedef Vector3<double> Vec3d;  ///< A vector with double components
typedef Vector3<int>    Vec3i;  ///< A vector with int components
typedef Vector3<uint>   Vec3ui;  ///< A vector with uint components
typedef Vector3<size_t> Vec3st;  ///< A vector with size_t components

}

#include "cvfVector3.inl"
