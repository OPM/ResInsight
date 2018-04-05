/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RivWellConnectionFactorGeometryGenerator.h"

#include "cafEffectGenerator.h"
#include "cvfArray.h"
#include "cvfDrawableGeo.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellConnectionFactorGeometryGenerator::RivWellConnectionFactorGeometryGenerator(
    std::vector<CompletionVizData>& completionVizData,
    float                           radius)
    : m_completionVizData(completionVizData)
    , m_radius(radius)
    , m_trianglesPerConnection(0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellConnectionFactorGeometryGenerator::~RivWellConnectionFactorGeometryGenerator() {}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellConnectionFactorGeometryGenerator::createSurfacePart(const cvf::ScalarMapper* scalarMapper, bool disableLighting)
{
    if (!scalarMapper) return nullptr;

    cvf::ref<cvf::DrawableGeo> drawable = createSurfaceGeometry();
    if (drawable.notNull())
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawable.p());

        // Compute texture coords
        cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray();
        {
            textureCoords->reserve(drawable->vertexArray()->size());
            size_t verticesPerItem = drawable->vertexArray()->size() / m_completionVizData.size();

            textureCoords->setAll(cvf::Vec2f(0.5f, 1.0f));

            for (const auto& item : m_completionVizData)
            {
                cvf::Vec2f textureCoord = cvf::Vec2f(0.5f, 1.0f);
                if (item.m_connectionFactor != HUGE_VAL)
                {
                    textureCoord = scalarMapper->mapToTextureCoord(item.m_connectionFactor);
                }

                for (size_t i = 0; i < verticesPerItem; i++)
                {
                    textureCoords->add(textureCoord);
                }
            }
        }

        drawable->setTextureCoordArray(textureCoords.p());

        caf::ScalarMapperEffectGenerator effGen(scalarMapper, caf::PO_1);
        effGen.disableLighting(disableLighting);

        cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();
        part->setEffect(eff.p());

        return part;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellConnectionFactorGeometryGenerator::createSurfaceGeometry()
{
    std::vector<cvf::Vec3f> verticesForOneObject;
    std::vector<cvf::uint>  indicesForOneObject;

    RivWellConnectionFactorGeometryGenerator::createSimplifiedStarGeometry(
        &verticesForOneObject, &indicesForOneObject, m_radius, m_radius * 0.3f);

    m_trianglesPerConnection = indicesForOneObject.size() / 3;

    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray;
    cvf::ref<cvf::UIntArray>  indices  = new cvf::UIntArray;

    auto indexCount  = m_completionVizData.size() * indicesForOneObject.size();
    auto vertexCount = m_completionVizData.size() * verticesForOneObject.size();
    indices->reserve(indexCount);
    vertices->reserve(vertexCount);

    for (const auto& item : m_completionVizData)
    {
        auto rotMatrix = rotationMatrixBetweenVectors(cvf::Vec3d::Y_AXIS, item.m_direction);

        cvf::uint indexOffset = static_cast<cvf::uint>(vertices->size());

        for (const auto& v : verticesForOneObject)
        {
            auto rotatedPoint = v.getTransformedPoint(rotMatrix);

            vertices->add(cvf::Vec3f(item.m_anchor) + rotatedPoint);
        }

        for (const auto& i : indicesForOneObject)
        {
            indices->add(i + indexOffset);
        }
    }

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();
    drawable->setVertexArray(vertices.p());

    drawable->addPrimitiveSet(new cvf::PrimitiveSetIndexedUInt(cvf::PT_TRIANGLES, indices.p()));
    drawable->computeNormals();

    return drawable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RivWellConnectionFactorGeometryGenerator::connectionFactor(cvf::uint triangleIndex) const
{
    size_t connectionIndex = mapFromTriangleToConnectionIndex(triangleIndex);

    return m_completionVizData[connectionIndex].m_connectionFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivWellConnectionFactorGeometryGenerator::globalCellIndexFromTriangleIndex(cvf::uint triangleIndex) const
{
    size_t connectionIndex = mapFromTriangleToConnectionIndex(triangleIndex);

    return m_completionVizData[connectionIndex].m_globalCellIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RivWellConnectionFactorGeometryGenerator::mapFromTriangleToConnectionIndex(cvf::uint triangleIndex) const
{
    if (m_trianglesPerConnection == 0) return 0;

    size_t connectionIndex = triangleIndex / m_trianglesPerConnection;

    return connectionIndex;
}

//--------------------------------------------------------------------------------------------------
/// Taken from OverlayNavigationCube::computeNewUpVector
/// Consider move to geometry util class
//--------------------------------------------------------------------------------------------------
cvf::Mat4f RivWellConnectionFactorGeometryGenerator::rotationMatrixBetweenVectors(const cvf::Vec3d& v1, const cvf::Vec3d& v2)
{
    using namespace cvf;

    Vec3d rotAxis = v1 ^ v2;
    rotAxis.normalize();

    // Guard acos against out-of-domain input
    const double dotProduct = Math::clamp(v1 * v2, -1.0, 1.0);
    const double angle      = Math::acos(dotProduct);
    Mat4d        rotMat     = Mat4d::fromRotation(rotAxis, angle);

    Mat4f myMat(rotMat);

    return myMat;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellConnectionFactorGeometryGenerator::createStarGeometry(std::vector<cvf::Vec3f>* vertices,
                                                                  std::vector<cvf::uint>*  indices,
                                                                  float                    radius,
                                                                  float                    thickness)
{
    auto p0 = cvf::Vec3f::Z_AXIS * radius;
    auto p2 = cvf::Vec3f::X_AXIS * -radius;
    auto p4 = -p0;
    auto p6 = -p2;

    float innerFactor = 5.0f;

    auto p1 = (p0 + p2) / innerFactor;
    auto p3 = (p2 + p4) / innerFactor;

    auto p5 = -p1;
    auto p7 = -p3;

    auto p8 = cvf::Vec3f::Y_AXIS * thickness;
    auto p9 = -p8;

    vertices->push_back(p0);
    vertices->push_back(p1);
    vertices->push_back(p2);
    vertices->push_back(p3);
    vertices->push_back(p4);
    vertices->push_back(p5);
    vertices->push_back(p6);
    vertices->push_back(p7);
    vertices->push_back(p8);
    vertices->push_back(p9);

    // Top
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(8);

    indices->push_back(0);
    indices->push_back(8);
    indices->push_back(7);

    indices->push_back(0);
    indices->push_back(9);
    indices->push_back(1);

    indices->push_back(0);
    indices->push_back(7);
    indices->push_back(9);

    // Left
    indices->push_back(2);
    indices->push_back(3);
    indices->push_back(8);

    indices->push_back(2);
    indices->push_back(8);
    indices->push_back(1);

    indices->push_back(2);
    indices->push_back(9);
    indices->push_back(3);

    indices->push_back(2);
    indices->push_back(1);
    indices->push_back(9);

    // Bottom
    indices->push_back(4);
    indices->push_back(5);
    indices->push_back(8);

    indices->push_back(4);
    indices->push_back(8);
    indices->push_back(3);

    indices->push_back(4);
    indices->push_back(9);
    indices->push_back(5);

    indices->push_back(4);
    indices->push_back(3);
    indices->push_back(9);

    // Right
    indices->push_back(6);
    indices->push_back(7);
    indices->push_back(8);

    indices->push_back(6);
    indices->push_back(8);
    indices->push_back(5);

    indices->push_back(6);
    indices->push_back(9);
    indices->push_back(7);

    indices->push_back(6);
    indices->push_back(5);
    indices->push_back(9);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellConnectionFactorGeometryGenerator::createSimplifiedStarGeometry(std::vector<cvf::Vec3f>* vertices,
                                                                            std::vector<cvf::uint>*  indices,
                                                                            float                    radius,
                                                                            float                    thickness)
{
    // Suggested improvement
    // As the nodes are duplicated, it will be possible create only vertices and then use DrawableGeo::setFromTriangleVertexArray

    auto p0 = cvf::Vec3f::Y_AXIS * thickness;
    auto p1 = -p0;
    auto p2 = cvf::Vec3f::Z_AXIS * radius;

    auto p3 = p0;
    auto p4 = p1;
    auto p5 = cvf::Vec3f::X_AXIS * radius;

    auto p6 = p0;
    auto p7 = p1;
    auto p8 = -p2;

    auto p9  = p0;
    auto p10 = p1;
    auto p11 = -p5;

    vertices->push_back(p0);
    vertices->push_back(p1);
    vertices->push_back(p2);
    vertices->push_back(p3);
    vertices->push_back(p4);
    vertices->push_back(p5);
    vertices->push_back(p6);
    vertices->push_back(p7);
    vertices->push_back(p8);
    vertices->push_back(p9);
    vertices->push_back(p10);
    vertices->push_back(p11);

    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);

    indices->push_back(3);
    indices->push_back(4);
    indices->push_back(5);

    indices->push_back(6);
    indices->push_back(7);
    indices->push_back(8);

    indices->push_back(9);
    indices->push_back(10);
    indices->push_back(11);
}
