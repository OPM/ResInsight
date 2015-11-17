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

#include "RivCrossSectionGeometryGenerator.h"
#include "cvfBoundingBox.h"
#include "RigMainGrid.h"
#include "cvfDrawableGeo.h"
#include "RigResultAccessor.h"
#include "cvfScalarMapper.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivCrossSectionGeometryGenerator::RivCrossSectionGeometryGenerator(const std::vector<cvf::Vec3d> &polyline, 
                                                                   const cvf::Vec3d& extrusionDirection, 
                                                                   const RigMainGrid* grid)
                                                                   : m_polyLine(polyline), 
                                                                   m_extrusionDirection(extrusionDirection), 
                                                                   m_mainGrid(grid)
{
    m_triangleVxes = new cvf::Vec3fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivCrossSectionGeometryGenerator::~RivCrossSectionGeometryGenerator()
{

}


//--------------------------------------------------------------------------------------------------
/// Find intersection between a line segment and a plane
///
/// \param a            Start of line segment
/// \param b            End of line segment
/// \param intersection Returns intersection point along the infinite line defined by a-b
/// \param normalizedDistFromA Returns the normalized (0..1) position from a to b of the intersection point. 
///                            Will return values along the complete line, and HUGE_VAL if plane and line are parallel. 
///
/// \return True if line segment intersects the plane
//--------------------------------------------------------------------------------------------------
bool planeLineIntersect(const cvf::Plane& plane, const cvf::Vec3d& a, const cvf::Vec3d& b, cvf::Vec3d* intersection, double* normalizedDistFromA)
{
    // From Real-Time Collision Detection by Christer Eriscon, published by Morgen Kaufmann Publishers, (c) 2005 Elsevier Inc

    cvf::Vec3d ab = b - a;
    cvf::Vec3d normal = plane.normal();

    double normDotAB = normal * ab;
    if (normDotAB == 0)
    {
        (*normalizedDistFromA) = HUGE_VAL;
        return false;
    }

    double interpolationParameter = (-plane.D() - (normal * a)) / normDotAB;

    (*intersection) = a + interpolationParameter * ab;
    (*normalizedDistFromA) = interpolationParameter;

    return (interpolationParameter >= 0.0 && interpolationParameter <= 1.0);
}


struct ClipVx 
{
    cvf::Vec3d vx;
    double normDistFromEdgeVx1;
    int clippedEdgeVx1Id;
    int clippedEdgeVx2Id;
};

//--------------------------------------------------------------------------------------------------
/// Returns whether the triangle was hit by the plane
/// isMostVxesOnPositiveSide returns true if all or two of the vxes is on the positive side of the plane
/// newVx1/2.vx1ClippedEdge returns the index of the single vx that is alone on one side
/// Going newVx1 to newVx2 will make the top triangle same winding as the original triangle, 
/// and thus the quad opposite winding

// Except for the trivial cases where all vertices are in front 
// or behind plane, these are the permutations
//
// Single vertex on positive side of plane 
// => return a triangle                                
//
//  +\   /\c               /\c   /+          /\c        .
//    \ /  \              /  \  /       +   /  \   +    .
//     \    \            /    \/        ---/----\---    .
//    / \    \          /     /\          /      \      .
//  a/___\____\b      a/_____/__\b      a/________\b    .
//       +\                 /+
//
// Two vertices vertex on positive side of plane 
// => return a quad
//
//     \+  /\c               /\c  +/        /\c         .   
//      \ /  \              /  \  /        /  \         .  
//       \    \            /    \/     ___/____\___     .
//      / \    \          /     /\     + /      \ +     .
//    a/___\____\b      a/_____/__\b   a/________\b     .        
//          \+               +/                       
//--------------------------------------------------------------------------------------------------

bool planeTriangleIntersection(const cvf::Plane& plane, 
                               const cvf::Vec3d& p1, int p1Id, 
                               const cvf::Vec3d& p2, int p2Id,  
                               const cvf::Vec3d& p3, int p3Id, 
                               ClipVx* newVx1, ClipVx* newVx2, 
                               bool * isMostVxesOnPositiveSide)
{
    int onPosSide[3];
    onPosSide[0] = plane.distanceSquared(p1) >= 0 ;
    onPosSide[1] = plane.distanceSquared(p2) >= 0 ;
    onPosSide[2] = plane.distanceSquared(p3) >= 0 ;

    const int numPositiveVertices = onPosSide[0] + onPosSide[1] + onPosSide[2];
    
    // The entire triangle is on the negative side
    // Clip everything
    if (numPositiveVertices == 0)
    {
        (*isMostVxesOnPositiveSide) = false;
        return false;
    }

    // All triangle vertices are on the positive side
    if (numPositiveVertices == 3) 
    {
        (*isMostVxesOnPositiveSide) = true;
        return false;
    }

    (*isMostVxesOnPositiveSide) = (numPositiveVertices == 2);

    int topVx = 0;
    if (numPositiveVertices == 1)
    {
        if (onPosSide[0]) topVx = 1;
        if (onPosSide[1]) topVx = 2;
        if (onPosSide[2]) topVx = 3;
    }
    else if (numPositiveVertices == 2)
    {
        if (!onPosSide[0]) topVx = 1;
        if (!onPosSide[1]) topVx = 2;
        if (!onPosSide[2]) topVx = 3;
    }
    else
    {
        CVF_ASSERT(false);
    }

    cvf::Vec3d intersection; 
    double normDistFromP1 = 0;
    if (topVx = 1)
    {
        planeLineIntersect(plane, p1, p2, &((*newVx1).vx), &((*newVx1).normDistFromEdgeVx1));
        (*newVx1).clippedEdgeVx1Id = p1Id;
        (*newVx1).clippedEdgeVx1Id = p2Id;
        planeLineIntersect(plane, p1, p3, &((*newVx2).vx), &((*newVx2).normDistFromEdgeVx1));
        (*newVx1).clippedEdgeVx1Id = p1Id;
        (*newVx1).clippedEdgeVx1Id = p3Id;
    }
    else if (topVx = 2)
    {
        planeLineIntersect(plane, p2, p3, &((*newVx1).vx), &((*newVx1).normDistFromEdgeVx1));
        (*newVx1).clippedEdgeVx1Id = p2Id;
        (*newVx1).clippedEdgeVx1Id = p3Id;
        planeLineIntersect(plane, p2, p1, &((*newVx2).vx), &((*newVx2).normDistFromEdgeVx1));
        (*newVx1).clippedEdgeVx1Id = p2Id;
        (*newVx1).clippedEdgeVx1Id = p1Id;
    }
    else if (topVx = 3)
    {
        planeLineIntersect(plane, p3, p1, &((*newVx1).vx), &((*newVx1).normDistFromEdgeVx1));
        (*newVx1).clippedEdgeVx1Id = p3Id;
        (*newVx1).clippedEdgeVx1Id = p1Id;
        planeLineIntersect(plane, p3, p2, &((*newVx2).vx), &((*newVx2).normDistFromEdgeVx1));
        (*newVx1).clippedEdgeVx1Id = p3Id;
        (*newVx1).clippedEdgeVx1Id = p2Id;
    }
    else
    {
        CVF_ASSERT(false);
    }
    
    return true;  
}

#if 0
bool planeTriangleIntersection(const cvf::Plane& plane, 
                               const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3, 
                               ClipVx* newVx1, ClipVx* newVx2)
{
}
#endif



#if 0
static void planeHexCellIntersectionOnTriangles(const cvf::Plane& plane, 
                                                const cvf::Vec3d hexCorners[8], 
                                                const int hexCornersIds[8],
                                                std::vector<ClipVx>* polygon)
{
    CVF_ASSERT(polygon != NULL);

    int intersectionCount = 0;

    std::vector<std::pair<ClipVx, ClipVx> > polygonEdges;
    for (int face = 0; face < 6 ; ++face)
    {
        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);

        ClipVx newVx1;
        ClipVx newVx2;

        bool isClipped = planeTriangleIntersection(plane,
                                                   hexCorners[faceVertexIndices[0]], hexCornersIds[faceVertexIndices[0]],
                                                   hexCorners[faceVertexIndices[1]], hexCornersIds[faceVertexIndices[1]],
                                                   hexCorners[faceVertexIndices[2]], hexCornersIds[faceVertexIndices[2]],
                                                   &newVx1, &newVx2);
        if (isClipped)
        {
            polygonEdges.push_back(std::make_pair(newVx1, newVx2));
        }

        isClipped = planeTriangleIntersection(plane,
                                                   hexCorners[faceVertexIndices[0]], hexCornersIds[faceVertexIndices[0]],
                                                   hexCorners[faceVertexIndices[2]], hexCornersIds[faceVertexIndices[2]],
                                                   hexCorners[faceVertexIndices[3]], hexCornersIds[faceVertexIndices[3]],
                                                   &newVx1, &newVx2);
        if (isClipped)
        {
            polygonEdges.push_back(std::make_pair(newVx1, newVx2));
        }

    }

    if (polygonEdges.size >= 3)
    {
    // Sort polygon edges in polygon order.
    // Do this by finding polygon vxes cutting the same edge.
    // And grow from the first edge
     (*polygon).push_back(polygonEdges[0].first);
     (*polygon).push_back(polygonEdges[0].second);
     //for ()
   }
}
#endif

//--------------------------------------------------------------------------------------------------
/// Will return the intersection point. If the plane is outside the line, it returns the closest line endpoint
//--------------------------------------------------------------------------------------------------
cvf::Vec3d planeLineIntersectionForMC(const cvf::Plane& plane, const cvf::Vec3d& p1, const cvf::Vec3d& p2, double* normalizedDistFromP1)
{
    // From http://local.wasp.uwa.edu.au/~pbourke/geometry/planeline/
    //
    // P1 (x1,y1,z1) and P2 (x2,y2,z2)
    //
    // P = P1 + u (P2 - P1)
    //
    //          A*x1 + B*y1 + C*z1 + D
    // u = ---------------------------------
    //     A*(x1-x2) + B*(y1-y2) + C*(z1-z2)

    CVF_TIGHT_ASSERT(normalizedDistFromP1);

    const cvf::Vec3d v = p2 - p1;

    (*normalizedDistFromP1) = 0.0;

    double denominator = -(plane.A()*v.x() + plane.B()*v.y() + plane.C()*v.z());
    if (denominator != 0)
    {
        double u = (plane.A()*p1.x() + plane.B()*p1.y() + plane.C()*p1.z() + plane.D())/denominator;
        (*normalizedDistFromP1) = u;
        if (u > 0.0 && u < 1.0)
        {
            return (p1 + u*v);
        }
        else
        {
            if (u >= 1.0)
            {
                return p2;
            }
            else
            {
                return p1;
            }
        }
    }
    else
    {
        return p1;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int planeHexIntersectionMC(const cvf::Plane& plane, 
                            const cvf::Vec3d cell[8], 
                            const int hexCornersIds[8], 
                            std::vector<ClipVx>* triangles)
{

// Based on description and implementation from Paul Bourke:
//   http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/

    static const uint cubeIdxToCutEdgeBitfield[256] =
    {
        0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
        0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
        0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
        0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
        0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
        0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
        0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
        0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
        0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
        0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
        0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
        0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
        0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
        0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
        0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
        0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
        0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
        0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
        0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
        0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
        0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
        0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
        0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
        0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
        0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
        0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
        0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
        0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
        0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
        0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
        0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
        0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
    };

    static const int cubeIdxToTriangleIndices[256][16] =
    {
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
        { 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 },
        { 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 },
        { 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
        { 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 },
        { 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 },
        { 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 },
        { 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
        { 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 },
        { 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 },
        { 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
        { 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 },
        { 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 },
        { 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 },
        { 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 },
        { 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 },
        { 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 },
        { 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
        { 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 },
        { 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
        { 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 },
        { 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 },
        { 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 },
        { 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
        { 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 },
        { 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 },
        { 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
        { 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 },
        { 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 },
        { 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 },
        { 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
        { 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 },
        { 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 },
        { 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
        { 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 },
        { 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 },
        { 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 },
        { 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
        { 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 },
        { 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 },
        { 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
        { 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 },
        { 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
        { 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
        { 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 },
        { 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 },
        { 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 },
        { 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 },
        { 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 },
        { 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 },
        { 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 },
        { 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 },
        { 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 },
        { 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
        { 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 },
        { 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 },
        { 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 },
        { 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 },
        { 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 },
        { 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 },
        { 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 },
        { 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 },
        { 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 },
        { 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 },
        { 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 },
        { 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 },
        { 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 },
        { 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 },
        { 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 },
        { 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 },
        { 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 },
        { 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 },
        { 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 },
        { 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 },
        { 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 },
        { 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 },
        { 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 },
        { 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 },
        { 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 },
        { 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 },
        { 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 },
        { 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 },
        { 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 },
        { 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 },
        { 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 },
        { 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 },
        { 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 },
        { 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 },
        { 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 },
        { 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 },
        { 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
        { 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 },
        { 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 },
        { 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 },
        { 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 },
        { 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 },
        { 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 },
        { 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 },
        { 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 },
        { 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 },
        { 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 },
        { 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 },
        { 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 },
        { 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 },
        { 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 },
        { 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 },
        { 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
        { 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 },
        { 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 },
        { 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 },
        { 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 },
        { 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 },
        { 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 },
        { 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
        { 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 },
        { 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
    };

    static const int edgeTable[12][2] =
    {
       {0, 1},
       {1, 2},
       {2, 3},
       {3, 0},
       {4, 5},
       {5, 6},
       {6, 7},
       {7, 4},
       {0, 4},
       {1, 5},
       {2, 6},
       {3, 7}
    };

    int cubeIndex = 0;
    if (plane.distanceSquared(cell[0]) < 0) cubeIndex |= 1;
    if (plane.distanceSquared(cell[1]) < 0) cubeIndex |= 2;
    if (plane.distanceSquared(cell[2]) < 0) cubeIndex |= 4;
    if (plane.distanceSquared(cell[3]) < 0) cubeIndex |= 8;
    if (plane.distanceSquared(cell[4]) < 0) cubeIndex |= 16;
    if (plane.distanceSquared(cell[5]) < 0) cubeIndex |= 32;
    if (plane.distanceSquared(cell[6]) < 0) cubeIndex |= 64;
    if (plane.distanceSquared(cell[7]) < 0) cubeIndex |= 128;

    if (cubeIdxToCutEdgeBitfield[cubeIndex] == 0)
    {
        return 0;
    }

    cvf::Vec3d edgeIntersections[12];
    double     normDistAlongEdge[12];


    // Compute vertex coordinates on the edges where we have intersections
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 1)    edgeIntersections[0] =   planeLineIntersectionForMC(plane, cell[0], cell[1],  &normDistAlongEdge[0] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 2)    edgeIntersections[1] =   planeLineIntersectionForMC(plane, cell[1], cell[2],  &normDistAlongEdge[1] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 4)    edgeIntersections[2] =   planeLineIntersectionForMC(plane, cell[2], cell[3],  &normDistAlongEdge[2] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 8)    edgeIntersections[3] =   planeLineIntersectionForMC(plane, cell[3], cell[0],  &normDistAlongEdge[3] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 16)   edgeIntersections[4] =   planeLineIntersectionForMC(plane, cell[4], cell[5],  &normDistAlongEdge[4] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 32)   edgeIntersections[5] =   planeLineIntersectionForMC(plane, cell[5], cell[6],  &normDistAlongEdge[5] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 64)   edgeIntersections[6] =   planeLineIntersectionForMC(plane, cell[6], cell[7],  &normDistAlongEdge[6] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 128)  edgeIntersections[7] =   planeLineIntersectionForMC(plane, cell[7], cell[4],  &normDistAlongEdge[7] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 256)  edgeIntersections[8] =   planeLineIntersectionForMC(plane, cell[0], cell[4],  &normDistAlongEdge[8] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 512)  edgeIntersections[9] =   planeLineIntersectionForMC(plane, cell[1], cell[5],  &normDistAlongEdge[9] );
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 1024) edgeIntersections[10] =  planeLineIntersectionForMC(plane, cell[2], cell[6],  &normDistAlongEdge[10]);
    if (cubeIdxToCutEdgeBitfield[cubeIndex] & 2048) edgeIntersections[11] =  planeLineIntersectionForMC(plane, cell[3], cell[7],  &normDistAlongEdge[11]);


    // Create the triangles

    const int* triConnects = cubeIdxToTriangleIndices[cubeIndex];
    uint n = 0;
    int edgeIdx = triConnects[n];
    while (edgeIdx != -1)
    {
        ClipVx cvx;
        cvx.vx = edgeIntersections[edgeIdx];
        cvx.normDistFromEdgeVx1 = normDistAlongEdge[edgeIdx];
        cvx.clippedEdgeVx1Id = hexCornersIds[edgeTable[edgeIdx][0]];
        cvx.clippedEdgeVx2Id = hexCornersIds[edgeTable[edgeIdx][1]];

        (*triangles).push_back(cvx);
        ++n;
        edgeIdx = triConnects[n];
    }

    uint numTriangles = n/3;

    return numTriangles;
}


void RivCrossSectionGeometryGenerator::calculateArrays()
{
    if (m_triangleVxes->size()) return;

    std::vector<cvf::Vec3f> triangleVertices;
    cvf::Vec3d displayOffset = m_mainGrid->displayModelOffset();

    cvf::BoundingBox gridBBox = m_mainGrid->boundingBox();
    m_extrusionDirection.normalize();

    size_t lineCount = m_polyLine.size();
    for (size_t lIdx = 0; lIdx < lineCount - 1; ++lIdx)
    {
        cvf::Vec3d p1 = m_polyLine[lIdx];
        cvf::Vec3d p2 = m_polyLine[lIdx+1];

        cvf::BoundingBox sectionBBox;
        sectionBBox.add(p1);
        sectionBBox.add(p1);
        double maxSectionHeight = gridBBox.radius();
        sectionBBox.add(p1 + m_extrusionDirection*maxSectionHeight);
        sectionBBox.add(p1 - m_extrusionDirection*maxSectionHeight);
        sectionBBox.add(p2 + m_extrusionDirection*maxSectionHeight);
        sectionBBox.add(p2 - m_extrusionDirection*maxSectionHeight);
        
        std::vector<size_t> columnCellCandidates;
        m_mainGrid->findIntersectingCells(sectionBBox, &columnCellCandidates);

        cvf::Plane plane;
        plane.setFromPoints(p1, p2, p2 + m_extrusionDirection*maxSectionHeight);

        cvf::Plane p1Plane;
        p1Plane.setFromPoints(p1, p1 + m_extrusionDirection*maxSectionHeight, p1 + plane.normal());
        cvf::Plane p2Plane;
        p2Plane.setFromPoints(p2, p2 + m_extrusionDirection*maxSectionHeight, p2 + plane.normal() );
        
        
        std::vector<ClipVx> triangleVxes;
        triangleVxes.reserve(5*3);

        for (size_t cccIdx = 0; cccIdx < columnCellCandidates.size(); ++cccIdx)
        {
            size_t globalCellIdx = columnCellCandidates[cccIdx];

            if (m_mainGrid->cells()[globalCellIdx].isInvalid()) continue;
            
            triangleVxes.clear();
            cvf::Vec3d cellCorners[8];
            m_mainGrid->cellCornerVertices(globalCellIdx, cellCorners);

            const caf::SizeTArray8& cornerIndices = m_mainGrid->cells()[globalCellIdx].cornerIndices();
            int hexCornersIds[8];
            hexCornersIds[0] = static_cast<int>(cornerIndices[0]);
            hexCornersIds[1] = static_cast<int>(cornerIndices[1]);
            hexCornersIds[2] = static_cast<int>(cornerIndices[2]);
            hexCornersIds[3] = static_cast<int>(cornerIndices[3]);
            hexCornersIds[4] = static_cast<int>(cornerIndices[4]);
            hexCornersIds[5] = static_cast<int>(cornerIndices[5]);
            hexCornersIds[6] = static_cast<int>(cornerIndices[6]);
            hexCornersIds[7] = static_cast<int>(cornerIndices[7]);

            int triangleCount = planeHexIntersectionMC(plane,
                                                        cellCorners,
                                                        hexCornersIds,
                                                        &triangleVxes);

            if (triangleCount)
            {
                #if 1
                for (int tIdx = 0; tIdx < triangleCount; ++tIdx)
                {
                    // Accumulate to geometry
                    int triVxIdx = tIdx*3;
                    triangleVertices.push_back(cvf::Vec3f(triangleVxes[triVxIdx+0].vx - displayOffset));
                    triangleVertices.push_back(cvf::Vec3f(triangleVxes[triVxIdx+1].vx - displayOffset));
                    triangleVertices.push_back(cvf::Vec3f(triangleVxes[triVxIdx+2].vx - displayOffset));

                    m_triangleToCellIdxMap.push_back(globalCellIdx);
                }
                #else

                // Clip triangles with plane 1
                std::vector<ClipVx> clippedTriangleVxes;
                for (int triVxIdx = 0; tIdx < triangleCount; ++tIdx)
                {
                    ClipVx newVx1;
                    ClipVx newVx2;
                    bool isMostVxesOnPositiveSide = false;
                    int triVxIdx = tIdx*3;
                    bool isIntersecting = planeTriangleIntersection(p1Plane,
                                                                    triangleVxes[triVxIdx + 0].vx, triVxIdx + 0,
                                                                    triangleVxes[triVxIdx + 1].vx, triVxIdx + 1,
                                                                    triangleVxes[triVxIdx + 2].vx, triVxIdx + 2,
                                                                    &newVx1, &newVx2, &isMostVxesOnPositiveSide);

                    if (!isIntersecting && !isMostVxesOnPositiveSide)
                    {
                        // Discard triangle
                    }
                    else if (!isIntersecting && isMostVxesOnPositiveSide)
                    {
                        // Keep the triangle
                        clippedTriangleVxes.push_back(triangleVxes[triVxIdx + 0]);
                        clippedTriangleVxes.push_back(triangleVxes[triVxIdx + 1]);
                        clippedTriangleVxes.push_back(triangleVxes[triVxIdx + 2]);
                    }
                    else if (isIntersecting && isMostVxesOnPositiveSide)
                    {
                        // Split the resulting quad and add the two triangles
                        clippedTriangleVxes.push_back(newVx2);
                        clippedTriangleVxes.push_back(newVx1);
                        clippedTriangleVxes.push_back(triangleVxes[newVx1.clippedEdgeVx2Id]);

                        clippedTriangleVxes.push_back(triangleVxes[newVx1.clippedEdgeVx2Id]);
                        clippedTriangleVxes.push_back(triangleVxes[newVx2.clippedEdgeVx2Id]);
                        clippedTriangleVxes.push_back(newVx2);

                    }
                    else if (isIntersecting && !isMostVxesOnPositiveSide)
                    {
                        // Add the top triangle
                        clippedTriangleVxes.push_back(newVx1);
                        clippedTriangleVxes.push_back(newVx2);
                        clippedTriangleVxes.push_back(triangleVxes[newVx1.clippedEdgeVx1Id]);

                    }
                }
                
                // Clip triangles with plane 2
                for (int triVxIdx = 0; tIdx < triangleCount; ++tIdx)
                {


                }

                for (int triVxIdx = 0; tIdx < triangleCount; ++tIdx)
                {

                    // Accumulate to geometry
                   
                }
                #endif
            }
        }

    }

    m_triangleVxes->assign(triangleVertices);
}


//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivCrossSectionGeometryGenerator::generateSurface()
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
cvf::ref<cvf::DrawableGeo> RivCrossSectionGeometryGenerator::createMeshDrawable()
{
    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    return geo;
}


//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimentional texture. 
/// Undefined values are coded with a y-texturecoordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivCrossSectionGeometryGenerator::textureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                          const RigResultAccessor* resultAccessor, 
                                                          const cvf::ScalarMapper* mapper) const
{
    if (!resultAccessor) return;

    size_t numVertices = m_triangleVxes->size();

    textureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    double cellScalarValue;
    cvf::Vec2f texCoord;
    
    int triangleCount = static_cast<int>(m_triangleToCellIdxMap.size());

#pragma omp parallel for private(texCoord, cellScalarValue)
    for (int tIdx = 0; tIdx < triangleCount; tIdx++)
    {
        cellScalarValue = resultAccessor->cellScalarGlobIdx(m_triangleToCellIdxMap[tIdx]);
        texCoord = mapper->mapToTextureCoord(cellScalarValue);
        if (cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for (j = 0; j < 3; j++)
        {   
            rawPtr[tIdx*3 + j] = texCoord;
        }
    }
}
