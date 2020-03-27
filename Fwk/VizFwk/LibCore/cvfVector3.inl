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
/// \class cvf::Vector3
/// \ingroup Core
///
/// Templated vector class for a 3 component vector.
///
/// Three ready-to-use typedefs are defined:\n
///  - cvf::Vec3f (Vector3<float>)
///  - cvf::Vec3d (Vector3<double>)
///  - cvf::Vec3i (Vector3<int>)
/// 
//==================================================================================================

template<typename S> Vector3<S> const Vector3<S>::X_AXIS(1,0,0);    
template<typename S> Vector3<S> const Vector3<S>::Y_AXIS(0,1,0);
template<typename S> Vector3<S> const Vector3<S>::Z_AXIS(0,0,1);
template<typename S> Vector3<S> const Vector3<S>::ZERO(0,0,0);

//--------------------------------------------------------------------------------------------------
/// Default constructor.
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector3<S>::Vector3()
{
    m_v[0] = 0;
    m_v[1] = 0;
    m_v[2] = 0;
}


//--------------------------------------------------------------------------------------------------
/// Set the vector to <x,y,z>
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector3<S>::Vector3(S x, S y, S z) 
{
    m_v[0] = x;
    m_v[1] = y;
    m_v[2] = z;
}


//--------------------------------------------------------------------------------------------------
/// Set the vector to the same as other
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector3<S>::Vector3(const Vector3& other) 
{ 
    *this = other; 
}


//--------------------------------------------------------------------------------------------------
/// An explicit cast constructor to convert from one vector type to another.
//--------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
Vector3<S>::Vector3(const T& other)
{
    m_v[0] = static_cast<S>(other.x());
    m_v[1] = static_cast<S>(other.y());
    m_v[2] = static_cast<S>(other.z());
}


//--------------------------------------------------------------------------------------------------
/// Explicit constructor from 2D vector.
/// 
/// Will initialize x and y components from the 2D vector, z will be set to 0.
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector3<S>::Vector3(const Vector2<S>& other)
{
    m_v[0] = other.x();
    m_v[1] = other.y();
    m_v[2] = 0.0;
}


//--------------------------------------------------------------------------------------------------
/// Constructor 
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector3<S>::Vector3(const Vector2<S>& other, S z)
{
    m_v[0] = other.x();
    m_v[1] = other.y();
    m_v[2] = z;
}


//--------------------------------------------------------------------------------------------------
/// Assign the vector to the contents of other
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector3<S>& Vector3<S>::operator=(const Vector3& other)
{
    m_v[0] = other.m_v[0];
    m_v[1] = other.m_v[1];
    m_v[2] = other.m_v[2];

    return *this;
}



//--------------------------------------------------------------------------------------------------
/// Check if two vectors are equal. An exact match is required.
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Vector3<S>::equals(const Vector3& other) const
{
    return (*this == other);
}


//--------------------------------------------------------------------------------------------------
/// Check if two vectors are equal. An exact match is required.
//--------------------------------------------------------------------------------------------------
template<typename S>
inline bool Vector3<S>::operator==(const Vector3& rhs) const
{
    return (m_v[0] == rhs.m_v[0]) && (m_v[1] == rhs.m_v[1]) && (m_v[2] == rhs.m_v[2]);
}


//--------------------------------------------------------------------------------------------------
/// Check if two vectors are different. Returns true if not an exact match
//--------------------------------------------------------------------------------------------------
template<typename S>
inline bool Vector3<S>::operator!=(const Vector3& rhs) const
{
     return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Adds the vector \a other to this vector
//--------------------------------------------------------------------------------------------------
template<typename S>
void cvf::Vector3<S>::add(const Vector3& other)
{
    (*this) += other;
}


//--------------------------------------------------------------------------------------------------
/// Subtracts the vector \a other from this vector
//--------------------------------------------------------------------------------------------------
template<typename S>
void cvf::Vector3<S>::subtract(const Vector3& other)
{
    (*this) -= other;
}


//--------------------------------------------------------------------------------------------------
/// Returns the sum of this vector and the rhs vector
//--------------------------------------------------------------------------------------------------
template<typename S>
inline const Vector3<S> Vector3<S>::operator+(const Vector3& rhs) const
{
    return Vector3(m_v[0]+rhs.m_v[0], m_v[1]+rhs.m_v[1], m_v[2]+rhs.m_v[2]);
}


//--------------------------------------------------------------------------------------------------
/// Compute this-rhs and return the result.
//--------------------------------------------------------------------------------------------------
template<typename S>
inline const Vector3<S> Vector3<S>::operator-(const Vector3& rhs) const
{
    return Vector3(m_v[0] - rhs.m_v[0], m_v[1] - rhs.m_v[1], m_v[2] - rhs.m_v[2]);
}


//--------------------------------------------------------------------------------------------------
/// Scale this vector by the given scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
void Vector3<S>::scale(S scalar)
{
    (*this) *= scalar;
}


//--------------------------------------------------------------------------------------------------
/// Return this vector scaled by the given scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::operator*(S scalar) const
{
    return Vector3(m_v[0]*scalar, m_v[1]*scalar, m_v[2]*scalar);
}


//--------------------------------------------------------------------------------------------------
/// Return vector scaled by the given scalar
//--------------------------------------------------------------------------------------------------
template<typename T>
const Vector3<T> operator*(T scalar, const Vector3<T>& rhs)
{
    // Note that this is a friend function
    return Vector3<T>(rhs.m_v[0]*scalar, rhs.m_v[1]*scalar, rhs.m_v[2]*scalar);
}


//--------------------------------------------------------------------------------------------------
/// Return a vector where each component is the corresponding component in this divided by scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::operator/(S scalar) const
{
    return Vector3(m_v[0]/scalar, m_v[1]/scalar, m_v[2]/scalar);
}


//--------------------------------------------------------------------------------------------------
/// Return a vector which is the negation of this
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::operator-() const
{
    return Vector3(-m_v[0], -m_v[1], -m_v[2]);
}


//--------------------------------------------------------------------------------------------------
/// Add the given vector to this
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector3<S>& Vector3<S>::operator+=(const Vector3& v)
{ 
    m_v[0] += v.x(); 
    m_v[1] += v.y(); 
    m_v[2] += v.z(); 
    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Subtract the given vector from this
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector3<S>& Vector3<S>::operator-=(const Vector3& v)
{ 
    m_v[0] -= v.x(); 
    m_v[1] -= v.y(); 
    m_v[2] -= v.z(); 
    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Scale this with the given scalar. Each component is multiplied with the given value
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector3<S>& Vector3<S>::operator*=(S scalar)
{ 
    m_v[0] *= scalar; 
    m_v[1] *= scalar; 
    m_v[2] *= scalar; 
    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Divide this with the given scalar. Each component is divided by the given scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector3<S>& Vector3<S>::operator/=(S scalar)
{ 
    m_v[0] /= scalar; 
    m_v[1] /= scalar; 
    m_v[2] /= scalar; 
    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Get component 0,1,2. E.g. x = v[0];
//--------------------------------------------------------------------------------------------------
template<typename S>
inline const S& Vector3<S>::operator[](int index) const
{
    CVF_TIGHT_ASSERT(index >= 0);
    CVF_TIGHT_ASSERT(index < 3);

    return m_v[index];
}


//--------------------------------------------------------------------------------------------------
/// Set component 0,1,2. E.g. v[0] = x;
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S& Vector3<S>::operator[](int index)
{
    CVF_TIGHT_ASSERT(index >= 0);
    CVF_TIGHT_ASSERT(index < 3);

    return m_v[index];
}


//--------------------------------------------------------------------------------------------------
/// Compute the dot product of this and \a other
//--------------------------------------------------------------------------------------------------
template<typename S>
S Vector3<S>::dot(const Vector3& other) const
{
    return (*this)*other;
}


//--------------------------------------------------------------------------------------------------
/// Compute the dot product of this and rhs and return the result (scalar)
/// 
/// Formula:
/// \code
/// S = tx*rx + ty*ry + tz*rz
/// \endcode
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S Vector3<S>::operator*(const Vector3& rhs) const
{
    return m_v[0]*rhs.m_v[0] + m_v[1]*rhs.m_v[1] + m_v[2]*rhs.m_v[2];
}



//--------------------------------------------------------------------------------------------------
/// Sets this vector to the cross product of vectors \a v1 and \a v2
//--------------------------------------------------------------------------------------------------
template<typename S>
void cvf::Vector3<S>::cross(const Vector3& v1, const Vector3& v2)
{
    *this = v1 ^ v2;
}


//--------------------------------------------------------------------------------------------------
/// Compute the cross product of this and rhs and return the result (vector)
/// 
/// Formula:
/// \code
/// vec = <ty*rz - tz*ry, tz*rx - tx*rz, tx*ry - ty*rx>
/// \endcode
//--------------------------------------------------------------------------------------------------
template<typename S>
inline const Vector3<S> Vector3<S>::operator^(const Vector3& rhs) const
{
    return Vector3(m_v[1]*rhs.m_v[2] - m_v[2]*rhs.m_v[1], m_v[2]*rhs.m_v[0] - m_v[0]*rhs.m_v[2], m_v[0]*rhs.m_v[1] - m_v[1]*rhs.m_v[0]); 
}


//--------------------------------------------------------------------------------------------------
/// Set the vector from the other vector (of different type). Cast each component to convert it.
//--------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
void Vector3<S>::set(const T& other)
{
    m_v[0] = static_cast<S>(other.x());
    m_v[1] = static_cast<S>(other.y());
    m_v[2] = static_cast<S>(other.z());
}


//--------------------------------------------------------------------------------------------------
/// Get the length of the vector
/// 
/// Formula:
/// \code
/// len = sqrt(x*x + y*y + z*z)
/// \endcode
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S Vector3<S>::length() const
{
    return Math::sqrt(m_v[0]*m_v[0] + m_v[1]*m_v[1] + m_v[2]*m_v[2]);
}


//--------------------------------------------------------------------------------------------------
/// Get the squared length (L2) of the vector
/// 
/// Formula:
/// \code
/// len = x*x + y*y + z*z
/// \endcode
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S Vector3<S>::lengthSquared() const
{
    return m_v[0]*m_v[0] + m_v[1]*m_v[1] + m_v[2]*m_v[2];
}


//--------------------------------------------------------------------------------------------------
/// Set the length of the vector to \a newLength. 
/// 
/// \return  The method returns true if scaling was a success. Returns false if the vector was of 
///          zero length and \a newLength was different from 0.
/// 
/// This is the same as calling normalize() and then scaling with \a newLength.
/// If the current vector length is 0 or \a newLength is 0, the vector is set to all zeros.
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Vector3<S>::setLength(S newLength)
{
    CVF_ASSERT(newLength >= 0);

    S currLen = length();

    if (currLen > std::numeric_limits<S>::epsilon() && newLength > 0)
    {
        S scale = newLength/currLen;
        m_v[0] *= scale;
        m_v[1] *= scale;
        m_v[2] *= scale;

        return true;
    }
    else
    {
        setZero();

        return (newLength == 0) ? true : false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Get the distance between this point and the point specified in \a otherPoint
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S Vector3<S>::pointDistance(const Vector3& otherPoint) const
{
    return Math::sqrt(pointDistanceSquared(otherPoint));
}

//--------------------------------------------------------------------------------------------------
/// Get the squared distance between this point and the point specified in \a otherPoint
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S Vector3<S>::pointDistanceSquared(const Vector3& otherPoint) const
{
    S x = otherPoint.m_v[0] - m_v[0];
    S y = otherPoint.m_v[1] - m_v[1];
    S z = otherPoint.m_v[2] - m_v[2];

    return (x*x + y*y + z*z);
}


//--------------------------------------------------------------------------------------------------
/// Normalize the vector (make sure the length is 1.0).
/// 
/// \return  Returns true if normalization was possible. Returns false if length is zero or a NaN vector.
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Vector3<S>::normalize()
{
    S len = length();
    if (len > 0.0)
    {
        // Precompute 1/length and do multiplication instead of division
        S oneOverLen = (static_cast<S>(1.0)/len);
        m_v[0] *= oneOverLen;
        m_v[1] *= oneOverLen;
        m_v[2] *= oneOverLen;

        return true;
    }
    else
    {
        // Might be NaN, so set it to zero
        m_v[0] = 0;
        m_v[1] = 0;
        m_v[2] = 0;

        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns a normalized version of the current vector. The vector is unchanged.
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::getNormalized(bool* normalizationOK) const
{
    S len = length();
    if (len > 0.0)
    {
        if (normalizationOK) *normalizationOK = true;

        S oneOverLen = (static_cast<S>(1.0)/len);
        return Vector3<S>(m_v[0]*oneOverLen, m_v[1]*oneOverLen, m_v[2]*oneOverLen);
    }
    else
    {
        if (normalizationOK) *normalizationOK = false;
        return Vector3<S>::ZERO;
    }
}


//--------------------------------------------------------------------------------------------------
/// Set all components to 0
//--------------------------------------------------------------------------------------------------
template<typename S>
inline void Vector3<S>::setZero()
{
    m_v[0] = 0;
    m_v[1] = 0;
    m_v[2] = 0;
}


//--------------------------------------------------------------------------------------------------
/// Check if all components are zero (exact match)
//--------------------------------------------------------------------------------------------------
template<typename S>
inline bool Vector3<S>::isZero() const
{
    return (m_v[0] == 0) && (m_v[1] == 0) && (m_v[2] == 0);
}


//--------------------------------------------------------------------------------------------------
/// Check if vector is undefined
/// 
/// \return Returns true if any one of the components is undefined
//--------------------------------------------------------------------------------------------------
template<typename S>
inline bool Vector3<S>::isUndefined() const
{
    if (Math::isUndefined(m_v[0]) ||
        Math::isUndefined(m_v[1]) ||
        Math::isUndefined(m_v[2]))
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Set the components of the vector
//--------------------------------------------------------------------------------------------------
template<typename S>
inline void Vector3<S>::set(S x, S y, S z)
{
    m_v[0] = x;
    m_v[1] = y;
    m_v[2] = z;
}


//--------------------------------------------------------------------------------------------------
/// Transforms this vector as a point.
/// 
/// Transforms this vector as a point by multiplying it with the passed matrix.
/// This will both rotate and translate this.
/// Assumes the matrix m doesn't contain any perspective projection
//--------------------------------------------------------------------------------------------------
template<typename S>
void Vector3<S>::transformPoint(const Matrix4<S>& m)
{
    const S* pV = m.ptr();

    S valX = pV[0]*x() + pV[4]*y() + pV[8]*z()  + pV[12];
    S valY = pV[1]*x() + pV[5]*y() + pV[9]*z()  + pV[13];
    S valZ = pV[2]*x() + pV[6]*y() + pV[10]*z() + pV[14];

    x() = valX;
    y() = valY;
    z() = valZ;
}


//--------------------------------------------------------------------------------------------------
/// Return this vector transformed as a point (rotation and translation) with the given matrix.
/// \sa
///  transformPoint
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::getTransformedPoint(const Matrix4<S>& m) const
{
    Vector3<S> pt(*this);
    pt.transformPoint(m);
    return pt;
}


//--------------------------------------------------------------------------------------------------
/// Transforms this vector as a vector.
/// 
/// Transforms this vector as a vector by multiplying it with the passed matrix. 
/// This will only rotate the vector, but not consider the translation part of the matrix.
/// Assumes the matrix m doesn't contain any perspective projection.
//--------------------------------------------------------------------------------------------------
template<typename S>
void Vector3<S>::transformVector(const Matrix4<S>& m)
{
    const S* pV = m.ptr();

    S valX = pV[0]*x() + pV[4]*y() + pV[8]*z();
    S valY = pV[1]*x() + pV[5]*y() + pV[9]*z();
    S valZ = pV[2]*x() + pV[6]*y() + pV[10]*z();

    x() = valX;
    y() = valY;
    z() = valZ;
}


//--------------------------------------------------------------------------------------------------
/// Return this vector transformed as a vector (rotation only) with the given matrix.
/// \sa
///  transformVector
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::getTransformedVector(const Matrix4<S>& m) const
{
    Vector3<S> vec(*this);
    vec.transformVector(m);
    return vec;
}


//--------------------------------------------------------------------------------------------------
/// Transforms this vector.
/// 
/// Transforms this vector by multiplying it with the passed matrix. 
//--------------------------------------------------------------------------------------------------
template<typename S>
void Vector3<S>::transformVector(const Matrix3<S>& m)
{
    const S* pV = m.ptr();

    S valX = pV[0]*x() + pV[3]*y() + pV[6]*z();
    S valY = pV[1]*x() + pV[4]*y() + pV[7]*z();
    S valZ = pV[2]*x() + pV[5]*y() + pV[8]*z();

    x() = valX;
    y() = valY;
    z() = valZ;
}


//--------------------------------------------------------------------------------------------------
/// Return this vector transformed with the given matrix.
/// \sa
///  transformVector
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::getTransformedVector(const Matrix3<S>& m) const
{
    Vector3<S> vec(*this);
    vec.transformVector(m);
    return vec;
}


//----------------------------------------------------------------------------------------------------
/// Create an orthonormal basis from this vector
/// 
/// \param mapToAxis  Specifies which axis this vector should be mapped onto (0=u, 1=v, 2=w)
/// \param uAxis      Output u vector
/// \param vAxis      Output v vector
/// \param wAxis      Output w vector
/// 
/// Will create an orthonormal basis from this vector. The returned u, v and w axes will all be unit
/// length and will be perpendicular to each other.
/// The vector does not have to be normalized before calling this function.
//----------------------------------------------------------------------------------------------------
template<typename S>
bool Vector3<S>::createOrthonormalBasis(int mapToAxis, Vector3<S>* uAxis, Vector3<S>* vAxis, Vector3<S>* wAxis) const
{
    bool normalizedOK = false;
    Vector3<S> W = getNormalized(&normalizedOK);
    if (!normalizedOK)
    {
        return false;
    }

    Vector3<S> U(0, 0, 0);
    if (Math::abs(W.x()) >= Math::abs(W.y()))
    {
        // W.x or W.z component has largest magnitude
        S length = Math::sqrt(W.x()*W.x() + W.z()*W.z());
        U.x() = -W.z()/length;
        U.y() = 0.0;
        U.z() = +W.x()/length;
    }
    else
    {
        // W.y or W.z component has largest magnitude
        S length = Math::sqrt(W.y()*W.y() + W.z()*W.z());
        U.x() = 0.0;
        U.y() = +W.z()/length;
        U.z() = -W.y()/length;
    }

    // Get V as cross product of WxU
    // Could be optimized by taking into account the fact that either the y or x component of U will be 0
    Vector3<S> V = W^U;

    // Algorithm above maps our vector onto W axis
    // If another axis is requested, we need to shuffle the vectors
    if (mapToAxis == 0)
    {
        if (uAxis) *uAxis = W;
        if (vAxis) *vAxis = U;
        if (wAxis) *wAxis = V;
    }
    else if (mapToAxis == 1)
    {
        if (uAxis) *uAxis = V;
        if (vAxis) *vAxis = W;
        if (wAxis) *wAxis = U;
    }
    else
    {
        if (uAxis) *uAxis = U;
        if (vAxis) *vAxis = V;
        if (wAxis) *wAxis = W;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Return a unit length perpendicular vector
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector3<S> Vector3<S>::perpendicularVector(bool* perpendicularOK) const
{
    Vector3<S> perpendic(0, 0, 0);
    bool ok = createOrthonormalBasis(0, NULL, &perpendic, NULL);
    if (perpendicularOK)
    {
        *perpendicularOK = ok;
    }

    return perpendic;
}


}  // namespace cvf
