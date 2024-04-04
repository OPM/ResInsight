/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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
#include "cvfEdgeKey.h"

#include <set>
#include <vector>

/*
 * Class for generating an enclosing polygon from a set of vertices.
 *
 * The class add triangle vertex indices and construct constructing the enclosing polygon from the indices.
 * The vertices must be provided in counter clock-wise order.
 *
 * The vertex welding, and mapping of vertex indices to vertex 3D points, must be handled externally.
 */
class RivEnclosingPolygonGenerator
{
public:
    RivEnclosingPolygonGenerator();

    std::vector<cvf::uint> getPolygonVertexIndices() const;

    bool isValidPolygon() const;

    void addTriangleVertexIndices( cvf::uint idxVx0, cvf::uint idxVx1, cvf::uint idxVx2 );
    void constructEnclosingPolygon();

private:
    static cvf::EdgeKey findNextEdge( cvf::uint vertextIndex, const std::set<cvf::EdgeKey>& boundaryEdges );

private:
    std::vector<cvf::int64> m_allEdgeKeys; // Create edge defined by vertex indices when adding triangle. Using cvf::EdgeKey::toKeyVal()
    std::vector<cvf::uint>  m_polygonVertexIndices; // List polygon vertex indices counter clock-wise
};
