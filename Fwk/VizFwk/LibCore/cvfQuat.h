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
// This class implements a quaternion 
//
//==================================================================================================
template<typename S>
class Quat
{
public:
    Quat();
    Quat(S x, S y, S z, S w);
    Quat(const Quat& other);

    template<typename T>
    explicit Quat(const T& other);

    Quat&   operator=(const Quat& rhs);
    bool    operator==(const Quat& rhs) const;
    bool    operator!=(const Quat& rhs) const;

    const S&    x() const   { return m_x; }     ///< Get the X element of the quaternion
    const S&    y() const   { return m_y; }     ///< Get the Y element of the quaternion
    const S&    z() const   { return m_z; }     ///< Get the Z element of the quaternion
    const S&    w() const   { return m_w; }     ///< Get the W element of the quaternion
    S&          x()         { return m_x; }     ///< Get a reference to the X element of the quaternion. E.g. x() = 2;
    S&          y()         { return m_y; }     ///< Get a reference to the Y element of the quaternion. E.g. y() = 2;
    S&          z()         { return m_z; }     ///< Get a reference to the Z element of the quaternion. E.g. z() = 2;
    S&          w()         { return m_w; }     ///< Get a reference to the W element of the quaternion. E.g. w() = 2;

    void        set(S x, S y, S z, S w);

    bool        normalize();

    // The setFromAxisAngle() function is left as a placeholder to illustrate naming of functions
    void        setFromAxisAngle(Vector3<S> rotationAxis, S angle);
	void        toAxisAngle(Vector3<S>* rotationAxis, S* angle) const;

    Matrix3<S>  toMatrix3() const;
    Matrix4<S>  toMatrix4() const;

    static Quat fromAxisAngle(Vector3<S> rotationAxis, S angle);
    static Quat fromRotationMatrix(const Matrix4<S>& rotMat);

private:
    S   m_x;
    S   m_y;
    S   m_z;
    S   m_w;
};

typedef Quat<float>  Quatf;
typedef Quat<double> Quatd;

}

#include "cvfQuat.inl"
