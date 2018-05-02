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
    if (wellPathPoints.empty())
    {
        return false;
    }

    for (cvf::Vec3d& wellPathPoint : wellPathPoints)
    {
        wellPathPoint = displayCoordTransform->transformToDisplayCoord(wellPathPoint);
    }

    std::vector<cvf::Vec3d> segmentNormals =
        RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathPoints, planeAngle);


    size_t originalWellPathSize = wellPathPoints.size();

    if (wellPathCollection->wellPathClip)
    {
        double clipZDistance = wellPathCollection->wellPathClipZDistance;
        double horizontalLengthAlongWellToClipPoint;
        cvf::Vec3d clipLocation = wellPathClipBoundingBox.max() + clipZDistance * cvf::Vec3d(0, 0, 1);
        clipLocation = displayCoordTransform->transformToDisplayCoord(clipLocation);
        size_t indexToFirstVisibleSegment;
        wellPathPoints = RigWellPath::clipPolylineStartAboveZ(
            wellPathPoints, clipLocation.z(), &horizontalLengthAlongWellToClipPoint, &indexToFirstVisibleSegment);
    }

    if (wellPathPoints.size() < (size_t) 2)
    {
        // Need at least two well path points to create a valid path.
        return false;
    }

    // Note that normals are calculated on the full non-clipped well path to increase the likelihood of creating good normals
    // for the end points of the curve. So we need to clip the remainder here.
    segmentNormals.erase(segmentNormals.begin(), segmentNormals.end() - wellPathPoints.size());

    {
        m_vertices.reserve(wellPathPoints.size() * 2);

        std::vector<cvf::uint> backgroundIndices;
        backgroundIndices.reserve(wellPathPoints.size() * 2);

        // Vertices are used for both surface and border
        for (size_t i = 0; i < wellPathPoints.size(); i++)
        {
            m_vertices.push_back(cvf::Vec3f(
                wellPathPoints[i] + segmentNormals[i] * planeOffsetFromWellPathCenter));
            m_vertices.push_back(cvf::Vec3f(
                wellPathPoints[i] + segmentNormals[i] * (planeOffsetFromWellPathCenter + planeWidth)));
            backgroundIndices.push_back((cvf::uint) (2 * i));
            backgroundIndices.push_back((cvf::uint) (2 * i + 1));
        }
        
        cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(m_vertices);

        {
            // Background specific
            cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_TRIANGLE_STRIP);
            cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(backgroundIndices);
            indexedUInt->setIndices(indexArray.p());

            m_background = new cvf::DrawableGeo();
            m_background->addPrimitiveSet(indexedUInt.p());
            m_background->setVertexArray(vertexArray.p());          
        }
       
        {
            std::vector<cvf::uint> borderIndices;
            borderIndices.reserve(m_vertices.size());

            int secondLastEvenVertex = (int) m_vertices.size() - 4;

            // Border close to the well. All even indices.
            for (int i = 0; i <= secondLastEvenVertex; i += 2)
            {
                borderIndices.push_back((cvf::uint) i);
                borderIndices.push_back((cvf::uint) i+2);
            }

            // Connect to border away from well
            borderIndices.push_back((cvf::uint) (m_vertices.size() - 2));
            borderIndices.push_back((cvf::uint) (m_vertices.size() - 1));

            int secondOddVertex = 3;
            int lastOddVertex = (int) m_vertices.size() - 1;

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
            m_border->setVertexArray(vertexArray.p());
        }
    }
    {
        std::vector<cvf::Vec3d> interpolatedGridPoints;
        std::vector<cvf::Vec3d> interpolatedGridCurveNormals;

        size_t newStartIndex = originalWellPathSize - wellPathPoints.size();
        double firstMd = wellPathGeometry()->m_measuredDepths.at(newStartIndex);
        double lastMd = wellPathGeometry()->m_measuredDepths.back();

        double md = lastMd;
        while (md >= firstMd)
        {
            cvf::Vec3d point = wellPathGeometry()->interpolatedVectorAlongWellPath(wellPathPoints, md);
            cvf::Vec3d curveNormal = wellPathGeometry()->interpolatedVectorAlongWellPath(segmentNormals, md);
            interpolatedGridPoints.push_back(point);
            interpolatedGridCurveNormals.push_back(curveNormal.getNormalized());
            md -= samplingIntervalSize;
        }

        std::vector<cvf::Vec3f> arrowVertices;
        std::vector<cvf::Vec3f> arrowVectors;
        arrowVertices.reserve(interpolatedGridPoints.size());
        arrowVectors.reserve(interpolatedGridPoints.size());

        double shaftRelativeRadius = 0.0125f;
        double arrowHeadRelativeRadius = shaftRelativeRadius * 3;
        double arrowHeadRelativeLength = arrowHeadRelativeRadius * 3;
        double totalArrowScaling = 1.0 / (1.0 - arrowHeadRelativeLength);
        // Normal lines. Start from one to avoid drawing at surface edge.        
        for (size_t i = 1; i < interpolatedGridCurveNormals.size(); i++)
        {
            arrowVertices.push_back(cvf::Vec3f(interpolatedGridPoints[i] + interpolatedGridCurveNormals[i] * planeOffsetFromWellPathCenter));

            arrowVectors.push_back(cvf::Vec3f(interpolatedGridCurveNormals[i] * planeWidth * totalArrowScaling));
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
const std::vector<cvf::Vec3f>& Riv3dWellLogDrawSurfaceGenerator::vertices() const
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* Riv3dWellLogDrawSurfaceGenerator::wellPathGeometry() const
{
    return m_wellPath->wellPathGeometry();
}
