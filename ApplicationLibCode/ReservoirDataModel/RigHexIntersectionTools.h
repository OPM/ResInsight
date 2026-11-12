/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cvfPlane.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <array>
#include <list>
#include <span>
#include <vector>

//==================================================================================================
///  Internal class for intersection point info
//==================================================================================================

struct HexIntersectionInfo
{
public:
    HexIntersectionInfo( cvf::Vec3d intersectionPoint, bool isIntersectionEntering, cvf::StructGridInterface::FaceType face, size_t hexIndex )
        : m_intersectionPoint( intersectionPoint )
        , m_isIntersectionEntering( isIntersectionEntering )
        , m_face( face )
        , m_hexIndex( hexIndex )
    {
    }

    cvf::Vec3d                         m_intersectionPoint;
    bool                               m_isIntersectionEntering;
    cvf::StructGridInterface::FaceType m_face;
    size_t                             m_hexIndex;
};

// Exact lexicographic ordering (strict weak ordering, no tolerance)
bool operator<( const HexIntersectionInfo& hi1, const HexIntersectionInfo& hi2 );

//--------------------------------------------------------------------------------------------------
/// Specialized Line - Hex intersection
//--------------------------------------------------------------------------------------------------
struct RigHexIntersectionTools
{
    // Relative merge tolerance for the segment parameter t in [0, 1], i.e. scaled by segment length
    static constexpr double intersectionMergeTolerance = 1.0e-8;

    // Crossing of a segment with a cell surface triangle, at parameter t along the segment
    struct SegmentHit
    {
        double                             t;
        cvf::Vec3d                         point;
        bool                               isEntering;
        cvf::StructGridInterface::FaceType face;
    };

    // Sorts the hits along the segment and appends them to intersections. A run of consecutive hits in the same
    // direction closer than mergeTolerance (in t) to the first hit of the run is reported once. A hit in the opposite
    // direction ends the run, so distinct crossings are never merged across an intervening crossing.
    static int sortMergeAndAppendHits( std::span<SegmentHit>             hits,
                                       double                            mergeTolerance,
                                       size_t                            hexIndex,
                                       std::vector<HexIntersectionInfo>* intersections );

    // Center of a face with corners in cyclic order. The result is bit-identical for any rotation or reversal of the
    // corner order, so the shared face of two neighbour cells gets the same center
    static cvf::Vec3d canonicalFaceCenter( const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3 );

    // The canonical triangulation of the cell surface, used by all line and point vs cell queries: each face is split
    // into four triangles (corner i, corner i+1, face center), in the corner order given by
    // cvf::StructGridInterface::cellFaceVertexIndices. The visitor is called as visitor(face, corner, nextCorner,
    // faceCenter) and returns false to stop the iteration. Returns false if the iteration was stopped.
    template <typename Visitor>
    static bool forEachCellSurfaceTriangle( const std::array<cvf::Vec3d, 8>& hexCorners, Visitor&& visitor );

    // Intersections are deduplicated deterministically and returned sorted by the parameter along p1 -> p2
    static int lineHexCellIntersection( const cvf::Vec3d&                 p1,
                                        const cvf::Vec3d&                 p2,
                                        const std::array<cvf::Vec3d, 8>&  hexCorners,
                                        const size_t                      hexIndex,
                                        std::vector<HexIntersectionInfo>* intersections,
                                        double                            parametricTolerance = 0.0 );

    static bool lineIntersectsHexCell( const cvf::Vec3d& p1, const cvf::Vec3d& p2, const std::array<cvf::Vec3d, 8>& hexCorners );

    static bool isPointInCell( const cvf::Vec3d& point, const std::array<cvf::Vec3d, 8>& hexCorners );

    static bool planeHexCellIntersection( const std::array<cvf::Vec3d, 8>&              hexCorners,
                                          const cvf::Plane&                             fracturePlane,
                                          std::list<std::pair<cvf::Vec3d, cvf::Vec3d>>& intersectionLineSegments );

    static bool planeHexIntersectionPolygons( const std::array<cvf::Vec3d, 8>&      hexCorners,
                                              cvf::Mat4d                            transformMatrixForPlane,
                                              std::vector<std::vector<cvf::Vec3d>>& polygons );
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename Visitor>
bool RigHexIntersectionTools::forEachCellSurfaceTriangle( const std::array<cvf::Vec3d, 8>& hexCorners, Visitor&& visitor )
{
    for ( int faceIdx = 0; faceIdx < 6; ++faceIdx )
    {
        const auto face = static_cast<cvf::StructGridInterface::FaceType>( faceIdx );

        cvf::ubyte faceVertexIndices[4];
        cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );

        const cvf::Vec3d faceCenter = canonicalFaceCenter( hexCorners[faceVertexIndices[0]],
                                                           hexCorners[faceVertexIndices[1]],
                                                           hexCorners[faceVertexIndices[2]],
                                                           hexCorners[faceVertexIndices[3]] );

        for ( int i = 0; i < 4; ++i )
        {
            const int next = i < 3 ? i + 1 : 0;
            if ( !visitor( face, hexCorners[faceVertexIndices[i]], hexCorners[faceVertexIndices[next]], faceCenter ) ) return false;
        }
    }
    return true;
}
