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

#pragma once

#include "cvfBase.h"
#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

namespace cvf
{
class DrawableGeo;
class Part;
class ScalarMapper;
} // namespace cvf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
struct CompletionVizData
{
    CompletionVizData(const cvf::Vec3d& anchor, const cvf::Vec3d& direction, double connectionFactor, size_t globalCellIndex)
        : m_anchor(anchor)
        , m_direction(direction)
        , m_connectionFactor(connectionFactor)
        , m_globalCellIndex(globalCellIndex)
    {
    }

    cvf::Vec3d m_anchor;
    cvf::Vec3d m_direction;
    double     m_connectionFactor;
    size_t     m_globalCellIndex;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RivWellConnectionFactorGeometryGenerator : public cvf::Object
{
public:
    RivWellConnectionFactorGeometryGenerator(std::vector<CompletionVizData>& completionVizData, float radius);
    ~RivWellConnectionFactorGeometryGenerator() override;

    cvf::ref<cvf::Part> createSurfacePart(const cvf::ScalarMapper* scalarMapper, bool disableLighting);

    double connectionFactor(cvf::uint triangleIndex) const;
    size_t globalCellIndexFromTriangleIndex(cvf::uint triangleIndex) const;

private:
    size_t mapFromTriangleToConnectionIndex(cvf::uint triangleIndex) const;
    cvf::ref<cvf::DrawableGeo> createSurfaceGeometry();

    static cvf::Mat4f rotationMatrixBetweenVectors(const cvf::Vec3d& v1, const cvf::Vec3d& v2);
    static void
        createStarGeometry(std::vector<cvf::Vec3f>* vertices, std::vector<cvf::uint>* indices, float radius, float thickness);

    static void createSimplifiedStarGeometry(std::vector<cvf::Vec3f>* vertices,
                                             std::vector<cvf::uint>*  indices,
                                             float                    radius,
                                             float                    thickness);

private:
    std::vector<CompletionVizData> m_completionVizData;
    float                          m_radius;
    size_t                         m_trianglesPerConnection;
};
