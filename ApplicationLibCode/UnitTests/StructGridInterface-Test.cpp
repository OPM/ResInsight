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

using namespace cvf;

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
    // Unit cube corners using the ResInsight corner ordering
    const std::array<Vec3d, 8> corners = { Vec3d( 0, 0, 0 ),
                                           Vec3d( 1, 0, 0 ),
                                           Vec3d( 1, 1, 0 ),
                                           Vec3d( 0, 1, 0 ),
                                           Vec3d( 0, 0, 1 ),
                                           Vec3d( 1, 0, 1 ),
                                           Vec3d( 1, 1, 1 ),
                                           Vec3d( 0, 1, 1 ) };

    const auto& map = StructGridInterface::oppositeFaceVertexIndexMap();

    for ( auto face : StructGridInterface::validFaceTypes() )
    {
        const auto axis = static_cast<int>( StructGridInterface::gridAxisFromFace( face ) );

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
