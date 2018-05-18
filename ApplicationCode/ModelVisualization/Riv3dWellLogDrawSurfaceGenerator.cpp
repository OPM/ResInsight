/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Riv3dWellLogDrawSurfaceGenerator.h"

#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "cafDisplayCoordTransform.h"

#include "cvfObject.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfArrowGenerator.h"

#include <algorithm>
#include <map>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogDrawSurfaceGenerator::Riv3dWellLogDrawSurfaceGenerator(RimWellPath* wellPath)
    : m_wellPath(wellPath)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool
Riv3dWellLogDrawSurfaceGenerator::createDrawSurface(const caf::DisplayCoordTransform* displayCoordTransform,
                                              const cvf::BoundingBox& wellPathClipBoundingBox,
                                              double                  planeAngle,
                                              double                  planeOffsetFromWellPathCenter,
                                              double                  planeWidth,
                                              double                  samplingIntervalSize)
{
    CVF_ASSERT(samplingIntervalSize > 0);

    clearGeometry();

    if (!wellPathGeometry() || wellPathGeometry()->m_measuredDepths.empty())
    {
        return false;
    }

    if (!wellPathClipBoundingBox.isValid())
    {
        return false;
    }


    RimWellPathCollection* wellPathCollection = nullptr;
    m_wellPath->firstAncestorOrThisOfTypeAsserted(wellPathCollection);

    std::vector<cvf::Vec3d> wellPathPoints = wellPathGeometry()->m_wellPathPoints;
    if (wellPathPoints.size() < (size_t)2)
    {
        // Need at least two well path points to create a valid path.
        return false;
    }

    for (cvf::Vec3d& wellPathPoint : wellPathPoints)
    {
        wellPathPoint = displayCoordTransform->transformToDisplayCoord(wellPathPoint);
    }

    std::vector<cvf::Vec3d> wellPathSegmentNormals =
        RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathPoints, planeAngle);

    size_t indexToFirstVisibleSegment = 0u;
    if (wellPathCollection->wellPathClip)
    {
        double clipZDistance = wellPathCollection->wellPathClipZDistance;
        cvf::Vec3d clipLocation = wellPathClipBoundingBox.max() + clipZDistance * cvf::Vec3d(0, 0, 1);        
        clipLocation = displayCoordTransform->transformToDisplayCoord(clipLocation);
        double horizontalLengthAlongWellToClipPoint;
        
        wellPathPoints = RigWellPath::clipPolylineStartAboveZ(
            wellPathPoints, clipLocation.z(), &horizontalLengthAlongWellToClipPoint, &indexToFirstVisibleSegment);
    }
      
    // Create curve normal vectors using the unclipped well path points and normals.
    createCurveNormalVectors(displayCoordTransform, indexToFirstVisibleSegment, planeOffsetFromWellPathCenter, planeWidth, samplingIntervalSize, wellPathSegmentNormals);

    // Note that normals are calculated on the full non-clipped well path. So we need to clip the start here.
    wellPathSegmentNormals.erase(wellPathSegmentNormals.begin(), wellPathSegmentNormals.end() - wellPathPoints.size());

    if (wellPathPoints.size() < (size_t)2)
    {
        // Need at least two well path points to create a valid path.
        return false;
    }

    m_vertices.reserve(wellPathPoints.size() * 2);
    for (size_t i = 0; i < wellPathPoints.size(); i++)
    {
        m_vertices.push_back(wellPathPoints[i] + wellPathSegmentNormals[i] * (planeOffsetFromWellPathCenter - 0.025*planeWidth));
        m_vertices.push_back(wellPathPoints[i] + wellPathSegmentNormals[i] * (planeOffsetFromWellPathCenter + 1.025*planeWidth));
    }
        
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(m_vertices.size());
    for (size_t i = 0; i < m_vertices.size(); ++i)
    {
        (*vertexArray)[i] = cvf::Vec3f(m_vertices[i]);
    }
    createBackground(vertexArray.p());
    createBorder(vertexArray.p());
  
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogDrawSurfaceGenerator::clearGeometry()
{
    m_background = nullptr;
    m_border = nullptr;
    m_curveNormalVectors = nullptr;
    m_vertices.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> Riv3dWellLogDrawSurfaceGenerator::background() const
{
    return m_background;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> Riv3dWellLogDrawSurfaceGenerator::border() const
{
    return m_border;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableVectors> Riv3dWellLogDrawSurfaceGenerator::curveNormalVectors() const
{
    return m_curveNormalVectors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& Riv3dWellLogDrawSurfaceGenerator::vertices() const
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogDrawSurfaceGenerator::createCurveNormalVectors(const caf::DisplayCoordTransform* displayCoordTransform, 
                                                                size_t                         clipStartIndex,
                                                                double                         planeOffsetFromWellPathCenter,
                                                                double                         planeWidth,
                                                                double                         samplingIntervalSize,
                                                                const std::vector<cvf::Vec3d>& segmentNormals)
{
    std::vector<cvf::Vec3d> interpolatedWellPathPoints;
    std::vector<cvf::Vec3d> interpolatedWellPathNormals;

    double firstMd = wellPathGeometry()->m_measuredDepths.at(clipStartIndex);
    double lastMd = wellPathGeometry()->m_measuredDepths.back();

    double md = lastMd;
    while (md >= firstMd)
    {
        cvf::Vec3d point = wellPathGeometry()->interpolatedPointAlongWellPath(md);
        point = displayCoordTransform->transformToDisplayCoord(point);
        cvf::Vec3d curveNormal = wellPathGeometry()->interpolatedVectorAlongWellPath(segmentNormals, md);
        interpolatedWellPathPoints.push_back(point);
        interpolatedWellPathNormals.push_back(curveNormal.getNormalized());
        md -= samplingIntervalSize;
    }

    std::vector<cvf::Vec3f> arrowVertices;
    std::vector<cvf::Vec3f> arrowVectors;
    arrowVertices.reserve(interpolatedWellPathPoints.size());
    arrowVectors.reserve(interpolatedWellPathPoints.size());

    double shaftRelativeRadius = 0.0125f;
    double arrowHeadRelativeRadius = shaftRelativeRadius * 3;
    double arrowHeadRelativeLength = arrowHeadRelativeRadius * 3;
    double totalArrowScaling = 1.0 / (1.0 - arrowHeadRelativeLength);
    // Normal lines. Start from one to avoid drawing at surface edge.        
    for (size_t i = 1; i < interpolatedWellPathNormals.size(); i++)
    {
        arrowVertices.push_back(cvf::Vec3f(interpolatedWellPathPoints[i] + interpolatedWellPathNormals[i] * planeOffsetFromWellPathCenter));

        arrowVectors.push_back(cvf::Vec3f(interpolatedWellPathNormals[i] * planeWidth * totalArrowScaling));
    }

    if (arrowVertices.empty() || arrowVectors.empty())
    {
        return;
    }

    m_curveNormalVectors = new cvf::DrawableVectors();

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(arrowVertices);
    cvf::ref<cvf::Vec3fArray> vectorArray = new cvf::Vec3fArray(arrowVectors);

    // Create the arrow glyph for the vector drawer
    cvf::GeometryBuilderTriangles arrowBuilder;
    cvf::ArrowGenerator gen;
    gen.setShaftRelativeRadius(shaftRelativeRadius);
    gen.setHeadRelativeRadius(arrowHeadRelativeRadius);
    gen.setHeadRelativeLength(arrowHeadRelativeLength);
    gen.setNumSlices(4);
    gen.generate(&arrowBuilder);

    m_curveNormalVectors->setGlyph(arrowBuilder.trianglesUShort().p(), arrowBuilder.vertices().p());
    m_curveNormalVectors->setVectors(vertexArray.p(), vectorArray.p());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogDrawSurfaceGenerator::createBackground(cvf::Vec3fArray* vertexArray)
{
    std::vector<cvf::uint> backgroundIndices;
    backgroundIndices.reserve(vertexArray->size());
    for (size_t i = 0; i < vertexArray->size(); ++i)
    {
        backgroundIndices.push_back((cvf::uint) (i));
    }

    // Background specific
    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_TRIANGLE_STRIP);
    cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(backgroundIndices);
    indexedUInt->setIndices(indexArray.p());

    m_background = new cvf::DrawableGeo();
    m_background->addPrimitiveSet(indexedUInt.p());
    m_background->setVertexArray(vertexArray);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogDrawSurfaceGenerator::createBorder(cvf::Vec3fArray* vertexArray)
{
    std::vector<cvf::uint> borderIndices;
    borderIndices.reserve(m_vertices.size());

    int secondLastEvenVertex = (int)vertexArray->size() - 4;

    // Border close to the well. All even indices.
    for (int i = 0; i <= secondLastEvenVertex; i += 2)
    {
        borderIndices.push_back((cvf::uint) i);
        borderIndices.push_back((cvf::uint) i + 2);
    }

    // Connect to border away from well
    borderIndices.push_back((cvf::uint) (vertexArray->size() - 2));
    borderIndices.push_back((cvf::uint) (vertexArray->size() - 1));

    int secondOddVertex = 3;
    int lastOddVertex = (int)vertexArray->size() - 1;

    // Border away from from well are odd indices in reverse order to create a closed surface.
    for (int i = lastOddVertex; i >= secondOddVertex; i -= 2)
    {
        borderIndices.push_back((cvf::uint) i);
        borderIndices.push_back((cvf::uint) i - 2);
    }
    // Close border
    borderIndices.push_back(1u);
    borderIndices.push_back(0u);

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
    cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(borderIndices);
    indexedUInt->setIndices(indexArray.p());

    m_border = new cvf::DrawableGeo();
    m_border->addPrimitiveSet(indexedUInt.p());
    m_border->setVertexArray(vertexArray);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* Riv3dWellLogDrawSurfaceGenerator::wellPathGeometry() const
{
    return m_wellPath->wellPathGeometry();
}
