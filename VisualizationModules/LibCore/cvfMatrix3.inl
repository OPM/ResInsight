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
/// \class cvf::Matrix3
/// \ingroup Core
///
/// Matrices are stored internally as a one dimensional array for performance reasons. 
/// 
/// The internal indices into the 1D array are as follows:
/// 
/// <PRE>
///   | m00  m01  m02 |     | 0  3   6 | 
///   | m10  m11  m12 |     | 1  4   7 | 
///   | m20  m21  m22 |     | 2  5  18 | 
/// </PRE>
///
/// See description of Matrix4 for more details on storage
///
//==================================================================================================

template<typename S> Matrix3<S> const Matrix3<S>::IDENTITY;
template<typename S> Matrix3<S> const Matrix3<S>::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0);

//----------------------------------------------------------------------------------------------------
/// Default constructor. Initializes matrix to identity 
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix3<S>::Matrix3()
{
    setIdentity();
}


//----------------------------------------------------------------------------------------------------
/// Copy constructor
//----------------------------------------------------------------------------------------------------
template <typename S>
inline Matrix3<S>::Matrix3(const Matrix3& other)
{
    System::memcpy(m_v, sizeof(m_v), other.m_v, sizeof(other.m_v));
}


//----------------------------------------------------------------------------------------------------
/// Constructor with explicit initialization of all matrix elements.
/// 
/// The value of the parameter \a mrc will be placed in row r and column c of the matrix, eg m12
/// goes into row 1, column 2.
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix3<S>::Matrix3(S m00, S m01, S m02, S m10, S m11, S m12, S m20, S m21, S m22)
{
    m_v[e00] = m00;
    m_v[e10] = m10;
    m_v[e20] = m20;
    m_v[e01] = m01;
    m_v[e11] = m11;
    m_v[e21] = m21;
    m_v[e02] = m02;
    m_v[e12] = m12;
    m_v[e22] = m22;
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
Matrix3<S>::Matrix3(const Matrix3<T>& other)
{
    m_v[e00] = static_cast<S>(other.rowCol(0, 0));
    m_v[e01] = static_cast<S>(other.rowCol(0, 1));
    m_v[e02] = static_cast<S>(other.rowCol(0, 2));
    m_v[e10] = static_cast<S>(other.rowCol(1, 0));
    m_v[e11] = static_cast<S>(other.rowCol(1, 1));
    m_v[e12] = static_cast<S>(other.rowCol(1, 2));
    m_v[e20] = static_cast<S>(other.rowCol(2, 0));
    m_v[e21] = static_cast<S>(other.rowCol(2, 1));
    m_v[e22] = static_cast<S>(other.rowCol(2, 2));
}


//----------------------------------------------------------------------------------------------------
/// Assignment operator
//----------------------------------------------------------------------------------------------------
template <typename S>
inline Matrix3<S>& Matrix3<S>::operator=(const Matrix3& obj)
{
    System::memcpy(m_v, sizeof(m_v), obj.m_v, sizeof(obj.m_v));
    return *this;
}


//----------------------------------------------------------------------------------------------------
/// Check if matrices are equal using exact comparisons.
//----------------------------------------------------------------------------------------------------
template<typename S>
bool Matrix3<S>::equals(const Matrix3& mat) const
{
    for (int i = 0; i < 9; i++)
    {
        if (m_v[i] != mat.m_v[i]) return false;
    }

    return true;
}


//----------------------------------------------------------------------------------------------------
/// Comparison operator. Checks for equality using exact comparisons.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix3<S>::operator==(const Matrix3& rhs) const
{
    return this->equals(rhs);
}


//----------------------------------------------------------------------------------------------------
/// Comparison operator. Checks for not equal using exact comparisons.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix3<S>::operator!=(const Matrix3& rhs) const
{
    int i;
    for (i = 0; i < 9; i++)
    {
        if (m_v[i] != rhs.m_v[i]) return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Multiplies this matrix M with the matrix \a mat, M = M*mat
//--------------------------------------------------------------------------------------------------
template<typename S>
void Matrix3<S>::multiply(const Matrix3& mat)
{
    *this = (*this)*mat;
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
const Matrix3<S> Matrix3<S>::operator*(const Matrix3& rhs) const
{
    Matrix3 m;
    
    int r;
    for (r = 0; r < 3; r++)
    {
        int c;
        for (c = 0; c < 3; c++)
        {
            S val = 0;

            int a;
            for (a = 0; a < 3; a++)
            {
                val += rowCol(r, a)*rhs.rowCol(a, c);
            }

            m.setRowCol(r, c, val);
        }
    }

    return m;
}


//----------------------------------------------------------------------------------------------------
/// Sets this matrix to identity
//----------------------------------------------------------------------------------------------------
template <typename S>
void Matrix3<S>::setIdentity()
{
    m_v[0] = 1;
    m_v[1] = 0;    
    m_v[2] = 0;

    m_v[3] = 0;
    m_v[4] = 1;
    m_v[5] = 0;

    m_v[6] = 0;
    m_v[7] = 0;
    m_v[8] = 1;
}


//----------------------------------------------------------------------------------------------------
/// Check if this matrix is identity using exact comparisons
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix3<S>::isIdentity() const
{
    if (m_v[0] != 1) return false;
    if (m_v[1] != 0) return false;
    if (m_v[2] != 0) return false;

    if (m_v[3] != 0) return false;
    if (m_v[4] != 1) return false;
    if (m_v[5] != 0) return false;

    if (m_v[6] != 0) return false;
    if (m_v[7] != 0) return false;
    if (m_v[8] != 1) return false;

    return true;
}


//----------------------------------------------------------------------------------------------------
/// Set all matrix elements to 0
//----------------------------------------------------------------------------------------------------
template <typename S>
void Matrix3<S>::setZero()
{
    m_v[0]  = 0;
    m_v[1]  = 0;    
    m_v[2]  = 0;
    m_v[3]  = 0;
    m_v[4]  = 0;
    m_v[5]  = 0;
    m_v[6]  = 0;
    m_v[7]  = 0;
    m_v[8]  = 0;
}


//----------------------------------------------------------------------------------------------------
/// Returns true if all elements of the matrix are 0. Uses exact comparison.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix3<S>::isZero() const
{
    if (m_v[0] != 0) return false;
    if (m_v[1] != 0) return false;
    if (m_v[2] != 0) return false;

    if (m_v[3] != 0) return false;
    if (m_v[4] != 0) return false;
    if (m_v[5] != 0) return false;

    if (m_v[6] != 0) return false;
    if (m_v[7] != 0) return false;
    if (m_v[8] != 0) return false;

    return true;
}


//----------------------------------------------------------------------------------------------------
/// Set the matrix element at the specified row and column
//----------------------------------------------------------------------------------------------------
template <typename S>
inline void Matrix3<S>::setRowCol(int row, int col, S value)
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 3);
    CVF_TIGHT_ASSERT(col >= 0 && col < 3);

    m_v[3*col + row] = value;
}


//----------------------------------------------------------------------------------------------------
/// Get the matrix element at the specified row and column
//----------------------------------------------------------------------------------------------------
template <typename S>
inline S Matrix3<S>::rowCol(int row, int col) const
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 3);
    CVF_TIGHT_ASSERT(col >= 0 && col < 3);

    return m_v[3*col + row];
}

//--------------------------------------------------------------------------------------------------
/// Get modifiable reference to the the matrix element at the specified row and column
//--------------------------------------------------------------------------------------------------
template <typename S>
inline S& Matrix3<S>::operator()(int row, int col)
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 3);
    CVF_TIGHT_ASSERT(col >= 0 && col < 3);

    return m_v[3*col + row];
}


//--------------------------------------------------------------------------------------------------
/// Get the matrix element at the specified row and column
//--------------------------------------------------------------------------------------------------
template <typename S>
inline S Matrix3<S>::operator()(int row, int col) const
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 3);
    CVF_TIGHT_ASSERT(col >= 0 && col < 3);

    return m_v[3*col + row];
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix3<S>::invert()
{
    // Use double internally here
    double m00 = static_cast<double>(m_v[e00]);
    double m10 = static_cast<double>(m_v[e10]);
    double m20 = static_cast<double>(m_v[e20]);
    double m01 = static_cast<double>(m_v[e01]);
    double m11 = static_cast<double>(m_v[e11]);
    double m21 = static_cast<double>(m_v[e21]);
    double m02 = static_cast<double>(m_v[e02]);
    double m12 = static_cast<double>(m_v[e12]);
    double m22 = static_cast<double>(m_v[e22]);

    double det = m00*m11*m22 + m01*m12*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 - m02*m11*m20;

    if (Math::abs(det) > std::numeric_limits<double>::epsilon())
    {
        double invDet = 1/det;

        m_v[e00] = static_cast<S>( (m11*m22 - m12*m21) * invDet );
        m_v[e01] = static_cast<S>( (m02*m21 - m01*m22) * invDet );
        m_v[e02] = static_cast<S>( (m01*m12 - m02*m11) * invDet );
        m_v[e10] = static_cast<S>( (m12*m20 - m10*m22) * invDet );
        m_v[e11] = static_cast<S>( (m00*m22 - m02*m20) * invDet );
        m_v[e12] = static_cast<S>( (m02*m10 - m00*m12) * invDet );
        m_v[e20] = static_cast<S>( (m10*m21 - m11*m20) * invDet );
        m_v[e21] = static_cast<S>( (m01*m20 - m00*m21) * invDet );
        m_v[e22] = static_cast<S>( (m00*m11 - m01*m10) * invDet );

        return true;
    }
    else
    {
        return false;
    }
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
const Matrix3<S> Matrix3<S>::getInverted(bool* pInvertible) const
{
    Matrix3 m(*this);
    if (m.invert())
    {
        if (pInvertible) *pInvertible = true;
    }
    else
    {
        if (pInvertible) *pInvertible = false;
        m.setZero();
    }

    return m;
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
S Matrix3<S>::determinant() const
{
    S det = m_v[e00]*m_v[e11]*m_v[e22] + 
            m_v[e01]*m_v[e12]*m_v[e20] + 
            m_v[e02]*m_v[e10]*m_v[e21] - 
            m_v[e00]*m_v[e12]*m_v[e21] - 
            m_v[e01]*m_v[e10]*m_v[e22] -
            m_v[e02]*m_v[e11]*m_v[e20];

    return det;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename S>
void Matrix3<S>::transpose()
{
    S tmp;
    int i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = i + 1; j < 3; j++)
        {
            tmp = m_v[j + 3*i];
            m_v[j + 3*i] = m_v[3*j + i];
            m_v[3*j + i] = tmp;
        }
    }
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
const S* Matrix3<S>::ptr() const
{
    return m_v;
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix3<S> Matrix3<S>::fromRotation(Vector3<S> axis, S angle)
{
    axis.normalize();

    S rcos = Math::cos(angle);
    S rsin = Math::sin(angle);

    Matrix3 m;
    m.m_v[e00] =             rcos + axis.x()*axis.x()*(1 - rcos);
    m.m_v[e10] =  axis.z() * rsin + axis.y()*axis.x()*(1 - rcos);
    m.m_v[e20] = -axis.y() * rsin + axis.z()*axis.x()*(1 - rcos);
    m.m_v[e01] = -axis.z() * rsin + axis.x()*axis.y()*(1 - rcos);
    m.m_v[e11] =             rcos + axis.y()*axis.y()*(1 - rcos);
    m.m_v[e21] =  axis.x() * rsin + axis.z()*axis.y()*(1 - rcos);
    m.m_v[e02] =  axis.y() * rsin + axis.x()*axis.z()*(1 - rcos);
    m.m_v[e12] = -axis.x() * rsin + axis.y()*axis.z()*(1 - rcos);
    m.m_v[e22] =             rcos + axis.z()*axis.z()*(1 - rcos);
    
    return m;
}


} 

