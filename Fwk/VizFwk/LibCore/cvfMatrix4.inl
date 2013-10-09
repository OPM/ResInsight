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
/// \class cvf::Matrix4
/// \ingroup Core
///
/// Matrices are stored internally as a one dimensional array for performance reasons. 
/// 
/// The internal indices into the 1D array are as follows:
/// 
/// <PRE>
///   | m00  m01  m02  m03 |     | 0  4   8  12 | 
///   | m10  m11  m12  m13 |     | 1  5   9  13 | 
///   | m20  m21  m22  m23 |     | 2  6  10  14 | 
///   | m30  m31  m32  m33 |     | 3  7  11  15 | 
/// </PRE>
///
/// This is consistent with the way matrices are represented in %OpenGL.
/// To exemplify, translation values are stored in elements 12,13,14; see figure below
///
/// <PRE>
///   | 1  0  0 Tx |
///   | 0  1  0 Ty |
///   | 0  0  1 Tz |
///   | 0  0  0  1 |
/// </PRE>
///
/// From the %OpenGL red book (page 68)   v' = M*v
///
/// <PRE>
///   | X'|   | 1  0  0 Tx |   | X |
///   | Y'|   | 0  1  0 Ty |   | Y |
///   | Z'| = | 0  0  1 Tz | * | Z |
///   | 1 |   | 0  0  0  1 |   | 1 |
/// </PRE>
///
/// Beware when porting code that uses C style double array indexing. In this case, the first 
/// index given will corrspond to the columnd, eg M[3][0] = Tx, M[3][1] = Ty, M[3][2] = Tz
///
/// To ease accessing the internal 1D array in implementations, the private eij constants can be used.
/// These are consistent with the normal row column ordering, so that e02 accesses the element 
/// in the first row and the third column.
///
//==================================================================================================

template<typename S> Matrix4<S> const Matrix4<S>::IDENTITY;
template<typename S> Matrix4<S> const Matrix4<S>::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

//----------------------------------------------------------------------------------------------------
/// Default constructor. Initializes matrix to identity 
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix4<S>::Matrix4()
{
    setIdentity();
}


//----------------------------------------------------------------------------------------------------
/// Copy constructor
//----------------------------------------------------------------------------------------------------
template <typename S>
inline Matrix4<S>::Matrix4(const Matrix4& other)
{
    System::memcpy(m_v, sizeof(m_v), other.m_v, sizeof(other.m_v));
}


//----------------------------------------------------------------------------------------------------
/// Constructor with explicit initialization of all matrix elements.
/// 
/// The value of the parameter \a mrc will be placed in row r and column c of the matrix, eg m23
/// goes into row 2, column 3.
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix4<S>::Matrix4(S m00, S m01, S m02, S m03, S m10, S m11, S m12, S m13, S m20, S m21, S m22, S m23, S m30, S m31, S m32, S m33)
{
    m_v[e00] = m00;
    m_v[e10] = m10;
    m_v[e20] = m20;
    m_v[e30] = m30;
    m_v[e01] = m01;
    m_v[e11] = m11;
    m_v[e21] = m21;
    m_v[e31] = m31;
    m_v[e02] = m02;
    m_v[e12] = m12;
    m_v[e22] = m22;
    m_v[e32] = m32;
    m_v[e03] = m03;
    m_v[e13] = m13;
    m_v[e23] = m23;
    m_v[e33] = m33;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
Matrix4<S>::Matrix4(const Matrix3<S>& other)
{
    setFromMatrix3(other);
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
Matrix4<S>::Matrix4(const Matrix4<T>& other)
{
    m_v[e00] = static_cast<S>(other.rowCol(0, 0));
    m_v[e10] = static_cast<S>(other.rowCol(1, 0));
    m_v[e20] = static_cast<S>(other.rowCol(2, 0));
    m_v[e30] = static_cast<S>(other.rowCol(3, 0));
    m_v[e01] = static_cast<S>(other.rowCol(0, 1));
    m_v[e11] = static_cast<S>(other.rowCol(1, 1));
    m_v[e21] = static_cast<S>(other.rowCol(2, 1));
    m_v[e31] = static_cast<S>(other.rowCol(3, 1));
    m_v[e02] = static_cast<S>(other.rowCol(0, 2));
    m_v[e12] = static_cast<S>(other.rowCol(1, 2));
    m_v[e22] = static_cast<S>(other.rowCol(2, 2));
    m_v[e32] = static_cast<S>(other.rowCol(3, 2));
    m_v[e03] = static_cast<S>(other.rowCol(0, 3));
    m_v[e13] = static_cast<S>(other.rowCol(1, 3));
    m_v[e23] = static_cast<S>(other.rowCol(2, 3));
    m_v[e33] = static_cast<S>(other.rowCol(3, 3));
}


//----------------------------------------------------------------------------------------------------
/// Assignment operator
//----------------------------------------------------------------------------------------------------
template <typename S>
inline Matrix4<S>& Matrix4<S>::operator=(const Matrix4& obj)
{
    System::memcpy(m_v, sizeof(m_v), obj.m_v, sizeof(obj.m_v));
    return *this;
}



//----------------------------------------------------------------------------------------------------
/// Check if matrices are equal using exact comparisons.
//----------------------------------------------------------------------------------------------------
template<typename S>
bool Matrix4<S>::equals(const Matrix4& mat) const
{
    for (int i = 0; i < 16; i++)
    {
        if (m_v[i] != mat.m_v[i]) return false;
    }

    return true;
}


//----------------------------------------------------------------------------------------------------
/// Comparison operator. Checks for equality using exact comparisons.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix4<S>::operator==(const Matrix4& rhs) const
{
    return this->equals(rhs);
}


//----------------------------------------------------------------------------------------------------
/// Comparison operator. Checks for not equal using exact comparisons.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix4<S>::operator!=(const Matrix4& rhs) const
{
    int i;
    for (i = 0; i < 16; i++)
    {
        if (m_v[i] != rhs.m_v[i]) return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Multiplies this matrix M with the matrix \a mat, M = M*mat
//--------------------------------------------------------------------------------------------------
template<typename S>
void Matrix4<S>::multiply(const Matrix4& mat)
{
    *this = (*this)*mat;
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
const Matrix4<S> Matrix4<S>::operator*(const Matrix4& rhs) const
{
    Matrix4 m;
    m.m_v[e00] = m_v[e00] * rhs.m_v[e00] + m_v[e01] * rhs.m_v[e10] + m_v[e02] * rhs.m_v[e20] + m_v[e03] * rhs.m_v[e30];
    m.m_v[e01] = m_v[e00] * rhs.m_v[e01] + m_v[e01] * rhs.m_v[e11] + m_v[e02] * rhs.m_v[e21] + m_v[e03] * rhs.m_v[e31];
    m.m_v[e02] = m_v[e00] * rhs.m_v[e02] + m_v[e01] * rhs.m_v[e12] + m_v[e02] * rhs.m_v[e22] + m_v[e03] * rhs.m_v[e32];
    m.m_v[e03] = m_v[e00] * rhs.m_v[e03] + m_v[e01] * rhs.m_v[e13] + m_v[e02] * rhs.m_v[e23] + m_v[e03] * rhs.m_v[e33];
         
    m.m_v[e10] = m_v[e10] * rhs.m_v[e00] + m_v[e11] * rhs.m_v[e10] + m_v[e12] * rhs.m_v[e20] + m_v[e13] * rhs.m_v[e30];
    m.m_v[e11] = m_v[e10] * rhs.m_v[e01] + m_v[e11] * rhs.m_v[e11] + m_v[e12] * rhs.m_v[e21] + m_v[e13] * rhs.m_v[e31];
    m.m_v[e12] = m_v[e10] * rhs.m_v[e02] + m_v[e11] * rhs.m_v[e12] + m_v[e12] * rhs.m_v[e22] + m_v[e13] * rhs.m_v[e32];
    m.m_v[e13] = m_v[e10] * rhs.m_v[e03] + m_v[e11] * rhs.m_v[e13] + m_v[e12] * rhs.m_v[e23] + m_v[e13] * rhs.m_v[e33];
         
    m.m_v[e20] = m_v[e20] * rhs.m_v[e00] + m_v[e21] * rhs.m_v[e10] + m_v[e22] * rhs.m_v[e20] + m_v[e23] * rhs.m_v[e30];
    m.m_v[e21] = m_v[e20] * rhs.m_v[e01] + m_v[e21] * rhs.m_v[e11] + m_v[e22] * rhs.m_v[e21] + m_v[e23] * rhs.m_v[e31];
    m.m_v[e22] = m_v[e20] * rhs.m_v[e02] + m_v[e21] * rhs.m_v[e12] + m_v[e22] * rhs.m_v[e22] + m_v[e23] * rhs.m_v[e32];
    m.m_v[e23] = m_v[e20] * rhs.m_v[e03] + m_v[e21] * rhs.m_v[e13] + m_v[e22] * rhs.m_v[e23] + m_v[e23] * rhs.m_v[e33];
         
    m.m_v[e30] = m_v[e30] * rhs.m_v[e00] + m_v[e31] * rhs.m_v[e10] + m_v[e32] * rhs.m_v[e20] + m_v[e33] * rhs.m_v[e30];
    m.m_v[e31] = m_v[e30] * rhs.m_v[e01] + m_v[e31] * rhs.m_v[e11] + m_v[e32] * rhs.m_v[e21] + m_v[e33] * rhs.m_v[e31];
    m.m_v[e32] = m_v[e30] * rhs.m_v[e02] + m_v[e31] * rhs.m_v[e12] + m_v[e32] * rhs.m_v[e22] + m_v[e33] * rhs.m_v[e32];
    m.m_v[e33] = m_v[e30] * rhs.m_v[e03] + m_v[e31] * rhs.m_v[e13] + m_v[e32] * rhs.m_v[e23] + m_v[e33] * rhs.m_v[e33];
          
    return m;
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
const Vector4<S> Matrix4<S>::operator*(const Vector4<S>& rhs) const
{
    return Vector4<S>(m_v[e00]*rhs.x() + m_v[e01]*rhs.y() + m_v[e02]*rhs.z() + m_v[e03]*rhs.w(),
                      m_v[e10]*rhs.x() + m_v[e11]*rhs.y() + m_v[e12]*rhs.z() + m_v[e13]*rhs.w(),
                      m_v[e20]*rhs.x() + m_v[e21]*rhs.y() + m_v[e22]*rhs.z() + m_v[e23]*rhs.w(),
                      m_v[e30]*rhs.x() + m_v[e31]*rhs.y() + m_v[e32]*rhs.z() + m_v[e33]*rhs.w());
}


//----------------------------------------------------------------------------------------------------
/// Sets this matrix to identity
//----------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::setIdentity()
{
    m_v[0]  = 1;
    m_v[1]  = 0;    
    m_v[2]  = 0;
    m_v[3]  = 0;

    m_v[4]  = 0;
    m_v[5]  = 1;
    m_v[6]  = 0;
    m_v[7]  = 0;

    m_v[8]  = 0;
    m_v[9]  = 0;
    m_v[10] = 1;
    m_v[11] = 0;

    m_v[12] = 0;
    m_v[13] = 0;
    m_v[14] = 0;
    m_v[15] = 1;
}


//----------------------------------------------------------------------------------------------------
/// Check if this matrix is identity using exact comparisons
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix4<S>::isIdentity() const
{
    if (m_v[0]  != 1) return false;
    if (m_v[1]  != 0) return false;
    if (m_v[2]  != 0) return false;
    if (m_v[3]  != 0) return false;

    if (m_v[4]  != 0) return false;
    if (m_v[5]  != 1) return false;
    if (m_v[6]  != 0) return false;
    if (m_v[7]  != 0) return false;

    if (m_v[8]  != 0) return false;
    if (m_v[9]  != 0) return false;
    if (m_v[10] != 1) return false;
    if (m_v[11] != 0) return false;

    if (m_v[12] != 0) return false;
    if (m_v[13] != 0) return false;
    if (m_v[14] != 0) return false;
    if (m_v[15] != 1) return false;

    return true;
}


//----------------------------------------------------------------------------------------------------
/// Set all matrix elements to 0
//----------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::setZero()
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
    m_v[9]  = 0;
    m_v[10] = 0;
    m_v[11] = 0;
    m_v[12] = 0;
    m_v[13] = 0;
    m_v[14] = 0;
    m_v[15] = 0;
}


//----------------------------------------------------------------------------------------------------
/// Returns true if all elements of the matrix are 0. Uses exact comparison.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix4<S>::isZero() const
{
    if (m_v[0]  != 0) return false;
    if (m_v[1]  != 0) return false;
    if (m_v[2]  != 0) return false;
    if (m_v[3]  != 0) return false;

    if (m_v[4]  != 0) return false;
    if (m_v[5]  != 0) return false;
    if (m_v[6]  != 0) return false;
    if (m_v[7]  != 0) return false;

    if (m_v[8]  != 0) return false;
    if (m_v[9]  != 0) return false;
    if (m_v[10] != 0) return false;
    if (m_v[11] != 0) return false;

    if (m_v[12] != 0) return false;
    if (m_v[13] != 0) return false;
    if (m_v[14] != 0) return false;
    if (m_v[15] != 0) return false;

    return true;
}


//----------------------------------------------------------------------------------------------------
/// Set the matrix element at the specified row and column
//----------------------------------------------------------------------------------------------------
template <typename S>
inline void Matrix4<S>::setRowCol(int row, int col, S value)
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 4);
    CVF_TIGHT_ASSERT(col >= 0 && col < 4);

    m_v[4*col + row] = value;
}


//----------------------------------------------------------------------------------------------------
/// Get the matrix element at the specified row and column
//----------------------------------------------------------------------------------------------------
template <typename S>
inline S Matrix4<S>::rowCol(int row, int col) const
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 4);
    CVF_TIGHT_ASSERT(col >= 0 && col < 4);

    return m_v[4*col + row];
}

//--------------------------------------------------------------------------------------------------
/// Get modifiable reference to the the matrix element at the specified row and column
//--------------------------------------------------------------------------------------------------
template <typename S>
inline S& Matrix4<S>::operator()(int row, int col)
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 4);
    CVF_TIGHT_ASSERT(col >= 0 && col < 4);

    return m_v[4*col + row];
}


//--------------------------------------------------------------------------------------------------
/// Get the matrix element at the specified row and column
//--------------------------------------------------------------------------------------------------
template <typename S>
inline S Matrix4<S>::operator()(int row, int col) const
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 4);
    CVF_TIGHT_ASSERT(col >= 0 && col < 4);

    return m_v[4*col + row];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::setRow(int row, const Vector4<S>& vector)
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 4);

    m_v[row]      = vector.x();
    m_v[row + 4]  = vector.y();
    m_v[row + 8]  = vector.z();
    m_v[row + 12] = vector.w();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename S>
Vector4<S> Matrix4<S>::row(int row) const
{
    CVF_TIGHT_ASSERT(row >= 0 && row < 4);

    return Vector4<S>(m_v[row], m_v[row + 4], m_v[row + 8], m_v[row + 12]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::setCol(int column, const Vector4<S>& vector)
{
    CVF_TIGHT_ASSERT(column >= 0 && column < 4);

    m_v[4*column]     = vector.x();
    m_v[4*column + 1] = vector.y();
    m_v[4*column + 2] = vector.z();
    m_v[4*column + 3] = vector.w();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename S>
Vector4<S> Matrix4<S>::col(int column) const
{
    CVF_TIGHT_ASSERT(column >= 0 && column < 4);

    return Vector4<S>(m_v[4*column], m_v[4*column + 1], m_v[4*column + 2], m_v[4*column + 3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
template<typename T>
void Matrix4<S>::set(const Matrix4<T>& mat)
{
    const T* matPtr = mat.ptr();
    for (int i = 0; i < 16; i++)
    {
        m_v[i] = static_cast<S>(matPtr[i]);
    }
}


//----------------------------------------------------------------------------------------------------
/// Extracts the translation part of the transformation matrix
//----------------------------------------------------------------------------------------------------
template <typename S>
Vector3<S> Matrix4<S>::translation() const
{
    return Vector3<S>(m_v[e03], m_v[e13], m_v[e23]);
}


//----------------------------------------------------------------------------------------------------
/// Sets the translation transformation part of the matrix
//----------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::setTranslation(const Vector3<S>& trans)
{
    m_v[e03] = trans.x();
    m_v[e13] = trans.y();
    m_v[e23] = trans.z();
}


//----------------------------------------------------------------------------------------------------
/// Adds translation by pre-multiplying the matrix with a matrix containing the specified translation
/// 
/// Adds translation to this (transformation) matrix by pre-multiplying the current matrix M with 
/// a matrix T containing only the specified translation. 
/// Calling this function has the effect of doing the multiplication M' = T x M
/// 
/// \param trans  Specifies the X, Y and Z components of the translation.
//----------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::translatePreMultiply(const Vector3<S>& trans)
{
    m_v[e00] += trans.x()*m_v[e30];
    m_v[e01] += trans.x()*m_v[e31];
    m_v[e02] += trans.x()*m_v[e32];
    m_v[e03] += trans.x()*m_v[e33];

    m_v[e10] += trans.y()*m_v[e30];
    m_v[e11] += trans.y()*m_v[e31];
    m_v[e12] += trans.y()*m_v[e32];
    m_v[e13] += trans.y()*m_v[e33];

    m_v[e20] += trans.z()*m_v[e30];
    m_v[e21] += trans.z()*m_v[e31];
    m_v[e22] += trans.z()*m_v[e32];
    m_v[e23] += trans.z()*m_v[e33];
}


//----------------------------------------------------------------------------------------------------
/// Adds translation by post-multiplying the matrix with a matrix containing the specified translation
///
/// Adds translation to this (transformation) matrix by post-multiplying the current matrix M with 
/// a matrix T containing only the specified translation. 
/// Calling this function has the effect of doing the multiplication M' = M x T
/// 
/// \param trans  Specifies the X, Y and Z coordinates of the translation.
//----------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::translatePostMultiply(const Vector3<S>& trans)
{
    m_v[e03] += trans.x()*m_v[e00] + trans.y()*m_v[e01] + trans.z()*m_v[e02];
    m_v[e13] += trans.x()*m_v[e10] + trans.y()*m_v[e11] + trans.z()*m_v[e12];
    m_v[e23] += trans.x()*m_v[e20] + trans.y()*m_v[e21] + trans.z()*m_v[e22];
    m_v[e33] += trans.x()*m_v[e30] + trans.y()*m_v[e31] + trans.z()*m_v[e32];
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Matrix4<S>::invert()
{
    // Use double internally here
    const double m00 = static_cast<double>(m_v[e00]);
    const double m10 = static_cast<double>(m_v[e10]);
    const double m20 = static_cast<double>(m_v[e20]);
    const double m30 = static_cast<double>(m_v[e30]);
    const double m01 = static_cast<double>(m_v[e01]);
    const double m11 = static_cast<double>(m_v[e11]);
    const double m21 = static_cast<double>(m_v[e21]);
    const double m31 = static_cast<double>(m_v[e31]);
    const double m02 = static_cast<double>(m_v[e02]);
    const double m12 = static_cast<double>(m_v[e12]);
    const double m22 = static_cast<double>(m_v[e22]);
    const double m32 = static_cast<double>(m_v[e32]);
    const double m03 = static_cast<double>(m_v[e03]);
    const double m13 = static_cast<double>(m_v[e13]);
    const double m23 = static_cast<double>(m_v[e23]);
    const double m33 = static_cast<double>(m_v[e33]);

    const double s0 = m00 * m11 - m01 * m10;   
    const double s1 = m00 * m12 - m02 * m10;   
    const double s2 = m00 * m13 - m03 * m10;   
    const double s3 = m01 * m12 - m02 * m11;   
    const double s4 = m01 * m13 - m03 * m11;   
    const double s5 = m02 * m13 - m03 * m12;   
    
    const double c5 = m22 * m33 - m23 * m32;
    const double c4 = m21 * m33 - m23 * m31;
    const double c3 = m21 * m32 - m22 * m31;
    const double c2 = m20 * m33 - m23 * m30;
    const double c1 = m20 * m32 - m22 * m30;
    const double c0 = m20 * m31 - m21 * m30;

    const double det = s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0;

    if (Math::abs(det) > std::numeric_limits<double>::epsilon())
    {
        const double invDet = 1/det;

        m_v[e00] = static_cast<S>( (   m11 * c5 - m12 * c4 + m13 * c3 ) * invDet );
        m_v[e10] = static_cast<S>( ( - m10 * c5 + m12 * c2 - m13 * c1 ) * invDet );
        m_v[e20] = static_cast<S>( (   m10 * c4 - m11 * c2 + m13 * c0 ) * invDet );
        m_v[e30] = static_cast<S>( ( - m10 * c3 + m11 * c1 - m12 * c0 ) * invDet );
        m_v[e01] = static_cast<S>( ( - m01 * c5 + m02 * c4 - m03 * c3 ) * invDet );
        m_v[e11] = static_cast<S>( (   m00 * c5 - m02 * c2 + m03 * c1 ) * invDet );
        m_v[e21] = static_cast<S>( ( - m00 * c4 + m01 * c2 - m03 * c0 ) * invDet );
        m_v[e31] = static_cast<S>( (   m00 * c3 - m01 * c1 + m02 * c0 ) * invDet );
        m_v[e02] = static_cast<S>( (   m31 * s5 - m32 * s4 + m33 * s3 ) * invDet );
        m_v[e12] = static_cast<S>( ( - m30 * s5 + m32 * s2 - m33 * s1 ) * invDet );
        m_v[e22] = static_cast<S>( (   m30 * s4 - m31 * s2 + m33 * s0 ) * invDet );
        m_v[e32] = static_cast<S>( ( - m30 * s3 + m31 * s1 - m32 * s0 ) * invDet );
        m_v[e03] = static_cast<S>( ( - m21 * s5 + m22 * s4 - m23 * s3 ) * invDet );
        m_v[e13] = static_cast<S>( (   m20 * s5 - m22 * s2 + m23 * s1 ) * invDet );
        m_v[e23] = static_cast<S>( ( - m20 * s4 + m21 * s2 - m23 * s0 ) * invDet );
        m_v[e33] = static_cast<S>( (   m20 * s3 - m21 * s1 + m22 * s0 ) * invDet );

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
const Matrix4<S> Matrix4<S>::getInverted(bool* pInvertible) const
{
    Matrix4 m(*this);
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
S Matrix4<S>::determinant() const
{
    // Consider using double precision for the intermediate calculations here
    // Haven't seen any direct need for it yet, but good old GLview API did that
    const S a0 = m_v[e00]*m_v[e11] - m_v[e01]*m_v[e10];
    const S a1 = m_v[e00]*m_v[e12] - m_v[e02]*m_v[e10];
    const S a2 = m_v[e00]*m_v[e13] - m_v[e03]*m_v[e10];
    const S a3 = m_v[e01]*m_v[e12] - m_v[e02]*m_v[e11];
    const S a4 = m_v[e01]*m_v[e13] - m_v[e03]*m_v[e11];
    const S a5 = m_v[e02]*m_v[e13] - m_v[e03]*m_v[e12];

    const S b0 = m_v[e20]*m_v[e31] - m_v[e21]*m_v[e30];
    const S b1 = m_v[e20]*m_v[e32] - m_v[e22]*m_v[e30];
    const S b2 = m_v[e20]*m_v[e33] - m_v[e23]*m_v[e30];
    const S b3 = m_v[e21]*m_v[e32] - m_v[e22]*m_v[e31];
    const S b4 = m_v[e21]*m_v[e33] - m_v[e23]*m_v[e31];
    const S b5 = m_v[e22]*m_v[e33] - m_v[e23]*m_v[e32];

    return (a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename S>
void Matrix4<S>::transpose()
{
    S tmp;
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = i + 1; j < 4; j++)
        {
            tmp = m_v[j + 4*i];
            m_v[j + 4*i] = m_v[4*j + i];
            m_v[4*j + i] = tmp;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename S>
const Matrix4<S> Matrix4<S>::getTransposed() const
{
    Matrix4 m(*this);
    m.transpose();
    return m;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
void Matrix4<S>::setFromMatrix3(const Matrix3<S>& mat3)
{
    m_v[e00] = mat3.rowCol(0, 0);
    m_v[e01] = mat3.rowCol(0, 1);
    m_v[e02] = mat3.rowCol(0, 2);
    m_v[e03] = 0;
    m_v[e10] = mat3.rowCol(1, 0);
    m_v[e11] = mat3.rowCol(1, 1);
    m_v[e12] = mat3.rowCol(1, 2);
    m_v[e13] = 0;
    m_v[e20] = mat3.rowCol(2, 0);
    m_v[e21] = mat3.rowCol(2, 1);
    m_v[e22] = mat3.rowCol(2, 2);
    m_v[e23] = 0;
    m_v[e30] = 0;
    m_v[e31] = 0;
    m_v[e32] = 0;
    m_v[e33] = 1;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
Matrix3<S> Matrix4<S>::toMatrix3() const
{
    return Matrix3<S>(m_v[e00], m_v[e01], m_v[e02],
                      m_v[e10], m_v[e11], m_v[e12],
                      m_v[e20], m_v[e21], m_v[e22]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename S>
void Matrix4<S>::toMatrix3(Matrix3<S>* mat3) const
{
    CVF_ASSERT(mat3);
    mat3->setRowCol(0, 0, m_v[e00]);
    mat3->setRowCol(0, 1, m_v[e01]);
    mat3->setRowCol(0, 2, m_v[e02]);
    mat3->setRowCol(1, 0, m_v[e10]);
    mat3->setRowCol(1, 1, m_v[e11]);
    mat3->setRowCol(1, 2, m_v[e12]);
    mat3->setRowCol(2, 0, m_v[e20]);
    mat3->setRowCol(2, 1, m_v[e21]);
    mat3->setRowCol(2, 2, m_v[e22]);
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
const S* Matrix4<S>::ptr() const
{
    return m_v;
}


//----------------------------------------------------------------------------------------------------
/// Static member function that creates a transformation matrix containing only translation
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix4<S> Matrix4<S>::fromTranslation(const Vector3<S>& trans)
{
    return Matrix4(1, 0, 0, trans.x(),
                   0, 1, 0, trans.y(),
                   0, 0, 1, trans.z(),
                   0, 0, 0, 1);
}


//----------------------------------------------------------------------------------------------------
/// Static member function that creates a transformation matrix containing only scaling
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix4<S> Matrix4<S>::fromScaling(const Vector3<S>& scale)
{
    return Matrix4(scale.x(), 0,         0,         0,
                   0,         scale.y(), 0,         0,
                   0,         0,         scale.z(), 0,
                   0,         0,         0,         1);
}


//----------------------------------------------------------------------------------------------------
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix4<S> Matrix4<S>::fromRotation(Vector3<S> axis, S angle)
{
    axis.normalize();

    S rcos = Math::cos(angle);
    S rsin = Math::sin(angle);

    Matrix4 m;
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


//----------------------------------------------------------------------------------------------------
/// Create a rotation matrix that will align the global X, Y and Z axes with the specified axes.
/// 
/// Note that at least one axis must be specified and all specified axes must be normalized. 
/// If two or three axes are specified, they must be orthogonal to each other.
/// 
/// \param xAxis  Orientation of x axis
/// \param yAxis  Orientation of y axis
/// \param zAxis  Orientation of z axis
//----------------------------------------------------------------------------------------------------
template <typename S>
Matrix4<S> Matrix4<S>::fromCoordSystemAxes(const Vector3<S>* xAxis, const Vector3<S>* yAxis, const Vector3<S>* zAxis)
{
    Vector3<S> x(0, 0, 0);
    Vector3<S> y(0, 0, 0);
    Vector3<S> z(0, 0, 0);

    if (xAxis && yAxis && zAxis)
    {
        x = *xAxis;
        y = *yAxis;
        z = *zAxis;
    }
    else if (xAxis && yAxis)
    {
        x = *xAxis;
        y = *yAxis;
        z = x ^ y;
    }
    else if (xAxis && zAxis)
    {
        x = *xAxis;
        z = *zAxis;
        y = z ^ x;
    }
    else if (yAxis && zAxis)
    {
        y = *yAxis;
        z = *zAxis;
        x = y ^ z;
    }
    else if (xAxis)
    {
        xAxis->createOrthonormalBasis(0, &x, &y, &z);
    }
    else if (yAxis)
    {
        yAxis->createOrthonormalBasis(1, &x, &y, &z);
    }
    else if (zAxis)
    {
        zAxis->createOrthonormalBasis(2, &x, &y, &z);
    }
    else
    {
        CVF_FAIL_MSG("Must specify at least one axis");
    }

    Matrix4 m;
    m.setIdentity();
    m.setCol(0, Vector4<S>(x, 0));
    m.setCol(1, Vector4<S>(y, 0));
    m.setCol(2, Vector4<S>(z, 0));

    return m;
}


//--------------------------------------------------------------------------------------------------
/// Post multiplication: matrix * column vector
/// 
/// v' = M*v
//--------------------------------------------------------------------------------------------------
template<typename S>
cvf::Vector4<S> operator*(const cvf::Matrix4<S>& m, const cvf::Vector4<S>& v)
{
    cvf::Vector4<S> vec(
        v.x()*m.rowCol(0,0) + v.y()*m.rowCol(0,1) + v.z()*m.rowCol(0,2) + v.w()*m.rowCol(0,3),
        v.x()*m.rowCol(1,0) + v.y()*m.rowCol(1,1) + v.z()*m.rowCol(1,2) + v.w()*m.rowCol(1,3),
        v.x()*m.rowCol(2,0) + v.y()*m.rowCol(2,1) + v.z()*m.rowCol(2,2) + v.w()*m.rowCol(2,3),
        v.x()*m.rowCol(3,0) + v.y()*m.rowCol(3,1) + v.z()*m.rowCol(3,2) + v.w()*m.rowCol(3,3)
        );

    return vec;
}

} 

