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

#include "RivEnclosingPolygonGenerator.h"
#include "cvfMath.h"

#include <algorithm>
#include <map>

RivEnclosingPolygonGenerator::RivEnclosingPolygonGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivEnclosingPolygonGenerator::constructEnclosingPolygon()
{
    // Construct the enclosing polygon from the edges
    //
    // d ________ c
    //   |     /|
    //   |    / |
    //   |   /  |
    //   |  /   |
    //   | /    |
    //   |/_____|
    //  a        b
    //
    //  The line segment ca/ac is the only edge that is not part of the enclosing polygon
    //  This line segment will occur twice in the list of edges as it is present in both triangles
    //  (a, b, c) and (a, c, d).
    //  The line segment ca/ac will be removed from the list of for the enclosing polygon
    //
    // Enclosing edges are defined as edges only occurring once in the list of edges
    //

    // Must have at least 3 edges to construct a polygon
    CVF_ASSERT( isValidPolygon() );

    // Map of edge key and number of occurrences
    std::map<cvf::uint64, cvf::uint> edgeKeysAndCount;

    // Extract boundary edge keys from all edge keys
    for ( const auto& edgeKey : m_allEdgeKeys )
    {
        // If edge is already in the set, it occurs more than once and is not a boundary edge
        // Assuming no degenerate triangles, i.e. triangle with two or more vertices at the same position
        if ( edgeKeysAndCount.contains( edgeKey ) )
        {
            edgeKeysAndCount[edgeKey]++;
        }
        else
        {
            edgeKeysAndCount[edgeKey] = 1;
        }
    }

    // At least a triangle is needed to construct a polygon
    CVF_ASSERT( edgeKeysAndCount.size() >= 3 );

    // Extract boundary edge keys from all edge keys and count
    std::set<cvf::EdgeKey> boundaryEdges;
    for ( const auto& [key, count] : edgeKeysAndCount )
    {
        if ( count == 1 )
        {
            boundaryEdges.insert( cvf::EdgeKey::fromkeyVal( key ) );
        }
    }

    // Lambda function to check if index exists in a vector
    auto indexExists = []( const std::vector<cvf::uint>& indices, cvf::uint index ) -> bool
    { return std::find( indices.cbegin(), indices.cend(), index ) != indices.cend(); };

    // Construct the enclosing polygon from the boundary edges
    cvf::EdgeKey           currentEdge                   = *boundaryEdges.begin();
    std::vector<cvf::uint> enclosingPolygonVertexIndices = { currentEdge.index1(), currentEdge.index2() };
    cvf::uint              nextVertexIndex               = currentEdge.index2();
    boundaryEdges.erase( currentEdge );
    while ( !boundaryEdges.empty() )
    {
        // Find next edge in the boundary, i.e. edge containing the next vertex index to look for
        currentEdge = findNextEdge( nextVertexIndex, boundaryEdges );
        boundaryEdges.erase( currentEdge );
        const auto start = currentEdge.index1();
        const auto end   = currentEdge.index2();
        if ( start == cvf::UNDEFINED_UINT || end == cvf::UNDEFINED_UINT )
        {
            // Throw error?
            break;
        }

        // The enclosing polygon is a closed loop, so the start and end vertices are always in the correct order
        if ( !indexExists( enclosingPolygonVertexIndices, end ) )
        {
            nextVertexIndex = end;
            enclosingPolygonVertexIndices.push_back( end );
        }
        else if ( !indexExists( enclosingPolygonVertexIndices, start ) )
        {
            nextVertexIndex = start;
            enclosingPolygonVertexIndices.push_back( start );
        }
    }

    m_polygonVertexIndices = enclosingPolygonVertexIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::EdgeKey RivEnclosingPolygonGenerator::findNextEdge( cvf::uint vertexIndex, const std::set<cvf::EdgeKey>& boundaryEdges )
{
    for ( auto& elm : boundaryEdges )
    {
        if ( elm.index1() == vertexIndex || elm.index2() == vertexIndex )
        {
            return elm;
        }
    }

    // Return a dummy edge to indicate no next edge found
    return cvf::EdgeKey( cvf::UNDEFINED_UINT, cvf::UNDEFINED_UINT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::uint> RivEnclosingPolygonGenerator::getPolygonVertexIndices() const
{
    return m_polygonVertexIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivEnclosingPolygonGenerator::isValidPolygon() const
{
    return m_allEdgeKeys.size() >= size_t( 3 );
}

//--------------------------------------------------------------------------------------------------
/// Add triangle vertices to the polygon. The vertex indices are welded and controlled outside
/// of this class.
///
/// Assumes the vertices are given in counter-clockwise order
//--------------------------------------------------------------------------------------------------
void RivEnclosingPolygonGenerator::addTriangleVertexIndices( cvf::uint idxVx0, cvf::uint idxVx1, cvf::uint idxVx2 )
{
    // Verify three unique vertices - i.e. no degenerate triangle
    if ( idxVx0 == idxVx1 || idxVx0 == idxVx2 || idxVx1 == idxVx2 )
    {
        return;
    }

    // Add edges keys to list of all edges
    m_allEdgeKeys.emplace_back( cvf::EdgeKey( idxVx0, idxVx1 ).toKeyVal() );
    m_allEdgeKeys.emplace_back( cvf::EdgeKey( idxVx1, idxVx2 ).toKeyVal() );
    m_allEdgeKeys.emplace_back( cvf::EdgeKey( idxVx2, idxVx0 ).toKeyVal() );
}
