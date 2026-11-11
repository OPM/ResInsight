/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "gtest/gtest.h"

#include "cvfStructGrid.h"

#include <algorithm>
#include <array>
#include <set>

using namespace cvf;

namespace
{
//--------------------------------------------------------------------------------------------------
/// Unit cube corners in local (i, j, k) coordinates using the ResInsight corner ordering
//--------------------------------------------------------------------------------------------------
std::array<Vec3d, 8> unitCubeCorners()
{
    return { Vec3d( 0, 0, 0 ),
             Vec3d( 1, 0, 0 ),
             Vec3d( 1, 1, 0 ),
             Vec3d( 0, 1, 0 ),
             Vec3d( 0, 0, 1 ),
             Vec3d( 1, 0, 1 ),
             Vec3d( 1, 1, 1 ),
             Vec3d( 0, 1, 1 ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isPositiveFace( StructGridInterface::FaceType face )
{
    return face == StructGridInterface::POS_I || face == StructGridInterface::POS_J || face == StructGridInterface::POS_K;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int axisIndex( StructGridInterface::FaceType face )
{
    return static_cast<int>( StructGridInterface::gridAxisFromFace( face ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Vec3d outwardNormal( StructGridInterface::FaceType face )
{
    Vec3d normal              = Vec3d::ZERO;
    normal[axisIndex( face )] = isPositiveFace( face ) ? 1.0 : -1.0;
    return normal;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, TestEdgeVertices )
{
    {
        // Baseline

        StructGridInterface::FaceType face1 = StructGridInterface::NEG_K;
        StructGridInterface::FaceType face2 = StructGridInterface::POS_I;

        auto indices = StructGridInterface::edgeVertexIndices( face1, face2 );
        EXPECT_EQ( indices.first, 2 );
        EXPECT_EQ( indices.second, 1 );
    }

    {
        // Opposite order as baseline test

        StructGridInterface::FaceType face1 = StructGridInterface::POS_I;
        StructGridInterface::FaceType face2 = StructGridInterface::NEG_K;

        auto indices = StructGridInterface::edgeVertexIndices( face1, face2 );
        EXPECT_EQ( indices.first, 2 );
        EXPECT_EQ( indices.second, 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, OppositeFaceVertexIndexMapConnectsCoincidentPillarVertices )
{
    const auto corners = unitCubeCorners();

    const auto& map = StructGridInterface::oppositeFaceVertexIndexMap();

    for ( auto face : StructGridInterface::validFaceTypes() )
    {
        const auto axis = axisIndex( face );

        const auto& faceIndices     = StructGridInterface::cellFaceVertexIndices( face );
        const auto& oppositeIndices = StructGridInterface::cellFaceVertexIndices( StructGridInterface::oppositeFace( face ) );

        for ( size_t i = 0; i < 4; i++ )
        {
            Vec3d diff = corners[faceIndices[i]] - corners[oppositeIndices[map[i]]];
            diff[axis] = 0.0;
            EXPECT_DOUBLE_EQ( 0.0, diff.length() ) << "face " << face << " local vertex " << i;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, LegacyCellFaceVertexIndicesMatchesTable )
{
    for ( auto face : StructGridInterface::validFaceTypes() )
    {
        ubyte legacy[4];
        StructGridInterface::cellFaceVertexIndices( face, legacy );

        const auto& indices = StructGridInterface::cellFaceVertexIndices( face );
        for ( size_t i = 0; i < 4; i++ )
        {
            EXPECT_EQ( indices[i], legacy[i] );
        }
    }

    ubyte untouched[4] = { 9, 9, 9, 9 };
    StructGridInterface::cellFaceVertexIndices( StructGridInterface::NO_FACE, untouched );
    EXPECT_EQ( 9, untouched[0] );
}

//--------------------------------------------------------------------------------------------------
/// The exact vertex order, including the start vertex, is relied upon by result files, geometry generators
/// and the opposite face vertex map, so it must never change
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, CellFaceVertexIndicesExactOrder )
{
    using FT                                                        = StructGridInterface::FaceType;
    const std::vector<std::pair<FT, std::array<ubyte, 4>>> expected = {
        { StructGridInterface::POS_I, { 1, 2, 6, 5 } },
        { StructGridInterface::NEG_I, { 0, 4, 7, 3 } },
        { StructGridInterface::POS_J, { 3, 7, 6, 2 } },
        { StructGridInterface::NEG_J, { 0, 1, 5, 4 } },
        { StructGridInterface::POS_K, { 4, 5, 6, 7 } },
        { StructGridInterface::NEG_K, { 0, 3, 2, 1 } },
    };

    for ( const auto& [face, indices] : expected )
    {
        EXPECT_EQ( indices, StructGridInterface::cellFaceVertexIndices( face ) ) << "face " << face;
    }

    EXPECT_EQ( ( std::array<ubyte, 4>{ 0, 3, 2, 1 } ), StructGridInterface::oppositeFaceVertexIndexMap() );
    EXPECT_EQ( ( std::array<ubyte, 8>{ 0, 1, 3, 2, 4, 5, 7, 6 } ), StructGridInterface::reservoirSimulatorCornerIndexMapping() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, CellFaceVertexIndicesLieOnFacePlane )
{
    const auto corners = unitCubeCorners();

    for ( auto face : StructGridInterface::validFaceTypes() )
    {
        const auto& indices = StructGridInterface::cellFaceVertexIndices( face );

        std::set<ubyte> unique( indices.begin(), indices.end() );
        EXPECT_EQ( 4u, unique.size() ) << "face " << face;

        const double expectedCoord = isPositiveFace( face ) ? 1.0 : 0.0;
        for ( auto idx : indices )
        {
            ASSERT_LT( idx, 8 );
            EXPECT_DOUBLE_EQ( expectedCoord, corners[idx][axisIndex( face )] ) << "face " << face << " vertex " << int( idx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, CellFaceVertexIndicesFormQuadWithOutwardNormal )
{
    const auto corners = unitCubeCorners();

    for ( auto face : StructGridInterface::validFaceTypes() )
    {
        const auto& indices = StructGridInterface::cellFaceVertexIndices( face );

        // Consecutive vertices must be connected by a cell edge, i.e. differ in exactly one coordinate
        for ( size_t i = 0; i < 4; i++ )
        {
            const Vec3d edge = corners[indices[( i + 1 ) % 4]] - corners[indices[i]];
            EXPECT_DOUBLE_EQ( 1.0, edge.length() ) << "face " << face << " edge " << i;
        }

        Vec3d normal = Vec3d::ZERO;
        for ( size_t i = 0; i < 4; i++ )
        {
            normal += corners[indices[i]] ^ corners[indices[( i + 1 ) % 4]];
        }
        normal.normalize();

        EXPECT_DOUBLE_EQ( 1.0, normal * outwardNormal( face ) ) << "face " << face;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, EachCornerIsSharedByThreeFaces )
{
    std::array<int, 8> faceCountPerCorner{};
    for ( auto face : StructGridInterface::validFaceTypes() )
    {
        for ( auto idx : StructGridInterface::cellFaceVertexIndices( face ) )
        {
            faceCountPerCorner[idx]++;
        }
    }

    for ( auto count : faceCountPerCorner )
    {
        EXPECT_EQ( 3, count );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, OppositeFaceIsOtherFaceOnSameAxis )
{
    for ( auto face : StructGridInterface::validFaceTypes() )
    {
        const auto opposite = StructGridInterface::oppositeFace( face );

        EXPECT_NE( face, opposite );
        EXPECT_EQ( face, StructGridInterface::oppositeFace( opposite ) );
        EXPECT_EQ( StructGridInterface::gridAxisFromFace( face ), StructGridInterface::gridAxisFromFace( opposite ) );
        EXPECT_NE( isPositiveFace( face ), isPositiveFace( opposite ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, OppositeFaceVertexIndexMapIsInvolution )
{
    const auto& map = StructGridInterface::oppositeFaceVertexIndexMap();

    std::set<ubyte> unique( map.begin(), map.end() );
    EXPECT_EQ( 4u, unique.size() );

    for ( size_t i = 0; i < 4; i++ )
    {
        ASSERT_LT( map[i], 4 );
        EXPECT_EQ( i, map[map[i]] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, EdgeVertexIndicesAreSharedByBothFacesInFaceOrder )
{
    for ( auto face1 : StructGridInterface::validFaceTypes() )
    {
        for ( auto face2 : StructGridInterface::validFaceTypes() )
        {
            auto [v0, v1] = StructGridInterface::edgeVertexIndices( face1, face2 );

            if ( StructGridInterface::gridAxisFromFace( face1 ) == StructGridInterface::gridAxisFromFace( face2 ) )
            {
                EXPECT_EQ( 0, v0 );
                EXPECT_EQ( 0, v1 );
                continue;
            }

            const auto& indices1 = StructGridInterface::cellFaceVertexIndices( face1 );
            const auto& indices2 = StructGridInterface::cellFaceVertexIndices( face2 );

            auto contains = []( const std::array<ubyte, 4>& a, ubyte v ) { return std::find( a.begin(), a.end(), v ) != a.end(); };
            EXPECT_NE( v0, v1 );
            EXPECT_TRUE( contains( indices1, v0 ) && contains( indices1, v1 ) ) << face1 << " " << face2;
            EXPECT_TRUE( contains( indices2, v0 ) && contains( indices2, v1 ) ) << face1 << " " << face2;

            // The edge follows the vertex order of the face with the largest enum value
            const auto& ordered = StructGridInterface::cellFaceVertexIndices( std::max( face1, face2 ) );
            auto        pos0    = std::find( ordered.begin(), ordered.end(), v0 ) - ordered.begin();
            EXPECT_EQ( v1, ordered[( pos0 + 1 ) % 4] ) << face1 << " " << face2;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, ReservoirSimulatorCornerIndexMappingIsInvolution )
{
    const auto& map = StructGridInterface::reservoirSimulatorCornerIndexMapping();

    std::set<ubyte> unique( map.begin(), map.end() );
    EXPECT_EQ( 8u, unique.size() );

    for ( size_t i = 0; i < 8; i++ )
    {
        ASSERT_LT( map[i], 8 );
        EXPECT_EQ( i, map[map[i]] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( StructGridInterfaceTest, ReservoirSimulatorCornerIndexMappingMatchesCornerPositions )
{
    // Reservoir simulator corners are ordered with i running fastest, then j, then k
    const auto  corners = unitCubeCorners();
    const auto& map     = StructGridInterface::reservoirSimulatorCornerIndexMapping();

    for ( size_t simIdx = 0; simIdx < 8; simIdx++ )
    {
        const Vec3d simulatorCorner( simIdx & 1, ( simIdx >> 1 ) & 1, ( simIdx >> 2 ) & 1 );
        EXPECT_EQ( simulatorCorner, corners[map[simIdx]] ) << "simulator corner " << simIdx;
    }
}
