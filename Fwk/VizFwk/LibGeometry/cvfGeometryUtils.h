//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfVector3.h"
#include "cvfGeometryBuilder.h"

namespace cvf {


//==================================================================================================
//
// Static helper class for building geometry
//
//==================================================================================================
class GeometryUtils 
{
public:
    static void     createPatch(const Vec3f& origin, const Vec3f& uUnit, const Vec3f& vUnit, uint uCellCount, uint vCellCount, GeometryBuilder* builder);

    static void     createBox(const Vec3f& min, const Vec3f& max, GeometryBuilder* builder);
    static void     createBox(const Vec3f& centerPos, float extentX, float extentY, float extentZ, GeometryBuilder* builder);

    static void     createDisc(double radius, uint numSlices, GeometryBuilder* builder);
    static void     createDisc(double outerRadius, double innerRadius, uint numSlices, GeometryBuilder* builder);
    
    static void     createSphere(double radius, uint numSlices, uint numStacks, GeometryBuilder* builder);

    // These two should be refactored out into separate generator classes
    static void     createObliqueCylinder(float bottomRadius, float topRadius, float height, float topOffsetX, float topOffsetY, uint numSlices, bool normalsOutwards, bool closedBot, bool closedTop, uint numPolysZDir, GeometryBuilder* builder);
    static void     createCone(float bottomRadius, float height, uint numSlices, bool normalsOutwards, bool closedBot, bool singleTopNode, GeometryBuilder* builder);

    //static void     generatePointsOnCircle(double radius, int numPoints, Vec3fArray* generatedPoints);

    static void     tesselatePatchAsQuads(uint pointCountU, uint pointCountV, uint indexOffset, bool windingCCW, UIntArray* indices);
    static void     tesselatePatchAsTriangles(uint pointCountU, uint pointCountV, uint indexOffset, bool windingCCW, UIntArray* indices);

    static bool     isConvexQuad(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d);
    static Vec3f    quadNormal(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d);
    static Vec3f    polygonNormal(const Vec3fValueArray& vertices, const uint* indices, uint indexCount);

    static void     removeUnusedVertices(const UIntValueArray& vertexIndices, UIntArray* newVertexIndices, UIntArray* newToOldMapping, uint maxVertexCount);

    static bool     project(const Mat4d& projectionMultViewMatrix, const Vec2i& viewportPosition, const Vec2ui& viewportSize, const Vec3d& point, Vec3d* out);
};

}
