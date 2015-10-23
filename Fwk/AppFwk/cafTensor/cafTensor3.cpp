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
#include "cvfBase.h"
#include "cafTensor3.h"
#include "cvfVector3.h"

#include "cvfMatrix3.h"

namespace caf {

//--------------------------------------------------------------------------------------------------
/// Compute the cofactors of the 3x3 matrix
/// Cofactor: 
/// The determinant obtained by deleting the row and column of a given element and 
/// preceded by a + or – sign depending whether the element is in a + or – position as follows:
/// 
///  + - + 
///  - + -
///  + - +
//--------------------------------------------------------------------------------------------------
cvf::Mat3d cofactor3(const cvf::Mat3d& mx)
{
    int detIdxi[2];
    int detIdxj[2];

    cvf::Mat3d cof;

    double sign;
    for (int i = 0; i < 3; i++)
    {
        detIdxi[0] = (i == 0) ? 1 : 0;
        detIdxi[1] = (i == 2) ? 1 : 2;

        for (int j = 0; j < 3; j++)
        {
            detIdxj[0] = (j == 0) ? 1 : 0;
            detIdxj[1] = (j == 2) ? 1 : 2;
            sign = (abs(j - i) == 1) ? -1 : 1;

            cof(i, j) = sign * (  mx(detIdxi[0], detIdxj[0]) * mx(detIdxi[1], detIdxj[1])
                                - mx(detIdxi[0], detIdxj[1]) * mx(detIdxi[1], detIdxj[0]));
        }
    }

    return cof;
}
//--------------------------------------------------------------------------------------------------
/// Compute the eigenvector of the matrix corresponding to the provided eigenvalue 
/// The provided eigenvalue must be an actual eigenvalue of the matrix
//--------------------------------------------------------------------------------------------------
cvf::Vec3d eigenVector3(const cvf::Mat3d& mx, double eigenValue, bool* computedOk)
{
    const double doubleThreshold = 1.0e-60;
    if (computedOk) (*computedOk) = false;
    cvf::Mat3d mxMinusEigv = mx;

    for (int i = 0; i < 3; i++) 
    {
        mxMinusEigv(i, i) -= eigenValue;
    }

    cvf::Mat3d cof = cofactor3(mxMinusEigv);

    // Find largest absolute cofactor

    int largestCof_i = -1; 
    int largestCof_j = -1;
    double largestCof = 0.0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            double absCof = fabs(cof(i,j));

            if (absCof > largestCof)
            {
                largestCof = absCof;
                largestCof_i = i;
                largestCof_j = j;
            }
        }
    }

    if (fabs(largestCof) < doubleThreshold) return cvf::Vec3d::ZERO;

    // Find largest matrix element not in the max cofactor row/col

    int largestMxElm_i = -1; 
    int largestMxElm_j = -1;
    double largestMxElm = 0.0;
    for (int i = 0; i < 3; i++)
    {
        if (i != largestCof_i)
        {
            for (int j = 0; j < 3; j++)
            {
                if (j != largestCof_j)
                {
                    double absMxElm = fabs(mxMinusEigv(i,j));

                    if (absMxElm > largestMxElm)
                    {
                        largestMxElm = absMxElm;
                        largestMxElm_i = i;
                        largestMxElm_j = j;
                    }
                }
            }
        }
    }

    // Check if largest coefficient is zero
    if (fabs(largestMxElm) < doubleThreshold) return cvf::Vec3d::ZERO;

    // Find last component index
    int lastComp_j = 0;
    for (int i = 0; i < 3; i++)
    {
        if ((i != largestCof_j) && (i != largestMxElm_j)) lastComp_j = i;
    }

    cvf::Vec3d eigenVector;
    eigenVector[largestCof_j]   = 1.0;
    eigenVector[lastComp_j]     = cof(largestCof_i, lastComp_j) / cof(largestCof_i, largestCof_j);
    eigenVector[largestMxElm_j] = (-mxMinusEigv(largestMxElm_i, largestCof_j) - mxMinusEigv(largestMxElm_i, lastComp_j)*eigenVector[lastComp_j] ) 
                                  / mxMinusEigv(largestMxElm_i, largestMxElm_j);

    if (computedOk) (*computedOk) = true;

    return eigenVector;
}


}