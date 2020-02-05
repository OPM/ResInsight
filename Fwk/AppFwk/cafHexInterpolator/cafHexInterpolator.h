//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
/*
Interpolating inside a general 8 node hexahedral element
Calculating an interpolated value at a position inside the element from values at each corner.
Author Jacob Støren 

                  | v1 |
Vectors:    [v] = | v2 | = { v1, v2, v3 }
                  | v3 |

the {} means that the elment list are a standing vector, but written as a row for convenience
[] means matrix or vector quantity

// HEX8
//     7---------6     
//    /|        /|     |k, z, 
//   / |       / |     | /j, y
//  4---------5  |     |/
//  |  3------|--2     *---i, x, 
//  | /       | /
//  |/        |/
//  0---------1     

Normalized coordinates i, j, k
Normalized origo in center of element
Normalized cell corners NCn in :
 
[NC0] = {-1,-1,-1}
[NC1] = { 1,-1,-1}
[NC2] = { 1, 1,-1}
[NC3] = {-1, 1,-1}
[NC4] = {-1,-1, 1}
[NC5] = { 1,-1, 1}
[NC6] = { 1, 1, 1}
[NC7] = {-1, 1, 1}

Thus Interpolation polynomials (follows sign of corner position) Nn = 

N0 =  1/8 *(1-i)(1-j)(1-k) =  1/8 * ( 1 - i - j - k + ij + ik + jk - ijk )
N1 =  1/8 *(1+i)(1-j)(1-k) =  1/8 * ( 1 + i - j - k - ij - ik + jk + ijk ) 
N2 =  1/8 *(1+i)(1+j)(1-k) =  1/8 * ( 1 + i + j - k + ij - ik - jk - ijk )  
N3 =  1/8 *(1-i)(1+j)(1-k) =  1/8 * ( 1 - i + j - k - ij + ik - jk + ijk )  
N4 =  1/8 *(1-i)(1-j)(1+k) =  1/8 * ( 1 - i - j + k + ij - ik - jk + ijk )
N5 =  1/8 *(1+i)(1-j)(1+k) =  1/8 * ( 1 + i - j + k - ij + ik - jk - ijk ) 
N6 =  1/8 *(1+i)(1+j)(1+k) =  1/8 * ( 1 + i + j + k + ij + ik + jk + ijk )  
N7 =  1/8 *(1-i)(1+j)(1+k) =  1/8 * ( 1 - i + j + k - ij - ik + jk - ijk )  

To calculate an interpolated value R at position { i, j, k } from values Vn in each corner of the cell
we use i,j,k to calculate all Nn, and then :

R = sum_n(Nn * Vn)

A point [P] = {x,y,z} inside a general element can then be calculated by the {i,j,k} coordinates correponding to the point and the corner positions Cn = {Cx_n, Cy_n, Cz_n}  (n = 0..7) of the general hex as:

x = sum_n(Nn * Cn_x)
y = sum_n(Nn * Cn_y)
z = sum_n(Nn * Cn_z)

If we define 

[Cn] = | [C0] ... [C7] |

Where 
       | Cx_0 |               | Cx_7 |
[C0] = | Cy_0 |, ... , [C7] = | Cy_7 |
       | Cz_0 |               | Cz_7 |
And
       | N0 | 
[Nn] = | .. |
       | N7 |

it can be written

[P] = [Cn]*[Nn]

As seen, we need to know the { i, j, k} position of the point. This can only be calculated using some sort of iterative solution, eg. Newton-Rapson. 
Using m as iteration number we can use { im, jm, km } to evaluate [Nn] and name it [Nnm]. 
We use an initial guess of  {im,jm,km} = { 0.5, 0.5, 0.5} // But why ? the element origo is at 0,0,0 !

[Pm] = {xm, ym, zm}  =  sum_n(Nnm * Cn_x), sum_n(Nnm * Cn_y), sum_n(Nnm * Cn_z)  }   When using im,jm,km to evaluate Nnm

P is the position we want to calculate i,j,k for.
So our function to solve for roots become F = Pm - P = 0

Then multidimensional Newton-Rapson says:

{im, jm, km}_new = {im,jm,km}  - [J-1]_m * ( [Pm] - [P] ) }

Where [J-1] is the inverse Jacobian giving the change in i,j,k when varying x,y,z (The partial derivatives)

The Jacobian is the partially derived x, y, z with respect to i, j, k as follows:
( The organization of the Jacobi is not quite concistent in the litterature. 
  It seems as if NTNU uses the following, while others use the Transpose.
  (Seems as if the correct one to use in the newton-rapson iteration is the transpose of the following: )

    | dx/di dy/di dz/di |
J = | dx/dj dy/dj dz/dj | 
    | dx/dk dy/dk dz/dk |
	
The partial derivatives are calculated by deriving the function [F(i,j,k)] = [Pm(i,j,k)] - [P]
Since [P] is constant, this is the same as deriving [Pm(i,j,k)] = [Cn]*[Nnm]
Explicitly:

       |xm|   | N0m*C0_x + ... + N7m*C7_x |
[Pm] = |ym| = | N0m*C0_y + ... + N7m*C7_y |
       |zm|   | N0m*C0_z + ... + N7m*C7_z |


In the following we remove the iteration index m for simplicity 
The elements of the Jacobi can be calculated by partially deriving the polynomials for each of the components x, y, z

dx/di = (dN0/di)*C0_x + ... + (dN7/di)*C7_x
dx/dj = (dN0/dj)*C0_x + ... + (dN7/dj)*C7_x
dx/dk = (dN0/dk)*C0_x + ... + (dN7/dk)*C7_x

dy/di = (dN0/di)*C0_y + ... + (dN7/di)*C7_y
dy/dj = (dN0/dj)*C0_y + ... + (dN7/dj)*C7_y
dy/dk = (dN0/dk)*C0_y + ... + (dN7/dk)*C7_y

dz/di = (dN0/di)*C0_z + ... + (dN7/di)*C7_z
dz/dj = (dN0/dj)*C0_z + ... + (dN7/dj)*C7_z
dz/dk = (dN0/dk)*C0_z + ... + (dN7/dk)*C7_z

Which can be written in more compact form:
| dx/di |
| dy/di | = [Cn]*[dNn/di]
| dz/di |

, etc.


And in detail every partial derivative of the polynomials:

dN0/di = 1/8 * ( - 1 + j + k - jk )
dN1/di = 1/8 * ( + 1 - j - k + jk )
dN2/di = 1/8 * ( + i + j - k - jk )
dN3/di = 1/8 * ( - 1 - j + k + jk )
dN4/di = 1/8 * ( - 1 + j - k + jk )
dN5/di = 1/8 * ( + 1 - j + k - jk )
dN6/di = 1/8 * ( + 1 + j + k + jk )
dN7/di = 1/8 * ( - 1 - j - k - jk )

dN0/dj = 1/8 * ( - 1 + i + k - ik )
dN1/dj = 1/8 * ( - 1 - i + k + ik )
dN2/dj = 1/8 * ( + 1 + i - k - ik )
dN3/dj = 1/8 * ( + 1 - i - k + ik )
dN4/dj = 1/8 * ( - 1 + i - k + ik )
dN5/dj = 1/8 * ( - 1 - i - k - ik )
dN6/dj = 1/8 * ( + 1 + i + k + ik )
dN7/dj = 1/8 * ( + 1 - i + k - ik )

dN0/dk = 1/8 * (  - 1 + i + j - ij )
dN1/dk = 1/8 * (  - 1 - i + j + ij )
dN2/dk = 1/8 * (  - 1 - i - j - ij )
dN3/dk = 1/8 * (  - 1 + i - j + ij )
dN4/dk = 1/8 * (  + 1 - i - j + ij )
dN5/dk = 1/8 * (  + 1 + i - j - ij )
dN6/dk = 1/8 * (  + 1 + i + j + ij )
dN7/dk = 1/8 * (  + 1 - i + j - ij )

The inverse Jacobian can be calculated by inverting the resulting Jacobian matrix for a specifically chosen set of i,j,k
*/

#include "cvfVector3.h"
#include "cvfMatrix3.h"

#include <array>

namespace caf {

class HexInterpolator
{
public:
    static double interpolateHex(const std::array<cvf::Vec3d, 8>& hexCorners, 
                                 const std::array<double, 8>& values,
                                 const cvf::Vec3d& point)
    {
        cvf::Vec3d pointInNormElm = findNormalizedCoords(hexCorners, point);
        return interpolateInNormElm( pointInNormElm, values);
    }

    static std::array<double, 8> vertexWeights(const std::array<cvf::Vec3d, 8>& hexCorners, 
                                               const cvf::Vec3d& point )
    {
        cvf::Vec3d pointInNormElm = findNormalizedCoords(hexCorners, point);
        return interpolationCoeffs(pointInNormElm);
    }


    static cvf::Mat3d jacobi(const std::array<cvf::Vec3d, 8>& hexCorners, const cvf::Vec3d & pointInNormalizedElement) 
    {
        const double k_1_8 = 0.125;

        double i = pointInNormalizedElement[0];
        double j = pointInNormalizedElement[1];
        double k = pointInNormalizedElement[2];

        double ij = i*j;
        double ik = i*k;
        double jk = j*k;

        double C0_x = hexCorners[0][0]; double C0_y = hexCorners[0][1]; double C0_z = hexCorners[0][2];
        double C1_x = hexCorners[1][0]; double C1_y = hexCorners[1][1]; double C1_z = hexCorners[1][2];
        double C2_x = hexCorners[2][0]; double C2_y = hexCorners[2][1]; double C2_z = hexCorners[2][2];
        double C3_x = hexCorners[3][0]; double C3_y = hexCorners[3][1]; double C3_z = hexCorners[3][2];
        double C4_x = hexCorners[4][0]; double C4_y = hexCorners[4][1]; double C4_z = hexCorners[4][2];
        double C5_x = hexCorners[5][0]; double C5_y = hexCorners[5][1]; double C5_z = hexCorners[5][2];
        double C6_x = hexCorners[6][0]; double C6_y = hexCorners[6][1]; double C6_z = hexCorners[6][2];
        double C7_x = hexCorners[7][0]; double C7_y = hexCorners[7][1]; double C7_z = hexCorners[7][2];

        double dN0_di = (- 1 + j + k - jk);
        double dN1_di = (+ 1 - j - k + jk);
        double dN2_di = (+ 1 + j - k - jk);
        double dN3_di = (- 1 - j + k + jk);
        double dN4_di = (- 1 + j - k + jk);
        double dN5_di = (+ 1 - j + k - jk);
        double dN6_di = (+ 1 + j + k + jk);
        double dN7_di = (- 1 - j - k - jk);

        double dN0_dj = (- 1 + i + k - ik);
        double dN1_dj = (- 1 - i + k + ik);
        double dN2_dj = (+ 1 + i - k - ik);
        double dN3_dj = (+ 1 - i - k + ik);
        double dN4_dj = (- 1 + i - k + ik);
        double dN5_dj = (- 1 - i - k - ik);
        double dN6_dj = (+ 1 + i + k + ik);
        double dN7_dj = (+ 1 - i + k - ik);

        double dN0_dk = (- 1 + i + j - ij);
        double dN1_dk = (- 1 - i + j + ij);
        double dN2_dk = (- 1 - i - j - ij);
        double dN3_dk = (- 1 + i - j + ij);
        double dN4_dk = (+ 1 - i - j + ij);
        double dN5_dk = (+ 1 + i - j - ij);
        double dN6_dk = (+ 1 + i + j + ij);
        double dN7_dk = (+ 1 - i + j - ij);

        double dx_di = k_1_8 * ( (dN0_di) * C0_x + (dN1_di) * C1_x + (dN2_di) * C2_x + (dN3_di) * C3_x + (dN4_di) * C4_x + (dN5_di) * C5_x + (dN6_di) * C6_x  + (dN7_di) * C7_x );
        double dx_dj = k_1_8 * ( (dN0_dj) * C0_x + (dN1_dj) * C1_x + (dN2_dj) * C2_x + (dN3_dj) * C3_x + (dN4_dj) * C4_x + (dN5_dj) * C5_x + (dN6_dj) * C6_x  + (dN7_dj) * C7_x );
        double dx_dk = k_1_8 * ( (dN0_dk) * C0_x + (dN1_dk) * C1_x + (dN2_dk) * C2_x + (dN3_dk) * C3_x + (dN4_dk) * C4_x + (dN5_dk) * C5_x + (dN6_dk) * C6_x  + (dN7_dk) * C7_x );

        double dy_di = k_1_8 * ( (dN0_di) * C0_y + (dN1_di) * C1_y + (dN2_di) * C2_y + (dN3_di) * C3_y + (dN4_di) * C4_y + (dN5_di) * C5_y + (dN6_di) * C6_y  + (dN7_di) * C7_y );
        double dy_dj = k_1_8 * ( (dN0_dj) * C0_y + (dN1_dj) * C1_y + (dN2_dj) * C2_y + (dN3_dj) * C3_y + (dN4_dj) * C4_y + (dN5_dj) * C5_y + (dN6_dj) * C6_y  + (dN7_dj) * C7_y );
        double dy_dk = k_1_8 * ( (dN0_dk) * C0_y + (dN1_dk) * C1_y + (dN2_dk) * C2_y + (dN3_dk) * C3_y + (dN4_dk) * C4_y + (dN5_dk) * C5_y + (dN6_dk) * C6_y  + (dN7_dk) * C7_y );
        
        double dz_di = k_1_8 * ( (dN0_di) * C0_z + (dN1_di) * C1_z + (dN2_di) * C2_z + (dN3_di) * C3_z + (dN4_di) * C4_z + (dN5_di) * C5_z + (dN6_di) * C6_z  + (dN7_di) * C7_z );
        double dz_dj = k_1_8 * ( (dN0_dj) * C0_z + (dN1_dj) * C1_z + (dN2_dj) * C2_z + (dN3_dj) * C3_z + (dN4_dj) * C4_z + (dN5_dj) * C5_z + (dN6_dj) * C6_z  + (dN7_dj) * C7_z );
        double dz_dk = k_1_8 * ( (dN0_dk) * C0_z + (dN1_dk) * C1_z + (dN2_dk) * C2_z + (dN3_dk) * C3_z + (dN4_dk) * C4_z + (dN5_dk) * C5_z + (dN6_dk) * C6_z  + (dN7_dk) * C7_z );
    
        // Do not know which ordering ends up as correct

        #if 1 // Use literature jacobi ordering
        return cvf::Mat3d(
            dx_di, dx_dj, dx_dk,
            dy_di, dy_dj, dy_dk,
            dz_di, dz_dj, dz_dk
            );
        #else // use NTNU ordering
        return cvf::Mat3d(
            dx_di, dy_di, dz_di,
            dx_dj, dy_dj, dz_dj,
            dx_dk, dy_dk, dz_dk
            );
        #endif
    }

    
private:
    
    static double interpolateInNormElm( const cvf::Vec3d & pointInNormElm, const std::array<double, 8>& values)
    {
        std::array<double, 8> Ni = interpolationCoeffs(pointInNormElm);

        double result = 0.0;
        
        for (int i = 0 ; i < 8 ; ++i )
        {
            result += values[i]*Ni[i];
        }

        return result;
    }

    static cvf::Vec3d interpolateInNormElm( const cvf::Vec3d & pointInNormElm, const std::array<cvf::Vec3d, 8>& values)
    {
        std::array<double, 8> Ni = interpolationCoeffs(pointInNormElm);

        cvf::Vec3d result(cvf::Vec3d::ZERO);

        for (int i = 0 ; i < 8 ; ++i )
        {
            result += values[i]*Ni[i];
        }

        return result;
    }

    static std::array<double, 8> interpolationCoeffs( const cvf::Vec3d & pointInNormalizedElement)
    {
        const double k_1_8 = 0.125;

        double i = pointInNormalizedElement[0];
        double j = pointInNormalizedElement[1];
        double k = pointInNormalizedElement[2];

        // could use as optimization ... >> double oni = 1 - i;
        // could use as optimization ... >> double opi = 1 + i;
        // could use as optimization ... >> double onj = 1 - j;
        // could use as optimization ... >> double opj = 1 + j;
        // could use as optimization ... >> double onk = 1 - k;
        // could use as optimization ... >> double opk = 1 + k;
        
        return  {
            k_1_8 *(1-i)*(1-j)*(1-k),
            k_1_8 *(1+i)*(1-j)*(1-k),
            k_1_8 *(1+i)*(1+j)*(1-k),
            k_1_8 *(1-i)*(1+j)*(1-k),
            k_1_8 *(1-i)*(1-j)*(1+k),
            k_1_8 *(1+i)*(1-j)*(1+k),
            k_1_8 *(1+i)*(1+j)*(1+k),
            k_1_8 *(1-i)*(1+j)*(1+k)
        };
    }

    static cvf::Vec3d findNormalizedCoords(const std::array<cvf::Vec3d, 8>& hexCorners, const cvf::Vec3d& P)
    {
        cvf::Vec3d normPoint = {0.0, 0.0, 0.0}; // Start value
        int m = 0;
        while ( m < 5 )
        {
            cvf::Vec3d Pm = interpolateInNormElm(normPoint, hexCorners);
            cvf::Vec3d Pdiff = Pm - P;
            if (Pdiff.lengthSquared() < 1e-3) break;

            cvf::Mat3d J_inv = jacobi(hexCorners, normPoint) ;
            bool inversionOk = J_inv.invert();
            if ( !inversionOk ) break;

            cvf::Vec3d ijkStep = Pdiff.getTransformedVector(J_inv);

            normPoint = normPoint - ijkStep;

            m++;
        }

        return normPoint;
    }
};

}
