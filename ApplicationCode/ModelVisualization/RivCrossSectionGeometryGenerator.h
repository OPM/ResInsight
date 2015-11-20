/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
#include "cvfArray.h"

#include <vector>

class RigMainGrid;
class RigResultAccessor;

namespace cvf
{
    class ScalarMapper;
    class DrawableGeo;
}

#include "cvfBoundingBox.h"
namespace cvf {

class CrossSectionHexGridIntf : public Object
{
public:
    virtual Vec3d displayOffset() const = 0;
    virtual BoundingBox boundingBox() const = 0;
    virtual void findIntersectingCells(const BoundingBox& intersectingBB, std::vector<size_t>* intersectedCells) const = 0;
    virtual bool useCell(size_t cellIndex) const = 0;
    virtual void cellCornerVertices(size_t cellIndex, Vec3d cellCorners[8]) const = 0;
    virtual void cellCornerIndices(size_t cellIndex, size_t cornerIndices[8]) const = 0;
}; 

}

class EclipseCrossSectionGrid : public cvf::CrossSectionHexGridIntf
{
public:
    EclipseCrossSectionGrid(const RigMainGrid * mainGrid);
    
    virtual cvf::Vec3d displayOffset() const;
    virtual cvf::BoundingBox boundingBox() const;
    virtual void findIntersectingCells(const cvf::BoundingBox& intersectingBB, std::vector<size_t>* intersectedCells) const;
    virtual bool useCell(size_t cellIndex) const;
    virtual void cellCornerVertices(size_t cellIndex, cvf::Vec3d cellCorners[8]) const;
    virtual void cellCornerIndices(size_t cellIndex, size_t cornerIndices[8]) const;

private:
    cvf::cref<RigMainGrid>      m_mainGrid;
};

class RivVertexWeights
{
public:
    explicit RivVertexWeights(size_t edge1Vx1, size_t edge1Vx2, double normDistFromE1V1,
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

    explicit RivVertexWeights(size_t edge1Vx1, size_t edge1Vx2, double normDistFromE1V1) : m_count(2)
    {
        m_vxIds[0] = (edge1Vx1);
        m_vxIds[1] = (edge1Vx2);

        m_weights[0] = ((float)(1.0 - normDistFromE1V1));
        m_weights[1] = ((float)(normDistFromE1V1));
    }

    int     size() const            { return m_count;}
    size_t  vxId(int idx) const     { return m_vxIds[idx];}
    float   weight(int idx)const    { return m_weights[idx];}
    
private:

    size_t m_vxIds[4];
    float  m_weights[4];
    int    m_count;
};

class RivCrossSectionGeometryGenerator : public cvf::Object
{
public:
    RivCrossSectionGeometryGenerator(const std::vector<cvf::Vec3d> &polyline, 
                                     const cvf::Vec3d& extrusionDirection, 
                                     const RigMainGrid* grid );

    ~RivCrossSectionGeometryGenerator();


    void                        textureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                   const RigResultAccessor* resultAccessor,
                                                   const cvf::ScalarMapper* mapper) const;

    // Mapping between cells and geometry
    const std::vector<size_t>&  triangleToCellIndex() const;
    const std::vector<RivVertexWeights>& triangleVxToCellCornerInterpolationWeights() const;

    // Generated geometry
    cvf::ref<cvf::DrawableGeo>  generateSurface();
    cvf::ref<cvf::DrawableGeo>  createMeshDrawable();

private:
    void                        calculateArrays();



    void                        adjustPolyline();

    cvf::cref<RigMainGrid>      m_mainGrid;
    std::vector<cvf::Vec3d>     m_polyLine;
    cvf::Vec3d                  m_extrusionDirection;
    std::vector<cvf::Vec3d>     m_adjustedPolyline;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>   m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>   m_cellBorderLineVxes;
    std::vector<size_t>         m_triangleToCellIdxMap;

    std::vector<RivVertexWeights> m_triVxToCellCornerWeights;
};

