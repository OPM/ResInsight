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



namespace cvf {


    
//==================================================================================================
///
/// \class cvf::Quat
/// \ingroup Core
///
/// Templated quaternion 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor, initializes all elements to 0
//--------------------------------------------------------------------------------------------------
template<typename S>
Quat<S>::Quat()
:   m_x(0), 
    m_y(0), 
    m_z(0), 
    m_w(0)
{
}


//--------------------------------------------------------------------------------------------------
/// Constructor with initialization
//--------------------------------------------------------------------------------------------------
template<typename S>
Quat<S>::Quat(S x, S y, S z, S w)
:   m_x(x), 
    m_y(y), 
    m_z(z), 
    m_w(w)
{
}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
template<typename S>
Quat<S>::Quat(const Quat& other)
:   m_x(other.m_x), 
    m_y(other.m_y), 
    m_z(other.m_z), 
    m_w(other.m_w)
{
}


//--------------------------------------------------------------------------------------------------
/// Conversion constructor for explicit conversion
//--------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
Quat<S>::Quat(const T& other)
{
    m_x = static_cast<S>(other.x());
    m_y = static_cast<S>(other.y());
    m_z = static_cast<S>(other.z());
    m_w = static_cast<S>(other.w());
}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
template<typename S>
Quat<S>& Quat<S>::operator=(const Quat& rhs)
{
    m_x = rhs.m_x;
    m_y = rhs.m_y;
    m_z = rhs.m_z;
    m_w = rhs.m_w;

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Quat<S>::operator==(const Quat& rhs) const
{
    if (m_x != rhs.m_x) return false;
    if (m_y != rhs.m_y) return false;
    if (m_z != rhs.m_z) return false;
    if (m_w != rhs.m_w) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Quat<S>::operator!=(const Quat& rhs) const
{
    if (m_x != rhs.m_x) return true;
    if (m_y != rhs.m_y) return true;
    if (m_z != rhs.m_z) return true;
    if (m_w != rhs.m_w) return true;

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
void Quat<S>::set(S x, S y, S z, S w)
{
    m_x = x;
    m_y = y;
    m_z = z;
    m_w = w;
}


//--------------------------------------------------------------------------------------------------
/// Normalize the quaternion
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Quat<S>::normalize()
{
    // Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0. 
    // If they don't add up to 1.0, dividing by their magnitude will re-normalize them.

    S len = Math::sqrt(m_x*m_x + m_y*m_y + m_z*m_z + m_w*m_w);

    if (len > 0)
    {
        // Pre-compute 1/length and do multiplication instead of division
        S oneOverLen = static_cast<S>(1.0/len);
        m_x *= oneOverLen;
        m_y *= oneOverLen;
        m_z *= oneOverLen;
        m_w *= oneOverLen;

        return true;
    }
    else
    {
        // Might be NaN, so set it to zero
        m_x = 0;
        m_y = 0;
        m_z = 0;
        m_w = 0;

        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Sets the quaternion from a rotation axis and an angle (in radians)
//--------------------------------------------------------------------------------------------------
template<typename S>
void Quat<S>::setFromAxisAngle(Vector3<S> rotationAxis, S angle)
{
    *this = Quat::fromAxisAngle(rotationAxis, angle);
}


//--------------------------------------------------------------------------------------------------
/// Extracts the rotation in this quaternion as an axis and an angle (in radians)
//--------------------------------------------------------------------------------------------------
template<typename S>
void Quat<S>::toAxisAngle(Vector3<S>* rotationAxis, S* angle) const
{
    // From matrix an quaternion FAQ

    CVF_ASSERT(rotationAxis);
    CVF_ASSERT(angle);

    // Need a normalized quaternion
    Quat nQ(*this);
    nQ.normalize();

    // Clamp to acos' input domain
    S cos_a = Math::clamp(nQ.m_w, static_cast<S>(-1), static_cast<S>(1));
    *angle = 2*Math::acos(cos_a);

    S sin_a = Math::sqrt(1 - cos_a*cos_a);
    if (Math::abs(sin_a) < 0.0005) sin_a = 1;

    rotationAxis->x() = nQ.m_x/sin_a;
    rotationAxis->y() = nQ.m_y/sin_a;
    rotationAxis->z() = nQ.m_z/sin_a;
}


//--------------------------------------------------------------------------------------------------
/// Constructs a rotation matrix from this quaternion
//--------------------------------------------------------------------------------------------------
template<typename S>
Matrix3<S> Quat<S>::toMatrix3() const
{
    // Based on Quaternions paper by Ken Shoemake
    // The current version works even if quaternion isn't normalized
    // If we assume that the quaternion is normalized, this function can be made simpler/quicker

    S Nq = m_x*m_x + m_y*m_y + m_z*m_z + m_w*m_w;
    S s = (Nq > 0) ? (2/Nq) : 0;

    S xs = m_x*s;         
    S ys = m_y*s;
    S zs = m_z*s;

    S wx = m_w*xs;
    S wy = m_w*ys;
    S wz = m_w*zs;
    S xx = m_x*xs;
    S xy = m_x*ys;
    S xz = m_x*zs;
    S yy = m_y*ys;
    S yz = m_y*zs;
    S zz = m_z*zs;


    // Default constructor initializes to identity
    Matrix3<S> m;
    m.setRowCol( 0, 0,  1 - (yy + zz)); 
    m.setRowCol( 0, 1,       xy - wz ); 
    m.setRowCol( 0, 2,       xz + wy ); 

    m.setRowCol( 1, 0,       xy + wz ); 
    m.setRowCol( 1, 1,  1 - (xx + zz)); 
    m.setRowCol( 1, 2,       yz - wx ); 
 
    m.setRowCol( 2, 0,       xz - wy );
    m.setRowCol( 2, 1,       yz + wx );
    m.setRowCol( 2, 2,  1 - (xx + yy));

    return m;
}


//--------------------------------------------------------------------------------------------------
/// Constructs a rotation matrix from this quaternion
//--------------------------------------------------------------------------------------------------
template<typename S>
Matrix4<S> Quat<S>::toMatrix4() const
{
    // Create via the Matrix3 version
    return Matrix4<S>(toMatrix3());
}


//--------------------------------------------------------------------------------------------------
/// Creates a quaternion from specified rotation axis and angle (in radians)
//--------------------------------------------------------------------------------------------------
template<typename S>
Quat<S> Quat<S>::fromAxisAngle(Vector3<S> rotationAxis, S angle) 
{
    // From matrix an quaternion FAQ

    rotationAxis.normalize();

    S halfAngle = static_cast<S>(angle/2.0);
    S sinHalfAngle = Math::sin(halfAngle);

    Quat q;
    q.m_x = rotationAxis.x()*sinHalfAngle;;
    q.m_y = rotationAxis.y()*sinHalfAngle;
    q.m_z = rotationAxis.z()*sinHalfAngle;
    q.m_w = Math::cos(halfAngle);

    return q;
}


//--------------------------------------------------------------------------------------------------
/// Creates a quaternion from the passed rotation matrix
//--------------------------------------------------------------------------------------------------
template<typename S>
Quat<S> Quat<S>::fromRotationMatrix(const Matrix4<S>& rotMat)
{
    const S m00 = rotMat.rowCol(0, 0);
    const S m01 = rotMat.rowCol(0, 1);
    const S m02 = rotMat.rowCol(0, 2);
    const S m10 = rotMat.rowCol(1, 0);
    const S m11 = rotMat.rowCol(1, 1);
    const S m12 = rotMat.rowCol(1, 2);
    const S m20 = rotMat.rowCol(2, 0);
    const S m21 = rotMat.rowCol(2, 1);
    const S m22 = rotMat.rowCol(2, 2);


    // If the trace of the matrix is greater than zero, then perform an "instant" calculation.
    S trace = m00 + m11 + m22;

    Quat q(0, 0, 0, 1);

    // Try and test against a very small value instead of 0 here
    // The FAQ suggests testing against ( T > 0.00000001 ) to avoid large distortions
    if (trace > 0.00000001)
    {
        S s = Math::sqrt(trace + 1)*2;

        q.m_x = (m21 - m12)/s;
        q.m_y = (m02 - m20)/s;
        q.m_z = (m10 - m01)/s;
        q.m_w = static_cast<S>(0.25*s);
    }
    else
    {
        // If the trace of the matrix is equal to zero then identify
        // which major diagonal element has the greatest value.
        int iDominantColumn = 0;
        if (m11 > m00) iDominantColumn = 1;
        if (m22 > m11) iDominantColumn = 2;

        if (iDominantColumn == 0)
        {
            S s = Math::sqrt(1 + m00 - m11 - m22)*2;
            if (s > 0)
            {
                q.m_x = static_cast<S>(0.25*s);
                q.m_y = (m10 + m01)/s;
                q.m_z = (m20 + m02)/s;
                q.m_w = (m21 - m12)/s;
            }
        }
        else if (iDominantColumn == 1)
        {
            S s = Math::sqrt(1 + m11 - m00 - m22)*2;
            if (s > 0)
            {
                q.m_x = (m10 + m01)/s;
                q.m_y = static_cast<S>(0.25*s);
                q.m_z = (m21 + m12)/s;
                q.m_w = (m02 - m20)/s;
            }
        }
        else 
        {
            S s = Math::sqrt(1 + m22 - m00 - m11)*2;
            if (s > 0)
            {
                q.m_x = (m20 + m02)/s;
                q.m_y = (m21 + m12)/s;
                q.m_z = static_cast<S>(0.25*s);
                q.m_w = (m10 - m01)/s;
            }
        }
    }

    return q;
}

}  // namespace cvf
