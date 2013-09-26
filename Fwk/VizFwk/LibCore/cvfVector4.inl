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
/// \class cvf::Vector4
/// \ingroup Core
///
/// Templated vector class for a 4 component vector.
///
/// Three ready-to-use typedefs are defined:\n
///  - cvf::Vec4f (Vector4<float>)
///  - cvf::Vec4d (Vector4<double>)
///  - cvf::Vec4i (Vector4<int>)
/// 
//==================================================================================================

template<typename S> Vector4<S> const Vector4<S>::ZERO(0,0,0,0);

//--------------------------------------------------------------------------------------------------
/// Set the vector to <x,y,z>
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector4<S>::Vector4(S x, S y, S z, S w) 
{
    m_v[0] = x;
    m_v[1] = y;
    m_v[2] = z;
    m_v[3] = w;
}


//--------------------------------------------------------------------------------------------------
/// Set the vector to the same as other
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector4<S>::Vector4(const Vector4& other) 
{ 
    *this = other; 
}


//--------------------------------------------------------------------------------------------------
/// An explicit cast constructor to convert from one vector type to another.
//--------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
Vector4<S>::Vector4(const T& other)
{
    m_v[0] = static_cast<S>(other.x());
    m_v[1] = static_cast<S>(other.y());
    m_v[2] = static_cast<S>(other.z());
    m_v[3] = static_cast<S>(other.w());
}


//--------------------------------------------------------------------------------------------------
/// Assign the vector to the contents of other
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector4<S>& Vector4<S>::operator=(const Vector4& other)
{
    m_v[0] = other.m_v[0];
    m_v[1] = other.m_v[1];
    m_v[2] = other.m_v[2];
    m_v[3] = other.m_v[3];

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
Vector4<S>::Vector4(const Vector3<S>& other, S w)
{
    m_v[0] = other.x();
    m_v[1] = other.y();
    m_v[2] = other.z();
    m_v[3] = w;
}



//--------------------------------------------------------------------------------------------------
/// Check if two vectors are equal. An exact match is required.
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Vector4<S>::equals(const Vector4& other) const
{
    return (*this == other);
}


//--------------------------------------------------------------------------------------------------
/// Check if two vectors are equal. An exact match is required.
//--------------------------------------------------------------------------------------------------
template<typename S>
inline bool Vector4<S>::operator==(const Vector4& rhs) const
{
    return (m_v[0] == rhs.m_v[0]) && (m_v[1] == rhs.m_v[1]) && (m_v[2] == rhs.m_v[2]) && (m_v[3] == rhs.m_v[3]);
}


//--------------------------------------------------------------------------------------------------
/// Check if two vectors are different. Returns true if not an exact match
//--------------------------------------------------------------------------------------------------
template<typename S>
inline bool Vector4<S>::operator!=(const Vector4& rhs) const
{
     return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Adds the vector \a other to this vector
//--------------------------------------------------------------------------------------------------
template<typename S>
void cvf::Vector4<S>::add(const Vector4& other)
{
    (*this) += other;
}


//--------------------------------------------------------------------------------------------------
/// Subtracts the vector \a other from this vector
//--------------------------------------------------------------------------------------------------
template<typename S>
void cvf::Vector4<S>::subtract(const Vector4& other)
{
    (*this) -= other;
}


//--------------------------------------------------------------------------------------------------
/// Returns the sum of this vector and the rhs vector
//--------------------------------------------------------------------------------------------------
template<typename S>
inline const Vector4<S> Vector4<S>::operator+(const Vector4& rhs) const
{
    return Vector4(m_v[0] + rhs.m_v[0], m_v[1] + rhs.m_v[1], m_v[2] + rhs.m_v[2], m_v[3] + rhs.m_v[3]);
}


//--------------------------------------------------------------------------------------------------
/// Compute this-rhs and return the result.
//--------------------------------------------------------------------------------------------------
template<typename S>
inline const Vector4<S> Vector4<S>::operator-(const Vector4& rhs) const
{
    return Vector4(m_v[0] - rhs.m_v[0], m_v[1] - rhs.m_v[1], m_v[2] - rhs.m_v[2], m_v[3] - rhs.m_v[3]);
}



//--------------------------------------------------------------------------------------------------
/// Scale this vector by the given scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
void Vector4<S>::scale(S scalar)
{
    (*this) *= scalar;
}


//--------------------------------------------------------------------------------------------------
/// Return this vector scaled by the given scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector4<S> Vector4<S>::operator*(S scalar) const
{
    return Vector4(m_v[0]*scalar, m_v[1]*scalar, m_v[2]*scalar, m_v[3]*scalar);
}


//--------------------------------------------------------------------------------------------------
/// Return a vector where each component is the corresponding component in this divided by scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector4<S> Vector4<S>::operator/(S scalar) const
{
    return Vector4(m_v[0]/scalar, m_v[1]/scalar, m_v[2]/scalar, m_v[3]/scalar);
}


//--------------------------------------------------------------------------------------------------
/// Return a vector which is the negation of this
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector4<S> Vector4<S>::operator-() const
{
    return Vector4(-m_v[0], -m_v[1], -m_v[2], -m_v[3]);
}


//--------------------------------------------------------------------------------------------------
/// Add the given vector to this
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector4<S>& Vector4<S>::operator+=(const Vector4& v)
{ 
    m_v[0] += v.x(); 
    m_v[1] += v.y(); 
    m_v[2] += v.z(); 
    m_v[3] += v.w(); 

    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Subtract the given vector from this
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector4<S>& Vector4<S>::operator-=(const Vector4& v)
{ 
    m_v[0] -= v.x(); 
    m_v[1] -= v.y(); 
    m_v[2] -= v.z(); 
    m_v[3] -= v.w(); 

    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Scale this with the given scalar. Each component is multiplied with the given value
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector4<S>& Vector4<S>::operator*=(S scalar)
{ 
    m_v[0] *= scalar; 
    m_v[1] *= scalar; 
    m_v[2] *= scalar; 
    m_v[3] *= scalar; 

    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Divide this with the given scalar. Each component is divided by the given scalar
//--------------------------------------------------------------------------------------------------
template<typename S>
inline Vector4<S>& Vector4<S>::operator/=(S scalar)
{ 
    m_v[0] /= scalar; 
    m_v[1] /= scalar; 
    m_v[2] /= scalar; 
    m_v[3] /= scalar; 

    return *this; 
}


//--------------------------------------------------------------------------------------------------
/// Get component 0,1,2,3. E.g. x = v[0];
//--------------------------------------------------------------------------------------------------
template<typename S>
inline const S& Vector4<S>::operator[](int index) const
{
    CVF_TIGHT_ASSERT(index >= 0);
    CVF_TIGHT_ASSERT(index < 4);

    return m_v[index];
}


//--------------------------------------------------------------------------------------------------
/// Set component 0,1,2,3. E.g. v[0] = x;
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S& Vector4<S>::operator[](int index)
{
    CVF_TIGHT_ASSERT(index >= 0);
    CVF_TIGHT_ASSERT(index < 4);

    return m_v[index];
}


//--------------------------------------------------------------------------------------------------
/// Compute the dot product of this and \a other
//--------------------------------------------------------------------------------------------------
template<typename S>
S Vector4<S>::dot(const Vector4& other) const
{
    return (*this)*other;
}


//--------------------------------------------------------------------------------------------------
/// Compute the dot product of this and rhs and return the result (scalar)
/// 
/// Formula:
/// \code
/// S = tx*rx + ty*ry + tz*rz + tw*rw
/// \endcode
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S Vector4<S>::operator*(const Vector4& rhs) const
{
    return m_v[0]*rhs.m_v[0] + m_v[1]*rhs.m_v[1] + m_v[2]*rhs.m_v[2] + m_v[3]*rhs.m_v[3];
}


//--------------------------------------------------------------------------------------------------
/// Set the vector from the other vector (of different type). Cast each component to convert it.
//--------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
void Vector4<S>::set(const T& other)
{
    m_v[0] = static_cast<S>(other.x());
    m_v[1] = static_cast<S>(other.y());
    m_v[2] = static_cast<S>(other.z());
    m_v[3] = static_cast<S>(other.w());
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
inline S Vector4<S>::length() const
{
    return Math::sqrt(m_v[0]*m_v[0] + m_v[1]*m_v[1] + m_v[2]*m_v[2] + m_v[3]*m_v[3]);
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
inline S Vector4<S>::lengthSquared() const
{
    return m_v[0]*m_v[0] + m_v[1]*m_v[1] + m_v[2]*m_v[2] + m_v[3]*m_v[3];
}


//--------------------------------------------------------------------------------------------------
/// Set the length of the vector to \a newLength. 
/// 
/// \sa Vector3::setLength() 
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Vector4<S>::setLength(S newLength)
{
    CVF_ASSERT(newLength >= 0);

    S currLen = length();

    if (currLen > std::numeric_limits<S>::epsilon() && newLength > 0)
    {
        S scale = newLength/currLen;
        m_v[0] *= scale;
        m_v[1] *= scale;
        m_v[2] *= scale;
        m_v[3] *= scale;

        return true;
    }
    else
    {
        setZero();

        return (newLength == 0) ? true : false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Normalize the vector (make sure the length is 1.0).
/// Returns true if normalization was possible. Returns false if length is zero or a NaN vector.
//--------------------------------------------------------------------------------------------------
template<typename S>
bool Vector4<S>::normalize()
{
    S len = length();
    if (len > 0.0)
    {
        // Precompute 1/length and do multiplication instead of division
        S oneOverLen = (static_cast<S>(1.0)/len);
        m_v[0] *= oneOverLen;
        m_v[1] *= oneOverLen;
        m_v[2] *= oneOverLen;
        m_v[3] *= oneOverLen;

        return true;
    }
    else
    {
        // Might be NaN, so set it to zero
        m_v[0] = 0.0f;
        m_v[1] = 0.0f;
        m_v[2] = 0.0f;
        m_v[3] = 0.0f;

        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns a normalized version of the current vector. The vector is unchanged.
//--------------------------------------------------------------------------------------------------
template<typename S>
const Vector4<S> Vector4<S>::getNormalized(bool* normalizationOK) const
{
    S len = length();
    if (len > 0.0)
    {
        if (normalizationOK) *normalizationOK = true;

        S oneOverLen = (static_cast<S>(1.0)/len);
        return Vector4<S>(m_v[0]*oneOverLen, m_v[1]*oneOverLen, m_v[2]*oneOverLen, m_v[3]*oneOverLen);
    }
    else
    {
        if (normalizationOK) *normalizationOK = false;
        return Vector4<S>::ZERO;
    }
}


//--------------------------------------------------------------------------------------------------
/// Set all components to 0
//--------------------------------------------------------------------------------------------------
template<typename S>
inline void Vector4<S>::setZero()
{
    m_v[0] = 0;
    m_v[1] = 0;
    m_v[2] = 0;
    m_v[3] = 0;
}


//--------------------------------------------------------------------------------------------------
/// Check if all components are zero (exact match)
//--------------------------------------------------------------------------------------------------
template<typename S>
inline bool Vector4<S>::isZero() const
{
    return (m_v[0] == 0) && (m_v[1] == 0) && (m_v[2] == 0) && (m_v[2] == 0);
}


//--------------------------------------------------------------------------------------------------
/// Set the components of the vector
//--------------------------------------------------------------------------------------------------
template<typename S>
inline void Vector4<S>::set(S x, S y, S z, S w)
{
    m_v[0] = x;
    m_v[1] = y;
    m_v[2] = z;
    m_v[3] = w;
}

}  // namespace cvf
