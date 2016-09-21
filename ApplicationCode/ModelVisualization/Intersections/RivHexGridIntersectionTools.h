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
    RivFemIntersectionGrid(const RigFemPart * femPart);

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

    size_t m_vxIds[4];
    float  m_weights[4];
    int    m_count;
};
