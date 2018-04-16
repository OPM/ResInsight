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

#include "Riv3dWellLogGridGeomertyGenerator.h"

#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "cafDisplayCoordTransform.h"
#include "cvfObject.h"
#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include "cvfBoundingBox.h"

#include <map>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogGridGeometryGenerator::Riv3dWellLogGridGeometryGenerator(RimWellPath* wellPath)
    : m_wellPath(wellPath)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool
Riv3dWellLogGridGeometryGenerator::createGrid(const caf::DisplayCoordTransform* displayCoordTransform,
                                              const cvf::BoundingBox& wellPathClipBoundingBox,
                                              double                  planeAngle,
                                              double                  planeOffsetFromWellPathCenter,
                                              double                  planeWidth,
                                              double                  gridIntervalSize)
{
    CVF_ASSERT(gridIntervalSize > 0);

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

    size_t originalWellPathSize = wellPathPoints.size();

    if (wellPathCollection->wellPathClip)
    {
        double horizontalLengthAlongWellToClipPoint;
        double maxZClipHeight = wellPathClipBoundingBox.max().z() + wellPathCollection->wellPathClipZDistance;
        size_t indexToFirstVisibleSegment;
        wellPathPoints = RigWellPath::clipPolylineStartAboveZ(
            wellPathPoints, maxZClipHeight, &horizontalLengthAlongWellToClipPoint, &indexToFirstVisibleSegment);
    }

    if (wellPathPoints.size() < (size_t) 2)
    {
        // Need at least two well path points to create a valid path.
        return false;
    }

    // calculateLineSegmentNormals returns normals for the whole well path. Erase the part which is clipped off
    std::vector<cvf::Vec3d> wellPathSegmentNormals =
        RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathGeometry(), planeAngle);
    wellPathSegmentNormals.erase(wellPathSegmentNormals.begin(), wellPathSegmentNormals.end() - wellPathPoints.size());

    {
        std::vector<cvf::Vec3f> vertices;
        vertices.reserve(wellPathPoints.size() * 2);

        std::vector<cvf::uint> backgroundIndices;
        backgroundIndices.reserve(wellPathPoints.size() * 2);

        // Vertices are used for both surface and border
        for (size_t i = 0; i < wellPathPoints.size(); i++)
        {
            vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
                wellPathPoints[i] + wellPathSegmentNormals[i] * planeOffsetFromWellPathCenter)));
            vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
                wellPathPoints[i] + wellPathSegmentNormals[i] * (planeOffsetFromWellPathCenter + planeWidth))));
            backgroundIndices.push_back((cvf::uint) (2 * i));
            backgroundIndices.push_back((cvf::uint) (2 * i + 1));
        }
        
        cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(vertices);

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
            borderIndices.reserve(vertices.size());

            int secondLastEvenVertex = (int) vertices.size() - 4;

            // Border close to the well. All even indices.
            for (size_t i = 0; i <= secondLastEvenVertex; i += 2)
            {
                borderIndices.push_back((cvf::uint) i);
                borderIndices.push_back((cvf::uint) i+2);
            }

            // Connect to border away from well
            borderIndices.push_back((cvf::uint) (vertices.size() - 2));
            borderIndices.push_back((cvf::uint) (vertices.size() - 1));

            int secondOddVertex = 3;
            int lastOddVertex = (int) vertices.size() - 1;

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
        std::vector<cvf::Vec3d> interpolatedGridNormals;

        size_t newStartIndex = originalWellPathSize - wellPathPoints.size();
        double firstMd = wellPathGeometry()->m_measuredDepths.at(newStartIndex);
        double lastMd = wellPathGeometry()->m_measuredDepths.back();

        double md = lastMd;
        while (md >= firstMd)
        {
            cvf::Vec3d point = wellPathGeometry()->interpolatedPointAlongWellPath(md);
            cvf::Vec3d normal = wellPathGeometry()->interpolatedVectorAlongWellPath(wellPathSegmentNormals, md);
            interpolatedGridPoints.push_back(point);
            interpolatedGridNormals.push_back(normal.getNormalized());
            md -= gridIntervalSize;
        }

        std::vector<cvf::Vec3f> vertices;
        vertices.reserve(interpolatedGridPoints.size());

        std::vector<cvf::uint> indices;
        indices.reserve(interpolatedGridPoints.size());
        cvf::uint indexCounter = 0;
        // Normal lines. Start from one to avoid drawing at surface edge.
        for (size_t i = 1; i < interpolatedGridNormals.size(); i++)
        {
            vertices.push_back(cvf::Vec3f(
               displayCoordTransform->transformToDisplayCoord(interpolatedGridPoints[i] + interpolatedGridNormals[i] * planeOffsetFromWellPathCenter)));

            vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
               interpolatedGridPoints[i] + interpolatedGridNormals[i] * (planeOffsetFromWellPathCenter + planeWidth))));

            indices.push_back(indexCounter++);
            indices.push_back(indexCounter++);
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);

        cvf::ref<cvf::DrawableGeo> normalLinesDrawable = new cvf::DrawableGeo();

        indexedUInt->setIndices(indexArray.p());
        normalLinesDrawable->addPrimitiveSet(indexedUInt.p());

        cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(vertices);
        normalLinesDrawable->setVertexArray(vertexArray.p());

        m_normalLines = normalLinesDrawable;
    }
    return true;
}

cvf::ref<cvf::DrawableGeo> Riv3dWellLogGridGeometryGenerator::background()
{
    return m_background;
}

cvf::ref<cvf::DrawableGeo> Riv3dWellLogGridGeometryGenerator::border()
{
    return m_border;
}

cvf::ref<cvf::DrawableGeo> Riv3dWellLogGridGeometryGenerator::normalLines()
{
    return m_normalLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* Riv3dWellLogGridGeometryGenerator::wellPathGeometry() const
{
    return m_wellPath->wellPathGeometry();
}
