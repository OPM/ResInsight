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


#include "cvfBase.h"
#include "cvfGeometryBuilder.h"
#include "cvfGeometryUtils.h"
#include "cvfMatrix4.h"

#include <map>

namespace cvf {



//==================================================================================================
///
/// \class cvf::GeometryUtils
/// \ingroup Geometry
///
/// Static helper class for creating geometries from primitive shapes.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Create a 2D patch
/// 
/// \param origin       The start point of the patch
/// \param uUnit        Direction vector u. First point 'to the right of' origin is origin + uUnit.
/// \param vUnit        Direction vector v. Coordinates of first point 'above' origin is origin + vunit. 
/// \param uCellCount   The number of cells/quads to generate along the uUnit dimension.
/// \param vCellCount   The number of cells/quads to generate along the vUnit dimension.
/// \param builder      Geometry builder to use when creating geometry
/// 
/// The figure below illustrates how the patch is constructed from the specified parameters.
/// 
/// <PRE>
///         v8-----v9----v10----v11             Parameters:            Resulting vertices:
///         |      |      |      |                origin = (10,20,0)     v0 = (10,20,0)
///  origin |      |      |      |                uUnit  = (2,0,0)       v1 = (12,20,0)
/// + vunit v4-----v5-----v6-----v7   |y          vUnit  = (0,1,0)       v2 = (14,20,0)
///         |      |      |      |    |           uCellCount = 3         v3 = (16,20,0)
///         |      |      |      |    |           vCellCount = 2         v4 = (10,21,0)
///         v0-----v1-----v2-----v3   *----x                             v5 = (12,21,0)
///     origin    origin                                                 :
///              + uUnit   </PRE>
/// 
/// The following quad connectivities will be produced:\n
/// <TT> &nbsp; &nbsp; (v4,v0,v1,v5)  (v5,v1,v2,v6)  (v6,v2,v3,v5) ... (v10,v6,v7,v11)</TT>
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createPatch(const Vec3f& origin, const Vec3f& uUnit, const Vec3f& vUnit, uint uCellCount, uint vCellCount, GeometryBuilder* builder)
{
    CVF_ASSERT(uCellCount > 0);
    CVF_ASSERT(vCellCount > 0);

    uint numVertices = (uCellCount + 1)*(vCellCount + 1);
    uint numQuads = uCellCount*vCellCount;
    
    Vec3fArray vertices;
    vertices.reserve(numVertices);

    uint u, v;
    for (v = 0; v <= vCellCount; v++)
    {
        for (u = 0; u <= uCellCount; u++)
        {
            vertices.add(origin + static_cast<float>(u)*uUnit + static_cast<float>(v)*vUnit);
        }
    }

    uint baseNodeIdx = builder->addVertices(vertices);

    UIntArray conn;
    conn.reserve(4*numQuads);

    for (v = 0; v < vCellCount; v++)
    {
        for (u = 0; u < uCellCount; u++)
        {
            conn.add(baseNodeIdx + u     + (v + 1)*(uCellCount + 1));
            conn.add(baseNodeIdx + u     + v*(uCellCount + 1));
            conn.add(baseNodeIdx + u + 1 + v*(uCellCount + 1));
            conn.add(baseNodeIdx + u + 1 + (v + 1)*(uCellCount + 1));
        }
    }

    builder->addQuads(conn);
}


//--------------------------------------------------------------------------------------------------
/// Create a 3D solid box spanning diagonally from min to max
/// 
/// \param min      The coordinate that represent one corner of the box.
/// \param max      The coordinate that lie diagonal from max on the opposite face of the box
/// \param builder  Geometry builder to use when creating geometry
///
/// This method creates a box with no shared vertices resulting in sharp corners during shading.
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createBox(const Vec3f& min, const Vec3f& max, GeometryBuilder* builder)
{
    // The ordering of the faces is consistent with GLviewAPI's hexahedron element.
    // Note that the vertex ordering within a face is not consistent 
    //
    //     7---------6                Faces:
    //    /|        /|     |z           0 bottom   0, 3, 2, 1
    //   / |       / |     | /y         1 top      4, 5, 6, 7
    //  4---------5  |     |/           2 front    4, 0, 1, 5
    //  |  3------|--2     *---x        3 right    5, 1, 2, 6
    //  | /       | /                   4 back     6, 2, 3, 7
    //  |/        |/                    5 left     7, 3, 0, 4
    //  0---------1                     

    Vec3f v0(min.x(), min.y(), min.z());
    Vec3f v1(max.x(), min.y(), min.z());
    Vec3f v2(max.x(), max.y(), min.z());
    Vec3f v3(min.x(), max.y(), min.z());
    
    Vec3f v4(min.x(), min.y(), max.z());
    Vec3f v5(max.x(), min.y(), max.z());
    Vec3f v6(max.x(), max.y(), max.z());
    Vec3f v7(min.x(), max.y(), max.z());

    builder->addQuadByVertices(v0, v3, v2, v1);
    builder->addQuadByVertices(v4, v5, v6, v7);
    builder->addQuadByVertices(v4, v0, v1, v5);
    builder->addQuadByVertices(v5, v1, v2, v6);
    builder->addQuadByVertices(v6, v2, v3, v7);
    builder->addQuadByVertices(v7, v3, v0, v4);
}


//--------------------------------------------------------------------------------------------------
/// Create a 3D solid box at the specified position and with the given total extents
/// 
/// \param centerPos  Position of center of box
/// \param extentX    Total extent of box along x-axis
/// \param extentY    Total extent of box along y-axis
/// \param extentZ    Total extent of box along z-axis
/// \param builder    Geometry builder to use when creating geometry
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createBox(const Vec3f& centerPos, float extentX, float extentY, float extentZ, GeometryBuilder* builder)
{
    Vec3f halfExtent(extentX/2, extentY/2, extentZ/2);
    Vec3f min(centerPos - halfExtent);
    Vec3f max(centerPos + halfExtent);

    createBox(min, max, builder);
}


//--------------------------------------------------------------------------------------------------
/// Create a disc centered at origin with its normal along positive z-axis
/// 
/// \param radius     Outer radius of the disc
/// \param numSlices  The number of subdivisions around the z-axis. Must be >= 4
/// \param builder    Geometry builder to use when creating geometry
///
/// Creates a disc on the z = 0 plane, centered at origin and with its surface normal pointing 
/// along the positive z-axis.
/// 
/// The disk is subdivided around the z axis into numSlices (as in pizza slices).
/// 
/// The sourceNodes that will be produced by this method:
/// <PRE>
///         1
///      /-----\ 8
///    2/\  |  /\            |y		
///    /  \ | /  \           |    
///    |   \|/   |           |    
///   3|----0----|7          |    
///    |   /|\   |           *-----x
///    \  / | \  /          /     
///    4\/  |  \/6         /z     
///      \-----/          
///         5               </PRE>
/// 
/// The following triangle connectivities will be produced:\n
/// <TT> &nbsp; &nbsp; (0,1,2)  (0,2,3)  (0,3,4) ... (0,8,1)</TT>
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createDisc(double radius, uint numSlices, GeometryBuilder* builder)
{
    CVF_ASSERT(numSlices >= 4); 
    CVF_ASSERT(builder);


    double da = 2*PI_D/numSlices;

    Vec3fArray verts;
    verts.reserve(numSlices + 1);

    // Center of disc
    verts.add(Vec3f::ZERO);


    Vec3f point = Vec3f::ZERO;

    uint i;
    for (i = 0; i < numSlices; i++) 
    {
        // Precompute this one (A = i*da;)
        double sinA = Math::sin(i*da);
        double cosA = Math::cos(i*da);

        point.x() = static_cast<float>(-sinA*radius);
        point.y() = static_cast<float>( cosA*radius);

        verts.add(point);
    }

    uint baseNodeIdx = builder->addVertices(verts);


//     Vec3fArray myArray;
//     myArray.resize(10);
//     generatePointsOnCircle(radius, numSlices, &myArray);


    uint conn[3] = { baseNodeIdx, 0, 0};

    for (i = numSlices; i > 0; i--) 
    {
        conn[1] = baseNodeIdx + i + 1;
        conn[2] = baseNodeIdx + i + 0;

        if (i == numSlices) conn[1] = baseNodeIdx + 1;

        builder->addTriangle(conn[0], conn[1], conn[2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// Create a disk with a hole in the middle
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createDisc(double outerRadius, double innerRadius, uint numSlices, GeometryBuilder* builder)
{
    CVF_ASSERT(numSlices >= 4); 
    CVF_ASSERT(builder);

    double da = 2*PI_D/numSlices;

    Vec3fArray verts;
    verts.reserve(2*numSlices);

    Vec3f point = Vec3f::ZERO;

    uint i;
    for (i = 0; i < numSlices; i++) 
    {
        // Precompute this one (A = i*da;)
        double sinA = Math::sin(i*da);
        double cosA = Math::cos(i*da);

        point.x() = static_cast<float>(-sinA*innerRadius);
        point.y() = static_cast<float>( cosA*innerRadius);

        verts.add(point);

        point.x() = static_cast<float>(-sinA*outerRadius);
        point.y() = static_cast<float>( cosA*outerRadius);

        verts.add(point);
    }

    uint baseNodeIdx = builder->addVertices(verts);

    uint conn[3] = { baseNodeIdx, 0, 0};

    for (i = 0; i < numSlices - 1; ++i) 
    {
        uint startIdx = baseNodeIdx + 2*i;

        conn[0] = startIdx + 0;
        conn[1] = startIdx + 3;
        conn[2] = startIdx + 1;
        builder->addTriangle(conn[0], conn[1], conn[2]);

        conn[0] = startIdx + 2;
        conn[1] = startIdx + 3;
        conn[2] = startIdx + 0;
        builder->addTriangle(conn[0], conn[1], conn[2]);
    }

    builder->addTriangle(baseNodeIdx + 0, baseNodeIdx + 1, baseNodeIdx + numSlices*2 - 1);
    builder->addTriangle(baseNodeIdx + 0, baseNodeIdx + numSlices*2 - 1, baseNodeIdx + numSlices*2 - 2);
}


// void GeometryUtils::generatePointsOnCircle(double radius, int numPoints, Vec3fArray* generatedPoints)
// {
//     CVF_ASSERT(generatedPoints);
//     generatedPoints->reserve(generatedPoints->size() + numPoints);
// 
//     double da = 2*PI_D/numPoints;
// 
//     Vec3f point = Vec3f::ZERO;
// 
//     int i;
//     for (i = 0; i < numPoints; i++) 
//     {
//         // Precompute this one (A = i*da;)
//         double sinA = sin(i*da);
//         double cosA = cos(i*da);
// 
//         point.x() = static_cast<float>(-sinA*radius);
//         point.y() = static_cast<float>( cosA*radius);
// 
//         generatedPoints->add(point);
//     }
// }
// 


// void GeometryUtils::createDiscUsingFan(double radius, int numSlices, GeometryBuilder* geometryBuilder)
// {
//     CVF_ASSERT(numSlices >= 4); 
//     CVF_ASSERT(geometryBuilder);
// 
// 
//     double da = 2*PI_D/numSlices;
// 
//     Vec3fArray verts;
//     verts.preAllocBuffer(numSlices + 1);
// 
//     // Center of disc
//     verts.addToPreAlloc(Vec3f::ZERO);
// 
// 
//     Vec3f point = Vec3f::ZERO;
// 
//     int i;
//     for (i = 0; i < numSlices; i++) 
//     {
//         // Precompute this one (A = i*da;)
//         double sinA = sin(i*da);
//         double cosA = cos(i*da);
// 
//         point.x() = static_cast<float>(-sinA*radius);
//         point.y() = static_cast<float>( cosA*radius);
// 
//         verts.addToPreAlloc(point);
//     }
// 
//     int baseNodeIdx = geometryBuilder->addVertices(verts);
// 
// 
//     IntArray conns;
//     conns.resize(numSlices + 2);
// 
//     for (i = 0; i < numSlices + 1; i++) 
//     {
//         conns[i] = baseNodeIdx + i;
//     }
// 
//     conns[numSlices + 1] = baseNodeIdx + 1;
// 
//     geometryBuilder->addTriangleFan(conns);
// }


//--------------------------------------------------------------------------------------------------
/// Create a sphere with center in origin
///
/// \param     radius     Radius of sphere
/// \param     numSlices  The number of subdivisions around the z-axis (similar to lines of longitude).  
/// \param     numStacks  The number of subdivisions along the z-axis (similar to lines of latitude). 
/// \param     builder    Geometry builder to use when creating geometry
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createSphere(double radius, uint numSlices, uint numStacks, GeometryBuilder* builder)
{
    // Code is strongly inspired by mesa.

    // From GLviewAPI:
    // float nsign = bNormalsOutwards ? 1.0f : -1.0f;
    // Could be added as a param if needed (e.g. dome)
    const double nsign = 1.0;

    double rho = PI_D/numStacks;
    double theta = 2.0*PI_D/static_cast<double>(numSlices);

    // Array to receive the node coordinates
    Vec3fArray vertices;
    uint vertexCount = 1 + 2*numSlices + (numStacks - 2)*numSlices;
    vertices.reserve(vertexCount);

    // Create the +Z end as triangles
    Vec3d vTop(0.0, 0.0, nsign*radius);
    vertices.add(Vec3f(vTop));

    ref<UIntArray> triangleFan = new UIntArray;
    triangleFan->reserve(numSlices + 2);
    triangleFan->add(0);

    uint j;
    for (j = 0; j < numSlices; j++) 
    {
        double localTheta = j * theta;

        Vec3d v;
        v.x() = -Math::sin(localTheta) * Math::sin(rho);
        v.y() =  Math::cos(localTheta) * Math::sin(rho);
        v.z() = nsign                  * Math::cos(rho);

        v *= radius;
        vertices.add(Vec3f(v));

        triangleFan->add(j + 1);
    }

    // Close top fan
    triangleFan->add(1);
    builder->addTriangleFan(*triangleFan);

    // Intermediate stacks as quad-strips
    // First and last stacks are handled separately

    ref<UIntArray> quadStrip = new UIntArray;
    quadStrip->reserve(numSlices*2 + 2);

    uint i;
    for (i = 1; i < numStacks - 1; i++) 
    {
        double localRho = i * rho;

        quadStrip->setSizeZero();

        for (j = 0; j < numSlices; j++) 
        {
            double localTheta = j * theta;

            Vec3d v;
            v.x() = -Math::sin(localTheta) * Math::sin(localRho + rho);
            v.y() =  Math::cos(localTheta) * Math::sin(localRho + rho);
            v.z() = nsign                  * Math::cos(localRho + rho);

            v *= radius;
            vertices.add(Vec3f(v));

            uint iC1 = (i*numSlices) + 1 + j;
            uint iC0 = iC1 - numSlices;
            quadStrip->add(iC0);
            quadStrip->add(iC1);
        }

        // Close quad-strip
        uint iStartC1 = (i*numSlices) + 1;
        uint iStartC0 = iStartC1 - numSlices;
        quadStrip->add(iStartC0);
        quadStrip->add(iStartC1);

        builder->addQuadStrip(*quadStrip);
    }

    // Create -Z end as triangles
    Vec3d vBot( 0.0, 0.0, -radius*nsign  );
    vertices.add(Vec3f(vBot));

    uint endNodeIndex = static_cast<uint>(vertices.size()) - 1;

    triangleFan->setSizeZero();
    triangleFan->add(endNodeIndex);

    for (j = 0; j < numSlices; j++) 
    {
        triangleFan->add(endNodeIndex - j - 1);
    }

    // Close bottom fan
    triangleFan->add(endNodeIndex - 1);
    builder->addTriangleFan(*triangleFan);

    builder->addVertices(vertices);
}


//--------------------------------------------------------------------------------------------------
/// Create a (possibly oblique) cylinder oriented along the z-axis
///
/// \param	bottomRadius    Bottom radius of cylinder
/// \param	topRadius       Top radius of cylinder
/// \param	height          Height of cylinder
/// \param	topOffsetX      Offset top disc relative to bottom in X direction
/// \param	topOffsetY      Offset top disc relative to bottom in Y direction
/// \param	numSlices       Number of slices
/// \param	normalsOutwards true to generate polygons with outward facing normals. 
/// \param	closedBot       true to close the bottom of the cylinder with a disc
/// \param	closedTop       true to close the top of the cylinder with a disc
/// \param	numPolysZDir    Number of (subdivisions) polygons along the Z axis.
/// \param  builder         Geometry builder to use when creating geometry
///
/// An oblique cylinder is a cylinder with bases that are not aligned one directly above the other
///	The base of the cylinder is placed at z = 0, and the top at z = height. 
///	Cylinder is subdivided around the z-axis into slices.
///	Use the cone functions instead of setting one of the radius params to 0
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createObliqueCylinder(float bottomRadius, float topRadius, float height, float topOffsetX, float topOffsetY, uint numSlices, bool normalsOutwards, bool closedBot, bool closedTop, uint numPolysZDir, GeometryBuilder* builder)
{
    // Create cylinder...
    Vec3f centBot(0, 0, 0);
    Vec3f centTop(topOffsetX, topOffsetY, height);

    // Create vertices
    uint zPoly;
    for (zPoly = 0; zPoly <= numPolysZDir; zPoly++)
    {
        float fT = static_cast<float>((1.0/numPolysZDir)*(zPoly));
        float radius = bottomRadius + fT*(topRadius - bottomRadius);
        Vec3f center(fT*topOffsetX, fT*topOffsetY, fT*height);

        Vec3fArray verts;
        verts.reserve(numSlices);
        Vec3f point = Vec3f::ZERO;

        double da = 2*PI_D/numSlices;
        uint i;
        for (i = 0; i < numSlices; i++) 
        {
            // Precompute this one (A = i*da;)
            double sinA = Math::sin(i*da);
            double cosA = Math::cos(i*da);

            point.x() = static_cast<float>(-sinA*radius);
            point.y() = static_cast<float>( cosA*radius);
            point.z() = 0;

            point += center;

            verts.add(point);
        }

        uint baseNodeIdx = builder->addVertices(verts);

        // First time we only create the sourceNodes
        if (zPoly != 0)
        {
            uint offset = baseNodeIdx - numSlices;
            uint piConn[4] = { 0, 0, 0, 0 };

            // Normals facing outwards
            if (normalsOutwards)
            {
                uint i;
                for (i = 0; i < numSlices; i++) 
                {
                    piConn[0] = offset + i;
                    piConn[1] = offset + i + 1;
                    piConn[2] = offset + i + numSlices + 1;
                    piConn[3] = offset + i + numSlices;

                    if (i == numSlices - 1) 
                    {
                        piConn[1] = offset;
                        piConn[2] = offset + numSlices;
                    }

                    builder->addQuad(piConn[0], piConn[1], piConn[2], piConn[3]);
                }
            }

            // Normals facing inwards
            else
            {
                uint i;
                for (i = 0; i < numSlices; i++) 
                {
                    piConn[0] = offset + i + 1;
                    piConn[1] = offset + i;
                    piConn[2] = offset + i + numSlices;
                    piConn[3] = offset + i + numSlices + 1;

                    if (i == numSlices - 1) 
                    {
                        piConn[0] = offset;
                        piConn[3] = offset + numSlices;
                    }

                    builder->addQuad(piConn[0], piConn[1], piConn[2], piConn[3]);
                }
            }
        }
    }

    if (closedBot)
    {
        createDisc(bottomRadius, numSlices, builder);
    }

    if (closedTop)
    {
        uint startIdx = builder->vertexCount();
        createDisc(topRadius, numSlices, builder);
        uint endIdx = builder->vertexCount() - 1;

        // Translate the top disc sourceNodes, also flip it to get the normals the right way
        Mat4f mat = Mat4f::fromRotation(Vec3f(1.0f, 0.0f, 0.0f), Math::toRadians(180.0f));
        mat.translatePreMultiply(Vec3f(topOffsetX, topOffsetY, height));

        builder->transformVertexRange(startIdx, endIdx, mat);
    }
}

//--------------------------------------------------------------------------------------------------
/// Create a cone oriented along the z-axis
///
/// \param	bottomRadius    Bottom radius of cone
/// \param	height          Height of cone
/// \param	numSlices       Number of slices
/// \param	normalsOutwards true to generate polygons with outward facing normals. 
/// \param	closedBot       true to close the bottom of the cone with a disc
/// \param	singleTopNode   Specify if a single top node should be used, or if each side triangle 
///                         should have its own top node.
/// \param  builder         Geometry builder to use when creating geometry
///
//--------------------------------------------------------------------------------------------------
void GeometryUtils::createCone(float bottomRadius, float height, uint numSlices, bool normalsOutwards, bool closedBot, bool singleTopNode, GeometryBuilder* builder)
{
    Vec3fArray verts;

    if (singleTopNode)
    {
        verts.reserve(numSlices + 1);
    }
    else
    {
        verts.reserve(numSlices*2);
    }

    Vec3f point = Vec3f::ZERO;

    double da = 2*PI_D/numSlices;
    uint i;
    for (i = 0; i < numSlices; i++) 
    {
        // Precompute this one (A = i*da;)
        double sinA = Math::sin(i*da);
        double cosA = Math::cos(i*da);

        point.x() = static_cast<float>(-sinA*bottomRadius);
        point.y() = static_cast<float>( cosA*bottomRadius);

        verts.add(point);
    }

    if (singleTopNode)
    {
        verts.add(Vec3f(0, 0, height));
    }
    else
    {
        // Unique sourceNodes at apex of cone
        Vec3f topNode(0, 0, height);

        uint i;
        for (i = 0; i < numSlices; i++)
        {
            verts.add(topNode);
        }
    }

    uint baseNodeIdx = builder->addVertices(verts);

    uint piConn[3] = { 0, 0, 0 };

    // Normals facing outwards
    if (normalsOutwards)
    {
        uint i;
        for (i = 0; i < numSlices; i++) 
        {
            piConn[0] = baseNodeIdx + i;
            piConn[1] = baseNodeIdx + i + 1;
            piConn[2] = singleTopNode ? baseNodeIdx + numSlices : baseNodeIdx + i + numSlices; 

            if (i == numSlices - 1) 
            {
                piConn[1] = baseNodeIdx;
            }
            
            if (normalsOutwards)
            {
                builder->addTriangle(piConn[0], piConn[1], piConn[2]);
            }
            else
            {
                builder->addTriangle(piConn[1], piConn[0], piConn[2]);
            }
        }
    }

    if (closedBot)
    {
        createDisc(bottomRadius, numSlices, builder);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryUtils::tesselatePatchAsQuads(uint pointCountU, uint pointCountV, uint indexOffset, bool windingCCW, UIntArray* indices)
{
    CVF_ASSERT(pointCountU >= 2);
    CVF_ASSERT(pointCountV >= 2);

    uint uCellCount = pointCountU - 1;
    uint vCellCount = pointCountV - 1;

    uint numQuads = uCellCount*vCellCount;
    indices->reserve(indices->size() + 4*numQuads);

    uint u, v;
    for (v = 0; v < vCellCount; v++)
    {
        for (u = 0; u < uCellCount; u++)
        {
            if (windingCCW)
            {
                indices->add(indexOffset +  u    +  (v+1)*pointCountU);
                indices->add(indexOffset +  u    +      v*pointCountU);
                indices->add(indexOffset +  u+1  +      v*pointCountU);
                indices->add(indexOffset +  u+1  +  (v+1)*pointCountU);
            }
            else
            {
                indices->add(indexOffset +  u    +      v*pointCountU);
                indices->add(indexOffset +  u    +  (v+1)*pointCountU);
                indices->add(indexOffset +  u+1  +  (v+1)*pointCountU);
                indices->add(indexOffset +  u+1  +      v*pointCountU);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryUtils::tesselatePatchAsTriangles(uint pointCountU, uint pointCountV, uint indexOffset, bool windingCCW, UIntArray* indices)
{
    CVF_ASSERT(pointCountU >= 2);
    CVF_ASSERT(pointCountV >= 2);

    uint uCellCount = pointCountU - 1;
    uint vCellCount = pointCountV - 1;

    uint numTris = 2*uCellCount*vCellCount;
    indices->reserve(indices->size() + 3*numTris);

    uint u, v;
    for (v = 0; v < vCellCount; v++)
    {
        for (u = 0; u < uCellCount; u++)
        {
            if (windingCCW)
            {
                indices->add(indexOffset +  u    +  (v+1)*pointCountU);
                indices->add(indexOffset +  u    +      v*pointCountU);
                indices->add(indexOffset +  u+1  +      v*pointCountU);

                indices->add(indexOffset +  u    +  (v+1)*pointCountU);
                indices->add(indexOffset +  u+1  +      v*pointCountU);
                indices->add(indexOffset +  u+1  +  (v+1)*pointCountU);
            }
            else
            {
                indices->add(indexOffset +  u    +      v*pointCountU);
                indices->add(indexOffset +  u    +  (v+1)*pointCountU);
                indices->add(indexOffset +  u+1  +  (v+1)*pointCountU);

                indices->add(indexOffset +  u    +      v*pointCountU);
                indices->add(indexOffset +  u+1  +  (v+1)*pointCountU);
                indices->add(indexOffset +  u+1  +      v*pointCountU);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Check if the specified quad is convex
//--------------------------------------------------------------------------------------------------
bool GeometryUtils::isConvexQuad(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d)
{
    // From "Real Time Collision Detection", p60
    // Quad is nonconvex if dot(cross(bd, ba), cross(bd, bc)) >= 0
    const Vec3f bda = (d - b) ^ (a - b);
    const Vec3f bdc = (d - b) ^ (c - b);
    if (bda*bdc >= 0) 
    {
        return false;
    }

    // Quad is now convex if dot(cross(ac, ad), cross(ac, ab)) < 0
    const Vec3f acd = (c - a) ^ (d - a);
    const Vec3f acb = (c - a) ^ (b - a);
    if (acd*acb < 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Compute surface normal for a quad
//--------------------------------------------------------------------------------------------------
Vec3f GeometryUtils::quadNormal(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d)
{
    // From "Real Time Collision Detection", p. 495
    Vec3f normal = (c - a) ^ (d - b);
    normal.normalize();

    return normal;
}


//--------------------------------------------------------------------------------------------------
/// Compute polygon normal using Newell's method
//--------------------------------------------------------------------------------------------------
Vec3f GeometryUtils::polygonNormal(const Vec3fValueArray& vertices, const uint* indices, uint indexCount)
{
    // From "Real Time Collision Detection", p. 495
    // Compute normal as being proportional to projected areas of polygon onto the yz, xz, and xy planes. 
    Vec3f normal(0, 0, 0);

    uint n;
    for (n = 0; n < indexCount; n++) 
    {
        uint i = indices[(n > 0) ? n - 1 : indexCount - 1];
        uint j = indices[n];

        normal.x() += (vertices.val(i).y() - vertices.val(j).y()) * (vertices.val(i).z() + vertices.val(j).z()); // projection on yz
        normal.y() += (vertices.val(i).z() - vertices.val(j).z()) * (vertices.val(i).x() + vertices.val(j).x()); // projection on xz
        normal.z() += (vertices.val(i).x() - vertices.val(j).x()) * (vertices.val(i).y() + vertices.val(j).y()); // projection on xy
    }

    normal.normalize();
    return normal;
}

//--------------------------------------------------------------------------------------------------
/// Compact an array of vertex indices by removing 'unused' indices
/// 
/// \param[in]  vertexIndices     The original vertex indices
/// \param[out] newVertexIndices  New compacted vertex indices  
/// \param[out] newToOldMapping   For each 'new' vertex, will contain its original index
/// \param[in]  maxVertexCount    The maximum resulting vertex count after removing unused vertices
//--------------------------------------------------------------------------------------------------
void GeometryUtils::removeUnusedVertices(const UIntValueArray& vertexIndices, UIntArray* newVertexIndices, UIntArray* newToOldMapping, uint maxVertexCount)
{
    if (vertexIndices.size() == 0 || maxVertexCount == 0)
    {
        return;
    }

    std::map<uint, uint> oldToNewVertexIndexMap;
    std::map<uint, uint>::const_iterator it;

    CVF_ASSERT(newVertexIndices);
    CVF_ASSERT(newToOldMapping);
    newVertexIndices->reserve(vertexIndices.size());
    newToOldMapping->reserve(maxVertexCount);

    size_t i;
    uint newVertexIndex = 0;
    for (i = 0; i < vertexIndices.size(); i++)
    {
        uint vertexIdx = vertexIndices.val(i);

        uint currentIndex = UNDEFINED_UINT;

        it = oldToNewVertexIndexMap.find(vertexIdx);
        if (it == oldToNewVertexIndexMap.end())
        {
            currentIndex = newVertexIndex++;
            oldToNewVertexIndexMap[vertexIdx] = currentIndex;
            newToOldMapping->add(vertexIdx);
        }
        else
        {
            currentIndex = it->second;
        }

        CVF_ASSERT(currentIndex != UNDEFINED_UINT);
        newVertexIndices->add(currentIndex);
    }

    newToOldMapping->squeeze();
}

bool GeometryUtils::project(const Mat4d& projectionMultViewMatrix, const Vec2i& viewportPosition, const Vec2ui& viewportSize, const Vec3d& point, Vec3d* out)
{
    CVF_ASSERT(out);

    Vec4d v = projectionMultViewMatrix * Vec4d(point, 1.0);

    if (v.w() == 0.0f)
    {
        return false;
    }

    v.x() /= v.w();
    v.y() /= v.w();
    v.z() /= v.w();

    // map to range 0-1
    out->x() = v.x()*0.5 + 0.5;
    out->y() = v.y()*0.5 + 0.5;
    out->z() = v.z()*0.5 + 0.5;

    // map to viewport
    out->x() = out->x() * viewportSize.x() + viewportPosition.x();
    out->y() = out->y() * viewportSize.y() + viewportPosition.y();

    return true;
}

} // namespace cvf
