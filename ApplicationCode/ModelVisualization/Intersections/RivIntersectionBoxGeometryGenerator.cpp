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

#include "RivIntersectionBoxGeometryGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPlane.h"
#include <array>
#include "RimIntersectionBox.h"
#include "cvfStructGrid.h"
#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxGeometryGenerator::RivIntersectionBoxGeometryGenerator(const RimIntersectionBox* intersectionBox, const RivIntersectionHexGridInterface* grid)
    : m_intersectionBoxDefinition(intersectionBox),
    m_hexGrid(grid)
{
    m_triangleVxes = new cvf::Vec3fArray;
    m_cellBorderLineVxes = new cvf::Vec3fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxGeometryGenerator::~RivIntersectionBoxGeometryGenerator()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivIntersectionBoxGeometryGenerator::isAnyGeometryPresent() const
{
    if (m_triangleVxes->size() == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionBoxGeometryGenerator::generateSurface()
{
    calculateArrays();

    CVF_ASSERT(m_triangleVxes.notNull());

    if (m_triangleVxes->size() == 0) return NULL;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromTriangleVertexArray(m_triangleVxes.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionBoxGeometryGenerator::createMeshDrawable()
{
    if (!(m_cellBorderLineVxes.notNull() && m_cellBorderLineVxes->size() != 0)) return NULL;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray(m_cellBorderLineVxes.p());


    cvf::ref<cvf::PrimitiveSetDirect> prim = new cvf::PrimitiveSetDirect(cvf::PT_LINES);
    prim->setIndexCount(m_cellBorderLineVxes->size());

    geo->addPrimitiveSet(prim.p());
    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivIntersectionBoxGeometryGenerator::triangleToCellIndex() const
{
    CVF_ASSERT(m_triangleVxes->size());
    return m_triangleToCellIdxMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RivIntersectionVertexWeights>& RivIntersectionBoxGeometryGenerator::triangleVxToCellCornerInterpolationWeights() const
{
    CVF_ASSERT(m_triangleVxes->size());
    return m_triVxToCellCornerWeights;
}


class Box
{
public:
    using FaceType = cvf::StructGridInterface::FaceType;

    Box(const cvf::Mat4d& origin, const cvf::Vec3d& size) : m_origin(origin), m_size(size) {}

    std::array<cvf::Plane, 6> planes()
    {
        std::array<cvf::Plane, 6> boxPlanes;

        boxPlanes[FaceType::POS_I].setFromPointAndNormal(cvf::Vec3d(m_origin.col(0)), m_origin.translation() + m_size);
        boxPlanes[FaceType::NEG_I].setFromPointAndNormal(-cvf::Vec3d(m_origin.col(0)), m_origin.translation());
        boxPlanes[FaceType::POS_J].setFromPointAndNormal(cvf::Vec3d(m_origin.col(1)), m_origin.translation() + m_size);
        boxPlanes[FaceType::NEG_J].setFromPointAndNormal(-cvf::Vec3d(m_origin.col(1)), m_origin.translation());
        boxPlanes[FaceType::POS_K].setFromPointAndNormal(cvf::Vec3d(m_origin.col(2)), m_origin.translation() + m_size);
        boxPlanes[FaceType::NEG_K].setFromPointAndNormal(-cvf::Vec3d(m_origin.col(2)), m_origin.translation());

        return boxPlanes;
    }

    std::array<cvf::Vec3d, 4> faceCorners(FaceType face)
    {
        std::array<cvf::Vec3d, 4> corners;

        cvf::Vec3d sx = cvf::Vec3d(m_origin.col(0)) * m_size[0] ;
        cvf::Vec3d sy = cvf::Vec3d(m_origin.col(1)) * m_size[1] ;
        cvf::Vec3d sz = cvf::Vec3d(m_origin.col(2)) * m_size[2] ;

        switch(face)
        {
            case FaceType::POS_I:
            corners[0] = m_origin.translation() + sx;
            corners[1] = m_origin.translation() + sx + sy;
            corners[2] = m_origin.translation() + sx + sy + sz;
            corners[3] = m_origin.translation() + sx      + sz;
            break;
            case FaceType::NEG_I:
            corners[0] = m_origin.translation();
            corners[1] = m_origin.translation() + sz;
            corners[2] = m_origin.translation() + sy + sz;
            corners[3] = m_origin.translation() + sy;
            break;
            case FaceType::POS_J:
            corners[0] = m_origin.translation() + sy;
            corners[1] = m_origin.translation() + sy      + sz;
            corners[2] = m_origin.translation() + sy + sx + sz;
            corners[3] = m_origin.translation() + sy + sx;
            break;
            case FaceType::NEG_J:
            corners[0] = m_origin.translation() ;
            corners[1] = m_origin.translation()  + sx;
            corners[2] = m_origin.translation()  + sx + sz;
            corners[3] = m_origin.translation()       + sz;
            break;
            case FaceType::POS_K:
            corners[0] = m_origin.translation() + sz;
            corners[1] = m_origin.translation() + sz      + sx;
            corners[2] = m_origin.translation() + sz + sx + sy;
            corners[3] = m_origin.translation() + sz + sy;
            break;
            case FaceType::NEG_K:
            corners[0] = m_origin.translation();
            corners[1] = m_origin.translation() + sy;
            corners[2] = m_origin.translation() + sx + sy;
            corners[3] = m_origin.translation()      + sx;
            break;
        }

        return corners;
    }

private:

    cvf::Mat4d m_origin;
    cvf::Vec3d m_size;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimIntersectionBox* RivIntersectionBoxGeometryGenerator::intersectionBox() const
{
    return m_intersectionBoxDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxGeometryGenerator::calculateArrays()
{
    if(m_triangleVxes->size()) return;

    std::vector<cvf::Vec3f> triangleVertices;
    std::vector<cvf::Vec3f> cellBorderLineVxes;
    cvf::Vec3d displayOffset = m_hexGrid->displayOffset();
    cvf::BoundingBox gridBBox = m_hexGrid->boundingBox();

    Box box(m_intersectionBoxDefinition->boxOrigin(), m_intersectionBoxDefinition->boxSize());
    std::array<cvf::Plane, 6> boxPlanes = box.planes();

    for (int faceIdx = 0; faceIdx < 6; ++faceIdx)    
    {
        cvf::Plane plane = boxPlanes[faceIdx];
        int clipPlaneIdx = faceIdx;
        clipPlaneIdx++; if (clipPlaneIdx >= 6) clipPlaneIdx = 0; // Skip the opposite face
        clipPlaneIdx++; if (clipPlaneIdx >= 6) clipPlaneIdx = 0;
        cvf::Plane p1Plane = boxPlanes[faceIdx + 2];
        clipPlaneIdx++; if (clipPlaneIdx >= 6) clipPlaneIdx = 0;
        cvf::Plane p2Plane = boxPlanes[faceIdx + 3];
        clipPlaneIdx++; if (clipPlaneIdx >= 6) clipPlaneIdx = 0;
        cvf::Plane p3Plane = boxPlanes[faceIdx + 4];
        clipPlaneIdx++; if (clipPlaneIdx >= 6) clipPlaneIdx = 0;
        cvf::Plane p4Plane = boxPlanes[faceIdx + 5];

        std::array<cvf::Vec3d, 4> faceCorners = box.faceCorners((Box::FaceType)faceIdx);
        cvf::BoundingBox sideBBox;
        
        for (cvf::Vec3d& corner : faceCorners) sideBBox.add(corner);



        std::vector<size_t> columnCellCandidates;
        m_hexGrid->findIntersectingCells(sideBBox, &columnCellCandidates);

        // Same code as IntersectionGenerator :

        std::vector<caf::HexGridIntersectionTools::ClipVx> hexPlaneCutTriangleVxes;
        hexPlaneCutTriangleVxes.reserve(5*3);
        std::vector<bool> isTriangleEdgeCellContour;
        isTriangleEdgeCellContour.reserve(5*3);
        cvf::Vec3d cellCorners[8];
        size_t cornerIndices[8];

        for(size_t cccIdx = 0; cccIdx < columnCellCandidates.size(); ++cccIdx)
        {
            size_t globalCellIdx = columnCellCandidates[cccIdx];

            if(!m_hexGrid->useCell(globalCellIdx)) continue;

            hexPlaneCutTriangleVxes.clear();
            m_hexGrid->cellCornerVertices(globalCellIdx, cellCorners);
            m_hexGrid->cellCornerIndices(globalCellIdx, cornerIndices);

            caf::HexGridIntersectionTools::planeHexIntersectionMC(plane,
                                                                  cellCorners,
                                                                  cornerIndices,
                                                                  &hexPlaneCutTriangleVxes,
                                                                  &isTriangleEdgeCellContour);

            std::vector<caf::HexGridIntersectionTools::ClipVx> clippedTriangleVxes;
            std::vector<bool> isClippedTriEdgeCellContour;

            caf::HexGridIntersectionTools::clipTrianglesBetweenTwoParallelPlanes(hexPlaneCutTriangleVxes, isTriangleEdgeCellContour, p1Plane, p2Plane,
                                                                                 &clippedTriangleVxes, &isClippedTriEdgeCellContour);

            size_t clippedTriangleCount = clippedTriangleVxes.size()/3;

            for(uint tIdx = 0; tIdx < clippedTriangleCount; ++tIdx)
            {
                uint triVxIdx = tIdx*3;

                // Accumulate triangle vertices

                cvf::Vec3f p0(clippedTriangleVxes[triVxIdx+0].vx - displayOffset);
                cvf::Vec3f p1(clippedTriangleVxes[triVxIdx+1].vx - displayOffset);
                cvf::Vec3f p2(clippedTriangleVxes[triVxIdx+2].vx - displayOffset);

                triangleVertices.push_back(p0);
                triangleVertices.push_back(p1);
                triangleVertices.push_back(p2);


                // Accumulate mesh lines

                if(isClippedTriEdgeCellContour[triVxIdx])
                {
                    cellBorderLineVxes.push_back(p0);
                    cellBorderLineVxes.push_back(p1);
                }
                if(isClippedTriEdgeCellContour[triVxIdx+1])
                {
                    cellBorderLineVxes.push_back(p1);
                    cellBorderLineVxes.push_back(p2);
                }
                if(isClippedTriEdgeCellContour[triVxIdx+2])
                {
                    cellBorderLineVxes.push_back(p2);
                    cellBorderLineVxes.push_back(p0);
                }

                // Mapping to cell index

                m_triangleToCellIdxMap.push_back(globalCellIdx);

                // Interpolation from nodes
                for(int i = 0; i < 3; ++i)
                {
                    caf::HexGridIntersectionTools::ClipVx cvx = clippedTriangleVxes[triVxIdx + i];
                    if(cvx.isVxIdsNative)
                    {
                        m_triVxToCellCornerWeights.push_back(
                            RivIntersectionVertexWeights(cvx.clippedEdgeVx1Id, cvx.clippedEdgeVx2Id, cvx.normDistFromEdgeVx1));
                    }
                    else
                    {
                        caf::HexGridIntersectionTools::ClipVx cvx1 = hexPlaneCutTriangleVxes[cvx.clippedEdgeVx1Id];
                        caf::HexGridIntersectionTools::ClipVx cvx2 = hexPlaneCutTriangleVxes[cvx.clippedEdgeVx2Id];

                        m_triVxToCellCornerWeights.push_back(
                            RivIntersectionVertexWeights(cvx1.clippedEdgeVx1Id, cvx1.clippedEdgeVx2Id, cvx1.normDistFromEdgeVx1,
                                                         cvx2.clippedEdgeVx1Id, cvx2.clippedEdgeVx2Id, cvx2.normDistFromEdgeVx1,
                                                         cvx.normDistFromEdgeVx1));

                    }
                }
            }
        }
    }
    m_cellBorderLineVxes->assign(cellBorderLineVxes);
    m_triangleVxes->assign(triangleVertices);
}
    


