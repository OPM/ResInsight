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
RivIntersectionBoxGeometryGenerator::RivIntersectionBoxGeometryGenerator(RimIntersectionBox* intersectionBox, const RivIntersectionHexGridInterface* grid)
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

    if (m_triangleVxes->size() == 0) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromTriangleVertexArray(m_triangleVxes.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionBoxGeometryGenerator::createMeshDrawable()
{
    if (!(m_cellBorderLineVxes.notNull() && m_cellBorderLineVxes->size() != 0)) return nullptr;

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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivIntersectionBoxGeometryGenerator::triangleVxes() const
{
    CVF_ASSERT(m_triangleVxes->size());

    return m_triangleVxes.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

class Box
{
public:
    using FaceType = cvf::StructGridInterface::FaceType;

    Box(const cvf::Mat4d& origin, const cvf::Vec3d& size) : m_origin(origin), m_size(size) {}

    std::array<cvf::Plane, 6> planes()
    {
        std::array<cvf::Plane, 6> boxPlanes;

        boxPlanes[FaceType::POS_I].setFromPointAndNormal( m_origin.translation() + m_size, cvf::Vec3d(m_origin.col(0)));
        boxPlanes[FaceType::NEG_I].setFromPointAndNormal( m_origin.translation()         , -cvf::Vec3d(m_origin.col(0)));
        boxPlanes[FaceType::POS_J].setFromPointAndNormal( m_origin.translation() + m_size, cvf::Vec3d(m_origin.col(1)));
        boxPlanes[FaceType::NEG_J].setFromPointAndNormal( m_origin.translation()         , -cvf::Vec3d(m_origin.col(1)));
        boxPlanes[FaceType::POS_K].setFromPointAndNormal( m_origin.translation() + m_size, cvf::Vec3d(m_origin.col(2)));
        boxPlanes[FaceType::NEG_K].setFromPointAndNormal( m_origin.translation()         , -cvf::Vec3d(m_origin.col(2)));

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

    // Returns the four adjacent faces in the Pos, Neg, Pos, Neg order

    std::array<FaceType, 4> adjacentFaces(FaceType face) 
    {
        std::array<FaceType, 4> clipFaces;
        FaceType oppFace = cvf::StructGridInterface::oppositeFace(face);
        int clipFaceCount = 0;
        for (int faceCand = 0; faceCand < 6; ++faceCand )
        {
            if (faceCand != face && faceCand != oppFace)
            {
                clipFaces[clipFaceCount] = (FaceType) faceCand;
                clipFaceCount++;
            }
        }

        return clipFaces;
    }

private:

    cvf::Mat4d m_origin;
    cvf::Vec3d m_size;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBox* RivIntersectionBoxGeometryGenerator::intersectionBox() const
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

    RimIntersectionBox::SinglePlaneState singlePlane = m_intersectionBoxDefinition->singlePlaneState();

    int startFace = 0; int endFace = 5;
    if (singlePlane == RimIntersectionBox::PLANE_STATE_X) startFace = endFace = Box::FaceType::POS_I;
    if (singlePlane == RimIntersectionBox::PLANE_STATE_Y) startFace = endFace = Box::FaceType::POS_J;
    if (singlePlane == RimIntersectionBox::PLANE_STATE_Z) startFace = endFace = Box::FaceType::POS_K;

    for (int faceIdx = startFace; faceIdx <= endFace; ++faceIdx)    
    {
        cvf::Plane plane = boxPlanes[faceIdx];
        
        std::array<Box::FaceType, 4> clipFaces = box.adjacentFaces((Box::FaceType)faceIdx);

        cvf::Plane p1Plane = boxPlanes[clipFaces[1]];
        cvf::Plane p2Plane = boxPlanes[clipFaces[0]];
        cvf::Plane p3Plane = boxPlanes[clipFaces[3]];
        cvf::Plane p4Plane = boxPlanes[clipFaces[2]];

        p1Plane.flip();
        p2Plane.flip();
        p3Plane.flip();
        p4Plane.flip();

        std::array<cvf::Vec3d, 4> faceCorners = box.faceCorners((Box::FaceType)faceIdx);

        cvf::BoundingBox sectionBBox;
        
        for (cvf::Vec3d& corner : faceCorners) sectionBBox.add(corner);

        // Similar code as IntersectionGenerator :

        std::vector<size_t> columnCellCandidates;
        m_hexGrid->findIntersectingCells(sectionBBox, &columnCellCandidates);

        std::vector<caf::HexGridIntersectionTools::ClipVx> hexPlaneCutTriangleVxes;
        hexPlaneCutTriangleVxes.reserve(5*3);
        std::vector<int> cellFaceForEachTriangleEdge;
        cellFaceForEachTriangleEdge.reserve(5*3);
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
                                                                  &cellFaceForEachTriangleEdge);

            std::vector<caf::HexGridIntersectionTools::ClipVx> clippedTriangleVxes_once;
            std::vector<int> cellFaceForEachClippedTriangleEdge_once;
            caf::HexGridIntersectionTools::clipTrianglesBetweenTwoParallelPlanes(hexPlaneCutTriangleVxes, cellFaceForEachTriangleEdge, p1Plane, p2Plane,
                                                                                 &clippedTriangleVxes_once, &cellFaceForEachClippedTriangleEdge_once);

            for (caf::HexGridIntersectionTools::ClipVx& clvx : clippedTriangleVxes_once) if (!clvx.isVxIdsNative) clvx.derivedVxLevel = 0;

            std::vector<caf::HexGridIntersectionTools::ClipVx> clippedTriangleVxes;
            std::vector<int> cellFaceForEachClippedTriangleEdge;

            caf::HexGridIntersectionTools::clipTrianglesBetweenTwoParallelPlanes(clippedTriangleVxes_once, cellFaceForEachClippedTriangleEdge_once, p3Plane, p4Plane,
                                                                                 &clippedTriangleVxes, &cellFaceForEachClippedTriangleEdge);
            for (caf::HexGridIntersectionTools::ClipVx& clvx : clippedTriangleVxes) if (!clvx.isVxIdsNative && clvx.derivedVxLevel == -1) clvx.derivedVxLevel = 1;

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
                
                #define isFace( faceEnum ) (0 <= faceEnum && faceEnum <= 5 )

                if(isFace(cellFaceForEachClippedTriangleEdge[triVxIdx]))
                {
                    cellBorderLineVxes.push_back(p0);
                    cellBorderLineVxes.push_back(p1);
                }
                if(isFace(cellFaceForEachClippedTriangleEdge[triVxIdx+1]))
                {
                    cellBorderLineVxes.push_back(p1);
                    cellBorderLineVxes.push_back(p2);
                }
                if(isFace(cellFaceForEachClippedTriangleEdge[triVxIdx+2]))
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
                        caf::HexGridIntersectionTools::ClipVx cvx1;
                        caf::HexGridIntersectionTools::ClipVx cvx2;
                        
                        if (cvx.derivedVxLevel == 0)
                        {
                            cvx1 = hexPlaneCutTriangleVxes[cvx.clippedEdgeVx1Id];
                            cvx2 = hexPlaneCutTriangleVxes[cvx.clippedEdgeVx2Id];
                        }
                        else if(cvx.derivedVxLevel == 1)
                        {
                            cvx1 = clippedTriangleVxes_once[cvx.clippedEdgeVx1Id];
                            cvx2 = clippedTriangleVxes_once[cvx.clippedEdgeVx2Id];
                        }
                        else
                        {
                            CVF_ASSERT(false);
                        }

                        if(cvx1.isVxIdsNative && cvx2.isVxIdsNative)
                        {
                            m_triVxToCellCornerWeights.push_back(
                                RivIntersectionVertexWeights(cvx1.clippedEdgeVx1Id, cvx1.clippedEdgeVx2Id, cvx1.normDistFromEdgeVx1,
                                                             cvx2.clippedEdgeVx1Id, cvx2.clippedEdgeVx2Id, cvx2.normDistFromEdgeVx1,
                                                             cvx.normDistFromEdgeVx1));
                        }
                        else
                        {
                            caf::HexGridIntersectionTools::ClipVx cvx11;
                            caf::HexGridIntersectionTools::ClipVx cvx12;
                            caf::HexGridIntersectionTools::ClipVx cvx21;
                            caf::HexGridIntersectionTools::ClipVx cvx22;

                            if(cvx1.isVxIdsNative)
                            {
                                cvx11 = cvx1;
                                cvx12 = cvx1;
                            }
                            else if(cvx1.derivedVxLevel == 0)
                            {
                                cvx11 = hexPlaneCutTriangleVxes[cvx1.clippedEdgeVx1Id];
                                cvx12 = hexPlaneCutTriangleVxes[cvx1.clippedEdgeVx2Id];
                            }
                            else if(cvx2.derivedVxLevel == 1)
                            {
                                cvx11 = clippedTriangleVxes_once[cvx1.clippedEdgeVx1Id];
                                cvx12 = clippedTriangleVxes_once[cvx1.clippedEdgeVx2Id];
                            }
                            else
                            {
                                CVF_ASSERT(false);
                            }


                            if(cvx2.isVxIdsNative)
                            {
                                cvx21 = cvx2;
                                cvx22 = cvx2;
                            }
                            else if(cvx2.derivedVxLevel == 0)
                            {
                                cvx21 = hexPlaneCutTriangleVxes[cvx2.clippedEdgeVx1Id];
                                cvx22 = hexPlaneCutTriangleVxes[cvx2.clippedEdgeVx2Id];
                            }
                            else if(cvx2.derivedVxLevel == 1)
                            {
                                cvx21 = clippedTriangleVxes_once[cvx2.clippedEdgeVx1Id];
                                cvx22 = clippedTriangleVxes_once[cvx2.clippedEdgeVx2Id];
                             
                            }
                            else
                            {
                                CVF_ASSERT(false);
                            }

                            CVF_TIGHT_ASSERT(cvx11.isVxIdsNative && cvx12.isVxIdsNative && cvx21.isVxIdsNative && cvx22.isVxIdsNative);

                            m_triVxToCellCornerWeights.push_back(
                                RivIntersectionVertexWeights(cvx11.clippedEdgeVx1Id, cvx11.clippedEdgeVx2Id, cvx11.normDistFromEdgeVx1,
                                                             cvx12.clippedEdgeVx1Id, cvx12.clippedEdgeVx2Id, cvx2.normDistFromEdgeVx1,
                                                             cvx21.clippedEdgeVx1Id, cvx21.clippedEdgeVx2Id, cvx21.normDistFromEdgeVx1,
                                                             cvx22.clippedEdgeVx1Id, cvx22.clippedEdgeVx2Id, cvx22.normDistFromEdgeVx1,
                                                             cvx1.normDistFromEdgeVx1,
                                                             cvx2.normDistFromEdgeVx1,
                                                             cvx.normDistFromEdgeVx1));
                        
                        }
                    }
                }
            }
        }
    }
    m_cellBorderLineVxes->assign(cellBorderLineVxes);
    m_triangleVxes->assign(triangleVertices);
}
    


