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

#include "cvfStructGrid.h"
#include "cvfBase.h"
#include "cvfBoundingBox.h"

#include <algorithm>
#include <array>

namespace caf
{
template <>
void cvf::StructGridInterface::FaceEnum::setUp()
{
    addItem( cvf::StructGridInterface::POS_I, "POS_I", "POS I" );
    addItem( cvf::StructGridInterface::NEG_I, "NEG_I", "NEG I" );
    addItem( cvf::StructGridInterface::POS_J, "POS_J", "POS J" );
    addItem( cvf::StructGridInterface::NEG_J, "NEG_J", "NEG J" );
    addItem( cvf::StructGridInterface::POS_K, "POS_K", "POS K" );
    addItem( cvf::StructGridInterface::NEG_K, "NEG_K", "NEG K" );
    addItem( cvf::StructGridInterface::NO_FACE, "UNDEF", "UnDef" );
}
} // namespace caf

namespace cvf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StructGridInterface::StructGridInterface()
{
    m_characteristicCellSize = cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StructGridInterface::cellFaceVertexIndices( FaceType face, cvf::ubyte vertexIndices[4] )
{
    if ( face < POS_I || face > NEG_K ) return;

    const auto& indices = cellFaceVertexIndices( face );
    std::copy( indices.begin(), indices.end(), vertexIndices );
}

//--------------------------------------------------------------------------------------------------
/// Corner indices of a cell face
///
///     7---------6
///    /|        /|     |k
///   / |       / |     | /j
///  4---------5  |     |/
///  |  3------|--2     *---i
///  | /       | /
///  |/        |/
///  0---------1
//--------------------------------------------------------------------------------------------------
const std::array<ubyte, 4>& StructGridInterface::cellFaceVertexIndices( FaceType face )
{
    // Indexed by FaceType: POS_I, NEG_I, POS_J, NEG_J, POS_K, NEG_K
    static constexpr std::array<std::array<ubyte, 4>, 6> faceVertexIndices = { {
        { 1, 2, 6, 5 },
        { 0, 4, 7, 3 },
        { 3, 7, 6, 2 },
        { 0, 1, 5, 4 },
        { 4, 5, 6, 7 },
        { 0, 3, 2, 1 },
    } };

    CVF_ASSERT( face >= POS_I && face <= NEG_K );
    return faceVertexIndices[face];
}

//--------------------------------------------------------------------------------------------------
/// Maps a face-local vertex index on a face to the face-local vertex index of the coincident vertex on the
/// opposite face, either of the same cell (connected by an edge) or of the neighbor cell across the face.
/// See cellFaceVertexIndices(). The mapping is its own inverse.
//--------------------------------------------------------------------------------------------------
const std::array<ubyte, 4>& StructGridInterface::oppositeFaceVertexIndexMap()
{
    static constexpr std::array<ubyte, 4> map = { 0, 3, 2, 1 };
    return map;
}

//--------------------------------------------------------------------------------------------------
/// Maps between reservoir simulator corner ordering and ResInsight corner ordering.
/// The mapping is its own inverse.
///
/// Reservoir simulator corner ordering (i fastest, then j, then k, with k pointing downwards)
///
///      2-------------3
///     /|            /|
///    / |           / |               /j
///   /  |          /  |              /
///  0-------------1   |             *---i
///  |   |         |   |             |
///  |   6---------|---7             |
///  |  /          |  /              |k
///  | /           | /
///  |/            |/
///  4-------------5
///
/// ResInsight corner ordering
///
///      7-------------6             |k
///     /|            /|             | /j
///    / |           / |             |/
///   /  |          /  |             *---i
///  4-------------5   |
///  |   |         |   |
///  |   3---------|---2
///  |  /          |  /
///  | /           | /
///  |/            |/
///  0-------------1
//--------------------------------------------------------------------------------------------------
const std::array<ubyte, 8>& StructGridInterface::reservoirSimulatorCornerIndexMapping()
{
    static constexpr std::array<ubyte, 8> map = { 0, 1, 3, 2, 4, 5, 7, 6 };
    return map;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<ubyte, ubyte> StructGridInterface::edgeVertexIndices( FaceType face1, FaceType face2 )
{
    // Return the two shared vertex indices between two faces
    // The ordering is identical to the ordering in StructGridInterface::cellFaceVertexIndices

    // Ensure face1 has the largest enum value
    if ( face2 > face1 ) std::swap( face1, face2 );

    if ( face1 == NEG_K )
    {
        if ( face2 == NEG_I ) return { 0, 3 };
        if ( face2 == POS_I ) return { 2, 1 };
        if ( face2 == NEG_J ) return { 1, 0 };
        if ( face2 == POS_J ) return { 3, 2 };
    }

    if ( face1 == POS_K )
    {
        if ( face2 == NEG_I ) return { 7, 4 };
        if ( face2 == POS_I ) return { 5, 6 };
        if ( face2 == NEG_J ) return { 4, 5 };
        if ( face2 == POS_J ) return { 6, 7 };
    }

    if ( face1 == NEG_J )
    {
        if ( face2 == NEG_I ) return { 4, 0 };
        if ( face2 == POS_I ) return { 1, 5 };
    }

    if ( face1 == POS_J )
    {
        if ( face2 == NEG_I ) return { 3, 7 };
        if ( face2 == POS_I ) return { 6, 2 };
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StructGridInterface::FaceType StructGridInterface::oppositeFace( FaceType face )
{
    FaceType opposite;

    switch ( face )
    {
        case NEG_I:
            opposite = POS_I;
            break;
        case POS_I:
            opposite = NEG_I;
            break;
        case NEG_J:
            opposite = POS_J;
            break;
        case POS_J:
            opposite = NEG_J;
            break;
        case NEG_K:
            opposite = POS_K;
            break;
        case POS_K:
            opposite = NEG_K;
            break;
        default:
            opposite = POS_I;
            CVF_ASSERT( false );
    }

    return opposite;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<StructGridInterface::FaceType> StructGridInterface::validFaceTypes()
{
    return {
        cvf::StructGridInterface::FaceType::NEG_I,
        cvf::StructGridInterface::FaceType::POS_I,
        cvf::StructGridInterface::FaceType::NEG_J,
        cvf::StructGridInterface::FaceType::POS_J,
        cvf::StructGridInterface::FaceType::NEG_K,
        cvf::StructGridInterface::FaceType::POS_K,
    };
}

//--------------------------------------------------------------------------------------------------
/// Return values are set to cvf::UNDEFINED_SIZE_T if the neighbor is in the negative area
//--------------------------------------------------------------------------------------------------
void StructGridInterface::neighborIJKAtCellFace( size_t i, size_t j, size_t k, FaceType face, size_t* ni, size_t* nj, size_t* nk )
{
    *ni = i;
    *nj = j;
    *nk = k;

    switch ( face )
    {
        case POS_I:
            ( *ni )++;
            break;
        case NEG_I:
            if ( i > 0 )
                ( *ni )--;
            else
                ( *ni ) = cvf::UNDEFINED_SIZE_T;
            break;
        case POS_J:
            ( *nj )++;
            break;
        case NEG_J:
            if ( j > 0 )
                ( *nj )--;
            else
                ( *nj ) = cvf::UNDEFINED_SIZE_T;
            break;
        case POS_K:
            ( *nk )++;
            break;
        case NEG_K:
            if ( k > 0 )
                ( *nk )--;
            else
                ( *nk ) = cvf::UNDEFINED_SIZE_T;
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StructGridInterface::GridAxisType StructGridInterface::gridAxisFromFace( FaceType face )
{
    GridAxisType axis = GridAxisType::NO_AXIS;

    if ( face == cvf::StructGridInterface::POS_I || face == cvf::StructGridInterface::NEG_I )
    {
        axis = GridAxisType::AXIS_I;
    }
    else if ( face == cvf::StructGridInterface::POS_J || face == cvf::StructGridInterface::NEG_J )
    {
        axis = GridAxisType::AXIS_J;
    }
    else if ( face == cvf::StructGridInterface::POS_K || face == cvf::StructGridInterface::NEG_K )
    {
        axis = GridAxisType::AXIS_K;
    }

    return axis;
}

//--------------------------------------------------------------------------------------------------
/// Models with large absolute values for coordinate scalars will often end up with z-fighting due
/// to numerical limits in float used by OpenGL. displayModelOffset() is intended
//  to be subtracted from a domain model coordinate when building geometry
//
//  Used in StructGridGeometryGenerator::computeArrays()
//
//  Vec3d domainModelCoord = ...
//  Vec3d vizCoord = domainModelCoord - displayModelOffset();
//--------------------------------------------------------------------------------------------------
cvf::Vec3d StructGridInterface::displayModelOffset() const
{
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d StructGridInterface::characteristicCellSizes() const
{
    if ( !hasValidCharacteristicCellSizes() )
    {
        std::vector<size_t> reservoirCellIndices;
        reservoirCellIndices.resize( cellCountI() * cellCountJ() * cellCountK() );
        std::iota( reservoirCellIndices.begin(), reservoirCellIndices.end(), 0 );

        computeCharacteristicCellSize( reservoirCellIndices );
    }

    return m_characteristicCellSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool StructGridInterface::hasValidCharacteristicCellSizes() const
{
    return !m_characteristicCellSize.isUndefined();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StructGridInterface::computeCharacteristicCellSize( const std::vector<size_t>& globalCellIndices ) const
{
    if ( globalCellIndices.empty() ) return;

    ubyte faceConnPosI[4];
    cellFaceVertexIndices( StructGridInterface::POS_I, faceConnPosI );

    ubyte faceConnNegI[4];
    cellFaceVertexIndices( StructGridInterface::NEG_I, faceConnNegI );

    ubyte faceConnPosJ[4];
    cellFaceVertexIndices( StructGridInterface::POS_J, faceConnPosJ );

    ubyte faceConnNegJ[4];
    cellFaceVertexIndices( StructGridInterface::NEG_J, faceConnNegJ );

    ubyte faceConnPosK[4];
    cellFaceVertexIndices( StructGridInterface::POS_K, faceConnPosK );

    ubyte faceConnNegK[4];
    cellFaceVertexIndices( StructGridInterface::NEG_K, faceConnNegK );

    double    tolerance         = 0.2;
    int       iterationIndex    = 0;
    const int iterationMaxCount = 3;
    while ( iterationIndex < iterationMaxCount )
    {
        if ( iterationIndex > 0 )
        {
            // Divide tolerance by a factor for each iteration
            tolerance = tolerance / 10.0;
        }

        double iLengthAccumulated = 0.0;
        double jLengthAccumulated = 0.0;
        double kLengthAccumulated = 0.0;

        size_t evaluatedCellCount = 0;

        // Evaluate N-th cells, compute the stride between each index
        size_t stride = std::max( size_t( 1 ), globalCellIndices.size() / 100 );

        size_t i, j, k = 0;
        size_t index = 0;
        while ( index < globalCellIndices.size() )
        {
            size_t cellIndex = globalCellIndices[index];
            ijkFromCellIndex( cellIndex, &i, &j, &k );
            if ( isCellValid( i, j, k ) )
            {
                std::array<cvf::Vec3d, 8> cornerVerts = cellCornerVertices( cellIndex );

                cvf::BoundingBox bb;
                for ( const auto& v : cornerVerts )
                {
                    bb.add( v );
                }

                // Exclude cells with very small volumes
                if ( bb.extent().z() > tolerance )
                {
                    iLengthAccumulated += ( cornerVerts[faceConnPosI[0]] - cornerVerts[faceConnNegI[0]] ).lengthSquared();
                    iLengthAccumulated += ( cornerVerts[faceConnPosI[1]] - cornerVerts[faceConnNegI[3]] ).lengthSquared();
                    iLengthAccumulated += ( cornerVerts[faceConnPosI[2]] - cornerVerts[faceConnNegI[2]] ).lengthSquared();
                    iLengthAccumulated += ( cornerVerts[faceConnPosI[3]] - cornerVerts[faceConnNegI[1]] ).lengthSquared();

                    jLengthAccumulated += ( cornerVerts[faceConnPosJ[0]] - cornerVerts[faceConnNegJ[0]] ).lengthSquared();
                    jLengthAccumulated += ( cornerVerts[faceConnPosJ[1]] - cornerVerts[faceConnNegJ[3]] ).lengthSquared();
                    jLengthAccumulated += ( cornerVerts[faceConnPosJ[2]] - cornerVerts[faceConnNegJ[2]] ).lengthSquared();
                    jLengthAccumulated += ( cornerVerts[faceConnPosJ[3]] - cornerVerts[faceConnNegJ[1]] ).lengthSquared();

                    kLengthAccumulated += ( cornerVerts[faceConnPosK[0]] - cornerVerts[faceConnNegK[0]] ).lengthSquared();
                    kLengthAccumulated += ( cornerVerts[faceConnPosK[1]] - cornerVerts[faceConnNegK[3]] ).lengthSquared();
                    kLengthAccumulated += ( cornerVerts[faceConnPosK[2]] - cornerVerts[faceConnNegK[2]] ).lengthSquared();
                    kLengthAccumulated += ( cornerVerts[faceConnPosK[3]] - cornerVerts[faceConnNegK[1]] ).lengthSquared();

                    evaluatedCellCount++;
                }
            }

            index += stride;
        }

        iterationIndex++;

        if ( evaluatedCellCount > 10 || iterationIndex == iterationMaxCount )
        {
            double divisor = evaluatedCellCount * 4.0;

            if ( divisor > 0.0 )
            {
                m_characteristicCellSize = cvf::Vec3d( cvf::Math::sqrt( iLengthAccumulated / divisor ),
                                                       cvf::Math::sqrt( jLengthAccumulated / divisor ),
                                                       cvf::Math::sqrt( kLengthAccumulated / divisor ) );

                return;
            }
        }
    }
}

} // namespace cvf
