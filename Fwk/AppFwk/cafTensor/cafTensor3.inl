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

#include "cvfAssert.h"
#include "cvfMath.h"
#include "cvfSystem.h"
#include <algorithm>
#include "cvfMatrix3.h"
#include <cmath>

namespace caf {


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template< typename S>
caf::Tensor3<S>::Tensor3()
{
    m_tensor[0] = (S) 0.0;
    m_tensor[1] = (S) 0.0;
    m_tensor[2] = (S) 0.0;
    m_tensor[3] = (S) 0.0;
    m_tensor[4] = (S) 0.0;
    m_tensor[5] = (S) 0.0;
}

//----------------------------------------------------------------------------------------------------
/// Copy constructor
//----------------------------------------------------------------------------------------------------
template <typename S>
inline Tensor3<S>::Tensor3(const Tensor3& other)
{
    cvf::System::memcpy(m_tensor, sizeof(m_tensor), other.m_tensor, sizeof(other.m_tensor));
}

//----------------------------------------------------------------------------------------------------
/// Explicit Cast constructor
//----------------------------------------------------------------------------------------------------
template <typename S>
template <typename T>
Tensor3<S>::Tensor3(const Tensor3<T>& other)
{
    m_tensor[SXX] = other[Tensor3<T>::SXX];
    m_tensor[SYY] = other[Tensor3<T>::SYY];
    m_tensor[SZZ] = other[Tensor3<T>::SZZ];
    m_tensor[SXY] = other[Tensor3<T>::SXY];
    m_tensor[SYZ] = other[Tensor3<T>::SYZ];
    m_tensor[SZX] = other[Tensor3<T>::SZX];    
}

//----------------------------------------------------------------------------------------------------
/// Constructor with explicit initialization of all tensor elements.
/// 
//----------------------------------------------------------------------------------------------------
template <typename S>
Tensor3<S>::Tensor3(S sxx, S syy, S szz, S sxy, S syz, S szx)
{
    m_tensor[0] = sxx;
    m_tensor[1] = syy;
    m_tensor[2] = szz;
    m_tensor[3] = sxy;
    m_tensor[4] = syz;
    m_tensor[5] = szx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
Tensor3<S> caf::Tensor3<S>::invalid()
{
    return caf::Tensor3<S>(std::numeric_limits<S>::infinity(), std::numeric_limits<S>::infinity(),
                           std::numeric_limits<S>::infinity(), std::numeric_limits<S>::infinity(),
                           std::numeric_limits<S>::infinity(), std::numeric_limits<S>::infinity());
}

//----------------------------------------------------------------------------------------------------
/// Assignment operator
//----------------------------------------------------------------------------------------------------
template <typename S>
inline Tensor3<S>& Tensor3<S>::operator=(const Tensor3& obj)
{
    cvf::System::memcpy(m_tensor, sizeof(m_tensor), obj.m_tensor, sizeof(obj.m_tensor));
    return *this;
}

//--------------------------------------------------------------------------------------------------
/// Component-wise addition
//--------------------------------------------------------------------------------------------------
template< typename S>
Tensor3<S> caf::Tensor3<S>::operator+(const Tensor3& rhs) const
{
    Tensor3<S> result(*this);

    result.m_tensor[0] += rhs.m_tensor[0];
    result.m_tensor[1] += rhs.m_tensor[1];
    result.m_tensor[2] += rhs.m_tensor[2];
    result.m_tensor[3] += rhs.m_tensor[3];
    result.m_tensor[4] += rhs.m_tensor[4];
    result.m_tensor[5] += rhs.m_tensor[5];

    return result;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template< typename S>
Tensor3<S> caf::Tensor3<S>::operator*(S scale) const
{
    Tensor3<S> result(*this);

    result.m_tensor[0] *= scale;
    result.m_tensor[1] *= scale;
    result.m_tensor[2] *= scale;
    result.m_tensor[3] *= scale;
    result.m_tensor[4] *= scale;
    result.m_tensor[5] *= scale;

    return result;
}


//----------------------------------------------------------------------------------------------------
/// Check if matrices are equal using exact comparisons.
//----------------------------------------------------------------------------------------------------
template<typename S>
bool Tensor3<S>::equals(const Tensor3& ten) const
{
    for (int i = 0; i < 6; i++)
    {
        if (m_tensor[i] != ten.m_tensor[i]) return false;
    }

    return true;
}


//----------------------------------------------------------------------------------------------------
/// Comparison operator. Checks for equality using exact comparisons.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Tensor3<S>::operator==(const Tensor3& rhs) const
{
    return this->equals(rhs);
}


//----------------------------------------------------------------------------------------------------
/// Comparison operator. Checks for not equal using exact comparisons.
//----------------------------------------------------------------------------------------------------
template <typename S>
bool Tensor3<S>::operator!=(const Tensor3& rhs) const
{
    int i;
    for (i = 0; i < 6; i++)
    {
        if (m_tensor[i] != rhs.m_tensor[i]) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Get modifiable component 0,1,2. E.g. x = v[0];
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S Tensor3<S>::operator[](TensorComponentEnum index) const
{
    CVF_TIGHT_ASSERT(index >= 0);
    CVF_TIGHT_ASSERT(index < 6);

    return m_tensor[index];
}


//--------------------------------------------------------------------------------------------------
/// Get const component 0,1,2. E.g. x = v[0];
//--------------------------------------------------------------------------------------------------
template<typename S>
inline S& Tensor3<S>::operator[](TensorComponentEnum index) 
{
    CVF_TIGHT_ASSERT(index >= 0);
    CVF_TIGHT_ASSERT(index < 6);

    return m_tensor[index];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template< typename S>
void Tensor3<S>::setFromInternalLayout(S* tensorData)
{
    m_tensor[0] = tensorData[0];
    m_tensor[1] = tensorData[1];
    m_tensor[2] = tensorData[2];
    m_tensor[3] = tensorData[3];
    m_tensor[4] = tensorData[4];
    m_tensor[5] = tensorData[5];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template< typename S>
void Tensor3<S>::setFromAbaqusLayout(S* tensorData)
{
    m_tensor[0] = tensorData[0];
    m_tensor[1] = tensorData[1];
    m_tensor[2] = tensorData[2];
    m_tensor[3] = tensorData[3];
    m_tensor[4] = tensorData[5];
    m_tensor[5] = tensorData[4];
}



cvf::Mat3d cofactor3(const cvf::Mat3d& mx);
cvf::Vec3d eigenVector3(const cvf::Mat3d& mx, double eigenValue, bool* computedOk);

//--------------------------------------------------------------------------------------------------
/// Compute principal values and optionally the principal directions
/// The tensor must be laid out as follows: SXX, SYY, SZZ, SXY, SYZ, SZX
//--------------------------------------------------------------------------------------------------
template<typename S>
cvf::Vec3f Tensor3<S>::calculatePrincipals( cvf::Vec3f principalDirections[3]) const
{
    CVF_TIGHT_ASSERT(m_tensor);

    const float floatThreshold = 1.0e-30f;
    const double doubleThreshold = 1.0e-60;

    cvf::Vec3f principalValues;
    
       // Init return arrays to invalid

    principalValues[0] = std::numeric_limits<float>::infinity();
    principalValues[1] = std::numeric_limits<float>::infinity();
    principalValues[2] = std::numeric_limits<float>::infinity();

    if (principalDirections)
    {
        principalDirections[0] = cvf::Vec3f::ZERO;
        principalDirections[1] = cvf::Vec3f::ZERO;
        principalDirections[2] = cvf::Vec3f::ZERO;
    }

    // Return if we have an undefined component
 
    int i;
    for (i = 0; i < 6; i++)
    {
        if (m_tensor[i] == std::numeric_limits<S>::infinity()) 
        {
            return principalValues;
        }
    }
    
    // Return 0, 0, 0 if all components are zero

    bool isAllTensCompsZero = true;
    for (i = 0; i < 6; i++)
    {
        if (!(fabs(m_tensor[i]) < floatThreshold))
        {
            isAllTensCompsZero = false;
            break;
        }
    }

    if (isAllTensCompsZero)
    {
        return cvf::Vec3f::ZERO;
    }

    double SXX = m_tensor[0], SYY = m_tensor[1], SZZ = m_tensor[2];
    double SXY = m_tensor[3], SYZ = m_tensor[4], SZX = m_tensor[5];

    double pressure = -(SXX + SYY + SZZ)/3.0;

    // Normally we would solve the eigenvalues by solving the 3'rd degree equation:
    //    -sigma^3 + A*sigma^2 - B*sigma + C = 0
    // in which A, B, and C are the invariants of the stress tensor.
    // http://www.engapplets.vt.edu/Mohr/java/nsfapplets/MohrCircles2-3D/Theory/theory.htm

    // But the roots(eigenvalues) are calculated by transforming the above equation into
    // s**3 + aa*s + b = 0 and using the trignometric solution.
    // See crc standard mathematical tables 19th edition pp. 103-104.

    SXX += pressure;
    SYY += pressure;
    SZZ += pressure;

    double S1, S2, S3;
    double AA, BB, CC, DD, angleP;

    AA = SXY*SXY + SYZ*SYZ + SZX*SZX - SXX*SYY - SYY*SZZ - SXX*SZZ;

    BB = SXX * SYZ * SYZ 
        + SYY * SZX * SZX 
        + SZZ * SXY * SXY 
        - SXX * SYY * SZZ 
        - 2.0 * SXY * SYZ * SZX;

    if (fabs(AA) < doubleThreshold)
    {
        S1 = 0.0;
        S2 = 0.0;
        S3 = 0.0;
    }
    else
    {
        CC = -sqrt(27.0/AA) * BB * 0.5 / AA;

        if      (CC > 1.0)  CC = 1.0;
        else if (CC < -1.0) CC = -1.0;

        angleP  = acos(CC)/3.0;
        DD      = 2.0*sqrt(AA/3.0);
        S1      = DD*cos(angleP);
        S2      = DD*cos(angleP + 4.0*cvf::PI_D/3.0);
        S3      = DD*cos(angleP + 2.0*cvf::PI_D/3.0);
    }

    int idxPMin = 2;
    int idxPMid = 1;
    int idxPMax = 0;

    double principalsd[3];
    principalsd[idxPMax] = (S1 - pressure);
    principalsd[idxPMid] = (S2 - pressure);
    principalsd[idxPMin] = (S3 - pressure);

    // Sort the principals if we have no Z component in the tensor at all 
    if ((m_tensor[2] == 0.0f) && (m_tensor[4] == 0.0f) && (m_tensor[5] == 0.0f))
    {
        if (fabs(principalsd[idxPMin]) > fabs(principalsd[idxPMid])) std::swap(idxPMin, idxPMid);
        if (fabs(principalsd[idxPMin]) > fabs(principalsd[idxPMax])) std::swap(idxPMin, idxPMax);
        if (principalsd[idxPMax]       < principalsd[idxPMid])       std::swap(idxPMax, idxPMid);

        principalsd[idxPMin] = 0;
    }

    // Calculate the principal directions if needed

    if (principalDirections)
    {
        cvf::Mat3d T;
        T(0,0) = m_tensor[0];     T(0,1) = m_tensor[3];     T(0,2) = m_tensor[5];
        T(1,0) = m_tensor[3];     T(1,1) = m_tensor[1];     T(1,2) = m_tensor[4];
        T(2,0) = m_tensor[5];     T(2,1) = m_tensor[4];     T(2,2) = m_tensor[2];

        principalDirections[0] = cvf::Vec3f(eigenVector3(T, principalsd[idxPMax], NULL));
        principalDirections[0].normalize();
        principalDirections[1] = cvf::Vec3f(eigenVector3(T, principalsd[idxPMid], NULL));
        principalDirections[1].normalize();
        principalDirections[2] = cvf::Vec3f(eigenVector3(T, principalsd[idxPMin], NULL));
        principalDirections[2].normalize();
    }
   
    principalValues[0] = (float)principalsd[idxPMax];
    principalValues[1] = (float)principalsd[idxPMid];
    principalValues[2] = (float)principalsd[idxPMin];

    return principalValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template< typename S>
float caf::Tensor3<S>::calculateVonMises() const
{
    return (float) sqrt( (   (m_tensor[0]*m_tensor[0] + m_tensor[1]*m_tensor[1] + m_tensor[2]*m_tensor[2]) ) +
                         (  -(m_tensor[0]*m_tensor[1] + m_tensor[1]*m_tensor[2] + m_tensor[0]*m_tensor[2]) ) +
                         ( 3*(m_tensor[3]*m_tensor[3] + m_tensor[4]*m_tensor[4] + m_tensor[5]*m_tensor[5]) )   );
}

//--------------------------------------------------------------------------------------------------
/// Calculates Trot = rotMx*T*transpose(rotMx)
//--------------------------------------------------------------------------------------------------
template< typename S>
Tensor3<S> caf::Tensor3<S>::rotated(const cvf::Matrix3<S>& rotMx) const
{
    cvf::Matrix3<S> tensor(m_tensor[SXX], m_tensor[SXY], m_tensor[SZX], 
                           m_tensor[SXY], m_tensor[SYY], m_tensor[SYZ], 
                           m_tensor[SZX], m_tensor[SYZ], m_tensor[SZZ]); 

    cvf::Matrix3<S> transposedRotMx = rotMx;
    transposedRotMx.transpose();
    cvf::Matrix3<S>  rotatedTensor = rotMx * tensor * transposedRotMx;

    return Tensor3(rotatedTensor(0,0), rotatedTensor(1,1), rotatedTensor(2,2), 
                   rotatedTensor(1,0), rotatedTensor(1,2), rotatedTensor(0,2));
}


}

