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

#include "RivIntersectionGeometryGenerator.h"

#include "RigMainGrid.h"
#include "RigResultAccessor.h"

#include "RimIntersection.h"

#include "RivHexGridIntersectionTools.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include "cvfDrawableGeo.h"
#include "cvfGeometryTools.h"
#include "cvfPlane.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionGeometryGenerator::RivIntersectionGeometryGenerator( RimIntersection* crossSection,
                                                                    std::vector<std::vector<cvf::Vec3d> > &polylines, 
                                                                    const cvf::Vec3d& extrusionDirection, 
                                                                    const RivIntersectionHexGridInterface* grid)
                                                                   : m_crossSection(crossSection),
                                                                   m_polyLines(polylines), 
                                                                   m_extrusionDirection(extrusionDirection), 
                                                                   m_hexGrid(grid)
{
    m_triangleVxes = new cvf::Vec3fArray;
    m_cellBorderLineVxes = new cvf::Vec3fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionGeometryGenerator::~RivIntersectionGeometryGenerator()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionGeometryGenerator::calculateArrays()
{
    if (m_triangleVxes->size()) return;

    m_extrusionDirection.normalize();
    std::vector<cvf::Vec3f> triangleVertices;
    std::vector<cvf::Vec3f> cellBorderLineVxes;
    cvf::Vec3d displayOffset = m_hexGrid->displayOffset();
    cvf::BoundingBox gridBBox = m_hexGrid->boundingBox();

    for (size_t pLineIdx = 0; pLineIdx < m_polyLines.size(); ++pLineIdx)
    {
        const std::vector<cvf::Vec3d>& m_polyLine = m_polyLines[pLineIdx];

        if (m_polyLine.size() < 2) continue;

        std::vector<cvf::Vec3d>     m_adjustedPolyline;
        adjustPolyline(m_polyLine, m_extrusionDirection, &m_adjustedPolyline);

        size_t lineCount = m_adjustedPolyline.size();
        for (size_t lIdx = 0; lIdx < lineCount - 1; ++lIdx)
        {
            cvf::Vec3d p1 = m_adjustedPolyline[lIdx];
            cvf::Vec3d p2 = m_adjustedPolyline[lIdx+1];

            cvf::BoundingBox sectionBBox;
            sectionBBox.add(p1);
            sectionBBox.add(p2);


            cvf::Vec3d maxHeightVec;

            double maxSectionHeight = sectionBBox.radius();
            double maxSectionHeightUp;
            double maxSectionHeightDown;

            if (m_crossSection->type == RimIntersection::CS_AZIMUTHLINE)
            {
                maxSectionHeightUp = m_crossSection->lengthUp();
                maxSectionHeightDown = m_crossSection->lengthDown();

                if (maxSectionHeightUp + maxSectionHeightDown == 0)
                {
                    return;
                }

                cvf::Vec3d maxHeightVecDown = m_extrusionDirection*maxSectionHeightUp;
                cvf::Vec3d maxHeightVecUp = m_extrusionDirection*maxSectionHeightDown;

                sectionBBox.add(p1 + maxHeightVecUp);
                sectionBBox.add(p1 - maxHeightVecDown);
                sectionBBox.add(p2 + maxHeightVecUp);
                sectionBBox.add(p2 - maxHeightVecDown);

                maxHeightVec = maxHeightVecUp + maxHeightVecDown;
            }
            else
            {
                maxHeightVec = m_extrusionDirection*maxSectionHeight;

                sectionBBox.add(p1 + maxHeightVec);
                sectionBBox.add(p1 - maxHeightVec);
                sectionBBox.add(p2 + maxHeightVec);
                sectionBBox.add(p2 - maxHeightVec);
            }


            std::vector<size_t> columnCellCandidates;
            m_hexGrid->findIntersectingCells(sectionBBox, &columnCellCandidates);

            cvf::Plane plane;
            plane.setFromPoints(p1, p2, p2 + maxHeightVec);

            cvf::Plane p1Plane;
            p1Plane.setFromPoints(p1, p1 + maxHeightVec, p1 + plane.normal());
            cvf::Plane p2Plane;
            p2Plane.setFromPoints(p2, p2 + maxHeightVec, p2 - plane.normal());

            std::vector<caf::HexGridIntersectionTools::ClipVx> hexPlaneCutTriangleVxes;
            hexPlaneCutTriangleVxes.reserve(5*3);
            std::vector<bool> isTriangleEdgeCellContour;
            isTriangleEdgeCellContour.reserve(5*3);
            cvf::Vec3d cellCorners[8];
            size_t cornerIndices[8];

            for (size_t cccIdx = 0; cccIdx < columnCellCandidates.size(); ++cccIdx)
            {
                size_t globalCellIdx = columnCellCandidates[cccIdx];

                if (!m_hexGrid->useCell(globalCellIdx)) continue;

                hexPlaneCutTriangleVxes.clear();
                m_hexGrid->cellCornerVertices(globalCellIdx, cellCorners);
                m_hexGrid->cellCornerIndices(globalCellIdx, cornerIndices);

                caf::HexGridIntersectionTools::planeHexIntersectionMC(plane,
                                       cellCorners,
                                       cornerIndices,
                                       &hexPlaneCutTriangleVxes,
                                       &isTriangleEdgeCellContour);

                if (m_crossSection->type == RimIntersection::CS_AZIMUTHLINE)
                {
                    bool hasAnyPointsOnSurface = false;
                    for (caf::HexGridIntersectionTools::ClipVx vertex : hexPlaneCutTriangleVxes)
                    {
                        cvf::Vec3d temp = vertex.vx - p1;
                        double dot = temp.dot(m_extrusionDirection);
                        double lengthCheck;
                        
                        if (dot < 0)
                        {
                            lengthCheck = maxSectionHeightUp;
                        }
                        else
                        {
                            lengthCheck = maxSectionHeightDown;
                        }

                        double distance = cvf::Math::sqrt(cvf::GeometryTools::linePointSquareDist(p1, p2, vertex.vx));
                        if (distance < lengthCheck)
                        {
                            hasAnyPointsOnSurface = true;
                            break;
                        }
                    }
                    if (!hasAnyPointsOnSurface)
                    {
                        continue;
                    }
                }

                std::vector<caf::HexGridIntersectionTools::ClipVx> clippedTriangleVxes;
                std::vector<bool> isClippedTriEdgeCellContour;

                caf::HexGridIntersectionTools::clipTrianglesBetweenTwoParallelPlanes(hexPlaneCutTriangleVxes, isTriangleEdgeCellContour, p1Plane, p2Plane,
                                                      &clippedTriangleVxes, &isClippedTriEdgeCellContour);

                size_t clippedTriangleCount = clippedTriangleVxes.size()/3;

                for (uint tIdx = 0; tIdx < clippedTriangleCount; ++tIdx)
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

                    if (isClippedTriEdgeCellContour[triVxIdx])
                    {
                        cellBorderLineVxes.push_back(p0);
                        cellBorderLineVxes.push_back(p1);
                    }
                    if (isClippedTriEdgeCellContour[triVxIdx+1])
                    {
                        cellBorderLineVxes.push_back(p1);
                        cellBorderLineVxes.push_back(p2);
                    }
                    if (isClippedTriEdgeCellContour[triVxIdx+2])
                    {
                        cellBorderLineVxes.push_back(p2);
                        cellBorderLineVxes.push_back(p0);
                    }

                    // Mapping to cell index

                    m_triangleToCellIdxMap.push_back(globalCellIdx);

                    // Interpolation from nodes
                    for (int i = 0; i < 3; ++i)
                    {
                        caf::HexGridIntersectionTools::ClipVx cvx = clippedTriangleVxes[triVxIdx + i];
                        if (cvx.isVxIdsNative)
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
    }
    m_triangleVxes->assign(triangleVertices);
    m_cellBorderLineVxes->assign(cellBorderLineVxes);
}


//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionGeometryGenerator::generateSurface()
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
cvf::ref<cvf::DrawableGeo> RivIntersectionGeometryGenerator::createMeshDrawable()
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
cvf::ref<cvf::DrawableGeo> RivIntersectionGeometryGenerator::createLineAlongPolylineDrawable()
{
    return createLineAlongPolylineDrawable(m_polyLines);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionGeometryGenerator::createLineAlongPolylineDrawable(const std::vector<std::vector<cvf::Vec3d> >& polyLines)
{
    std::vector<cvf::uint> lineIndices;
    std::vector<cvf::Vec3f> vertices;

    cvf::Vec3d displayOffset = m_hexGrid->displayOffset();

    for (size_t pLineIdx = 0; pLineIdx < polyLines.size(); ++pLineIdx)
    {
        const std::vector<cvf::Vec3d>& polyLine = polyLines[pLineIdx];
        if (polyLine.size() < 2) continue;

        for (size_t i = 0; i < polyLine.size(); ++i)
        {
            vertices.push_back(cvf::Vec3f(polyLine[i] - displayOffset));
            if (i < polyLine.size() - 1)
            {
                lineIndices.push_back(static_cast<cvf::uint>(i));
                lineIndices.push_back(static_cast<cvf::uint>(i + 1));
            }
        }
    }

    if (vertices.size() == 0) return nullptr;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray;
    vx->assign(vertices);
    cvf::ref<cvf::UIntArray> idxes = new cvf::UIntArray;
    idxes->assign(lineIndices);

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(idxes.p());

    cvf::ref<cvf::DrawableGeo> polylineGeo = new cvf::DrawableGeo;
    polylineGeo->setVertexArray(vx.p());
    polylineGeo->addPrimitiveSet(prim.p());

    return polylineGeo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionGeometryGenerator::createPointsFromPolylineDrawable()
{
    return createPointsFromPolylineDrawable(m_polyLines);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionGeometryGenerator::createPointsFromPolylineDrawable(const std::vector<std::vector<cvf::Vec3d> >& polyLines)
{
    std::vector<cvf::Vec3f> vertices;

    cvf::Vec3d displayOffset = m_hexGrid->displayOffset();

    for (size_t pLineIdx = 0; pLineIdx < polyLines.size(); ++pLineIdx)
    {
        const std::vector<cvf::Vec3d>& polyLine = polyLines[pLineIdx];
        for (size_t i = 0; i < polyLine.size(); ++i)
        {
            vertices.push_back(cvf::Vec3f(polyLine[i] - displayOffset));
        }
    }

    if (vertices.size() == 0) return nullptr;

    cvf::ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect(cvf::PT_POINTS);
    primSet->setStartIndex(0);
    primSet->setIndexCount(vertices.size());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray(vertices);
    geo->setVertexArray(vx.p());
    geo->addPrimitiveSet(primSet.p());

    return geo;

}

//--------------------------------------------------------------------------------------------------
/// Remove the lines from the polyline that is nearly parallel to the extrusion direction
//--------------------------------------------------------------------------------------------------
void RivIntersectionGeometryGenerator::adjustPolyline(const std::vector<cvf::Vec3d>& polyLine, 
                                                      const cvf::Vec3d extrDir,
                                                      std::vector<cvf::Vec3d>* adjustedPolyline)
{
    size_t lineCount = polyLine.size();
    if (!polyLine.size()) return;

    adjustedPolyline->push_back(polyLine[0]);
    cvf::Vec3d p1 = polyLine[0];

    for (size_t lIdx = 1; lIdx < lineCount; ++lIdx)
    {
        cvf::Vec3d p2 = polyLine[lIdx];
        cvf::Vec3d p1p2 = p2 - p1;

        if ((p1p2 - (p1p2 * extrDir)*extrDir).length() > 0.1 )
        {
            adjustedPolyline->push_back(p2);
            p1 = p2;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivIntersectionGeometryGenerator::triangleToCellIndex() const
{
    CVF_ASSERT(m_triangleVxes->size());
    return m_triangleToCellIdxMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RivIntersectionVertexWeights>& RivIntersectionGeometryGenerator::triangleVxToCellCornerInterpolationWeights() const
{
    CVF_ASSERT(m_triangleVxes->size());
    return m_triVxToCellCornerWeights;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::Vec3fArray* RivIntersectionGeometryGenerator::triangleVxes() const
{
    CVF_ASSERT(m_triangleVxes->size());
    return m_triangleVxes.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersection* RivIntersectionGeometryGenerator::crossSection() const
{
    return m_crossSection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivIntersectionGeometryGenerator::isAnyGeometryPresent() const
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

