/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cafLine.h"
#include "cvfArray.h"
#include "cvfEdgeKey.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <algorithm>
#include <vector>

/*
 * Class for handling welding of vertices in a polygon to prevent duplicated vertex 3D points within a tolerance margin
 */
class PolygonVertexWelder
{
public:
    PolygonVertexWelder( double weldEpsilon );

    void reserveVertices( cvf::uint vertexCount );

    cvf::uint weldVertexAndGetIndex( const cvf::Vec3d& vertex );

    const cvf::Vec3d&         vertex( cvf::uint index ) const;
    cvf::ref<cvf::Vec3dArray> createVertexArray() const;

private:
    cvf::uint locateVertexInPolygon( const cvf::Vec3d& vertex ) const;
    cvf::uint addVertexToPolygon( const cvf::Vec3d& vertex );

private:
    const double m_epsilonSquared; // Tolerance for vertex welding, radius around vertex defining welding neighborhood

    cvf::uint               m_first; // Start of linked list
    std::vector<cvf::uint>  m_next; // Links each vertex to next in linked list. Always numVertices long, will grow as vertices are added
    std::vector<cvf::Vec3d> m_vertex; // Unique vertices within tolerance
};

/*
 * Class for generating an enclosing polygon from a set of vertices.
 *
 * The class will weld triangle vertices close to each other and provide a vertex index for
 * the resulting set of vertices. These indices are used for algorithms constructing the enclosing polygon.
 *
 * The welding is done using a tolerance value to handle floating point errors.
 */
class RivEnclosingPolygonGenerator
{
public:
    RivEnclosingPolygonGenerator();

    std::vector<cvf::Vec3d> getPolygonVertices() const;

    bool isValidPolygon() const;

    void addTriangleVertices( const cvf::Vec3d& p0, const cvf::Vec3d& p1, const cvf::Vec3d& p2 );
    void constructEnclosingPolygon();

private:
    static cvf::EdgeKey findNextEdge( cvf::uint vertextIndex, const std::set<cvf::EdgeKey>& boundaryEdges );

private:
    PolygonVertexWelder     m_polygonVertexWelder; // Add and weld vertices for a polygon, provides vertex index
    std::vector<cvf::int64> m_allEdgeKeys; // Create edge defined by vertex indices when adding triangle. Using cvf::EdgeKey::toKeyVal()
    std::vector<cvf::Vec3d> m_polygonVertices; // List polygon vertices counter clock-wise
};
