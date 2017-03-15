/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfBoundingBox.h"

#include <vector>

class RigActiveCellInfo;
class RigFemPart;
class RigMainGrid;


//--------------------------------------------------------------------------------------------------
/// Interface definition used to compute the geometry for planes intersecting a grid
//--------------------------------------------------------------------------------------------------
class RivIntersectionHexGridInterface : public cvf::Object
{
public:
    virtual cvf::Vec3d displayOffset() const = 0;
    virtual cvf::BoundingBox boundingBox() const = 0;
    virtual void findIntersectingCells(const cvf::BoundingBox& intersectingBB, std::vector<size_t>* intersectedCells) const = 0;
    virtual bool useCell(size_t cellIndex) const = 0;
    virtual void cellCornerVertices(size_t cellIndex, cvf::Vec3d cellCorners[8]) const = 0;
    virtual void cellCornerIndices(size_t cellIndex, size_t cornerIndices[8]) const = 0;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivEclipseIntersectionGrid : public RivIntersectionHexGridInterface
{
public:
    RivEclipseIntersectionGrid(const RigMainGrid * mainGrid, const RigActiveCellInfo* activeCellInfo, bool showInactiveCells);

    virtual cvf::Vec3d displayOffset() const;
    virtual cvf::BoundingBox boundingBox() const;
    virtual void findIntersectingCells(const cvf::BoundingBox& intersectingBB, std::vector<size_t>* intersectedCells) const;
    virtual bool useCell(size_t cellIndex) const;
    virtual void cellCornerVertices(size_t cellIndex, cvf::Vec3d cellCorners[8]) const;
    virtual void cellCornerIndices(size_t cellIndex, size_t cornerIndices[8]) const;

private:
    cvf::cref<RigMainGrid>       m_mainGrid;
    cvf::cref<RigActiveCellInfo> m_activeCellInfo;
    bool m_showInactiveCells;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivFemIntersectionGrid : public RivIntersectionHexGridInterface
{
public:
    explicit RivFemIntersectionGrid(const RigFemPart * femPart);

    virtual cvf::Vec3d displayOffset() const;
    virtual cvf::BoundingBox boundingBox() const;
    virtual void findIntersectingCells(const cvf::BoundingBox& intersectingBB, std::vector<size_t>* intersectedCells) const;
    virtual bool useCell(size_t cellIndex) const;
    virtual void cellCornerVertices(size_t cellIndex, cvf::Vec3d cellCorners[8]) const;
    virtual void cellCornerIndices(size_t cellIndex, size_t cornerIndices[8]) const;

private:
    cvf::cref<RigFemPart>      m_femPart;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivIntersectionVertexWeights
{
public:
    explicit RivIntersectionVertexWeights(): m_count(0) {}

/*    
v111 k11 v11   v112  v211 k21  v21    v212
  +------+-----+      +--------+------+
         |                     |        
         |k1                   |k2 
         |   k                 |
       v1+--------+------------+v2
         |                     |
         |                     | 
         |                     |
  +------+----+       +--------+-------+
v121 k12 v12  v122   v221 k22  v22   v222

Where the k's are normalized distances along the edge, from the edge start vertex .

This is the interpolation sceme:

v   = (1 - k  )* v1   + k *v2;

v1  = (1 - k1 )* v11  + k1*v12;
v2  = (1 - k2 )* v21  + k2*v22;

v11 = (1 - k11)* v111 + k11*v112;
v12 = (1 - k12)* v121 + k12*v122;
v21 = (1 - k21)* v211 + k21*v212;
v22 = (1 - k22)* v221 + k22*v222;

Substitution and reorganizing gives the expressions seen below.

*/

    explicit RivIntersectionVertexWeights( size_t edgeVx111, size_t edgeVx112, double k11,
                                           size_t edgeVx121, size_t edgeVx122, double k12,
                                           size_t edgeVx211, size_t edgeVx212, double k21,
                                           size_t edgeVx221, size_t edgeVx222, double k22,
                                           double k1,
                                           double k2,
                                           double k) : m_count(8)
    {
        m_vxIds[0] = (edgeVx111);
        m_vxIds[1] = (edgeVx112);
        m_vxIds[2] = (edgeVx121);
        m_vxIds[3] = (edgeVx122);
        m_vxIds[4] = (edgeVx211);
        m_vxIds[5] = (edgeVx212);
        m_vxIds[6] = (edgeVx221);
        m_vxIds[7] = (edgeVx222);

        m_weights[7] = ((float)(k * k2 * k22));
        m_weights[6] = ((float)(k * k2 - k * k2 * k22));
        m_weights[5] = ((float)(( k - k * k2 ) * k21));
        m_weights[4] = ((float)((k * k2 - k ) * k21 - k * k2 + k));
        m_weights[3] = ((float)((1-k) * k1 * k12));
        m_weights[2] = ((float)((k-1) * k1 * k12 + ( 1 - k ) * k1));
        m_weights[1] = ((float)(( (k-1) * k1 - k + 1 ) * k11));
        m_weights[0] = ((float)(( (1-k) * k1 + k - 1 ) * k11 + ( k - 1 ) * k1 - k + 1));
    }

    explicit RivIntersectionVertexWeights(size_t edge1Vx1, size_t edge1Vx2, double normDistFromE1V1,
                                          size_t edge2Vx1, size_t edge2Vx2, double normDistFromE2V1,
                                          double normDistFromE1Cut) : m_count(4)
    {
        m_vxIds[0] = (edge1Vx1);
        m_vxIds[1] = (edge1Vx2);
        m_vxIds[2] = (edge2Vx1);
        m_vxIds[3] = (edge2Vx2);

        m_weights[0] = ((float)(1.0 - normDistFromE1V1 - normDistFromE1Cut + normDistFromE1V1*normDistFromE1Cut));
        m_weights[1] = ((float)(normDistFromE1V1 - normDistFromE1V1*normDistFromE1Cut));
        m_weights[2] = ((float)(normDistFromE1Cut - normDistFromE2V1*normDistFromE1Cut));
        m_weights[3] = ((float)(normDistFromE2V1*normDistFromE1Cut));
    }

    explicit RivIntersectionVertexWeights(size_t edge1Vx1, size_t edge1Vx2, double normDistFromE1V1) : m_count(2)
    {
        m_vxIds[0] = (edge1Vx1);
        m_vxIds[1] = (edge1Vx2);

        m_weights[0] = ((float)(1.0 - normDistFromE1V1));
        m_weights[1] = ((float)(normDistFromE1V1));
    }

    int     size() const         { return m_count; }
    size_t  vxId(int idx) const  { return m_vxIds[idx]; }
    float   weight(int idx)const { return m_weights[idx]; }

private:

    size_t m_vxIds[8];
    float  m_weights[8];
    int    m_count;
};
