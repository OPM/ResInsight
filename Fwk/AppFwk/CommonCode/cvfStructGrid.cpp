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

namespace caf
{
template <>
void cvf::StructGridInterface::FaceEnum::setUp()
{
    addItem( cvf::StructGridInterface::POS_I, "POS I", "" );
    addItem( cvf::StructGridInterface::NEG_I, "NEG I", "" );
    addItem( cvf::StructGridInterface::POS_J, "POS J", "" );
    addItem( cvf::StructGridInterface::NEG_J, "NEG J", "" );
    addItem( cvf::StructGridInterface::POS_K, "POS K", "" );
    addItem( cvf::StructGridInterface::NEG_K, "NEG K", "" );
    addItem( cvf::StructGridInterface::NO_FACE, "UnDef", "" );
}
} // namespace caf

namespace cvf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StructGridInterface::StructGridInterface()
{
    m_characteristicCellSizeI = cvf::UNDEFINED_DOUBLE;
    m_characteristicCellSizeJ = cvf::UNDEFINED_DOUBLE;
    m_characteristicCellSizeK = cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t StructGridInterface::cellCountI() const
{
    if ( gridPointCountI() == 0 ) return 0;

    return gridPointCountI() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t StructGridInterface::cellCountJ() const
{
    if ( gridPointCountJ() == 0 ) return 0;

    return gridPointCountJ() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t StructGridInterface::cellCountK() const
{
    if ( gridPointCountK() == 0 ) return 0;

    return gridPointCountK() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StructGridInterface::cellFaceVertexIndices( FaceType face, cvf::ubyte vertexIndices[4] )
{
    //
    //     7---------6
    //    /|        /|     |k
    //   / |       / |     | /j
    //  4---------5  |     |/
    //  |  3------|--2     *---i
    //  | /       | /
    //  |/        |/
    //  0---------1

    if ( face == NEG_K )
    {
        vertexIndices[0] = 0;
        vertexIndices[1] = 3;
        vertexIndices[2] = 2;
        vertexIndices[3] = 1;
    }
    else if ( face == POS_K )
    {
        vertexIndices[0] = 4;
        vertexIndices[1] = 5;
        vertexIndices[2] = 6;
        vertexIndices[3] = 7;
    }
    else if ( face == NEG_J )
    {
        vertexIndices[0] = 0;
        vertexIndices[1] = 1;
        vertexIndices[2] = 5;
        vertexIndices[3] = 4;
    }
    else if ( face == POS_I )
    {
        vertexIndices[0] = 1;
        vertexIndices[1] = 2;
        vertexIndices[2] = 6;
        vertexIndices[3] = 5;
    }
    else if ( face == POS_J )
    {
        vertexIndices[0] = 3;
        vertexIndices[1] = 7;
        vertexIndices[2] = 6;
        vertexIndices[3] = 2;
    }
    else if ( face == NEG_I )
    {
        vertexIndices[0] = 0;
        vertexIndices[1] = 4;
        vertexIndices[2] = 7;
        vertexIndices[3] = 3;
    }
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
void StructGridInterface::characteristicCellSizes( double* iSize, double* jSize, double* kSize ) const
{
    CVF_ASSERT( iSize && jSize && kSize );

    if ( m_characteristicCellSizeI == cvf::UNDEFINED_DOUBLE || m_characteristicCellSizeJ == cvf::UNDEFINED_DOUBLE ||
         m_characteristicCellSizeK == cvf::UNDEFINED_DOUBLE )
    {
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

        double iLengthAccumulated = 0.0;
        double jLengthAccumulated = 0.0;
        double kLengthAccumulated = 0.0;

        cvf::Vec3d cornerVerts[8];
        size_t     cellCount = 0;

        size_t k;
        for ( k = 0; k < cellCountK(); k++ )
        {
            size_t j;
            for ( j = 0; j < cellCountJ(); j++ )
            {
                size_t i;
                for ( i = 0; i < cellCountI(); i += 10 ) // NB! Evaluate every n-th cell
                {
                    if ( isCellValid( i, j, k ) )
                    {
                        size_t cellIndex = cellIndexFromIJK( i, j, k );
                        cellCornerVertices( cellIndex, cornerVerts );

                        iLengthAccumulated +=
                            ( cornerVerts[faceConnPosI[0]] - cornerVerts[faceConnNegI[0]] ).lengthSquared();
                        iLengthAccumulated +=
                            ( cornerVerts[faceConnPosI[1]] - cornerVerts[faceConnNegI[3]] ).lengthSquared();
                        iLengthAccumulated +=
                            ( cornerVerts[faceConnPosI[2]] - cornerVerts[faceConnNegI[2]] ).lengthSquared();
                        iLengthAccumulated +=
                            ( cornerVerts[faceConnPosI[3]] - cornerVerts[faceConnNegI[1]] ).lengthSquared();

                        jLengthAccumulated +=
                            ( cornerVerts[faceConnPosJ[0]] - cornerVerts[faceConnNegJ[0]] ).lengthSquared();
                        jLengthAccumulated +=
                            ( cornerVerts[faceConnPosJ[1]] - cornerVerts[faceConnNegJ[3]] ).lengthSquared();
                        jLengthAccumulated +=
                            ( cornerVerts[faceConnPosJ[2]] - cornerVerts[faceConnNegJ[2]] ).lengthSquared();
                        jLengthAccumulated +=
                            ( cornerVerts[faceConnPosJ[3]] - cornerVerts[faceConnNegJ[1]] ).lengthSquared();

                        kLengthAccumulated +=
                            ( cornerVerts[faceConnPosK[0]] - cornerVerts[faceConnNegK[0]] ).lengthSquared();
                        kLengthAccumulated +=
                            ( cornerVerts[faceConnPosK[1]] - cornerVerts[faceConnNegK[3]] ).lengthSquared();
                        kLengthAccumulated +=
                            ( cornerVerts[faceConnPosK[2]] - cornerVerts[faceConnNegK[2]] ).lengthSquared();
                        kLengthAccumulated +=
                            ( cornerVerts[faceConnPosK[3]] - cornerVerts[faceConnNegK[1]] ).lengthSquared();

                        cellCount++;
                    }
                }
            }
        }

        double divisor = cellCount * 4.0;

        if ( divisor > 0.0 )
        {
            m_characteristicCellSizeI = cvf::Math::sqrt( iLengthAccumulated / divisor );
            m_characteristicCellSizeJ = cvf::Math::sqrt( jLengthAccumulated / divisor );
            m_characteristicCellSizeK = cvf::Math::sqrt( kLengthAccumulated / divisor );
        }
    }

    *iSize = m_characteristicCellSizeI;
    *jSize = m_characteristicCellSizeJ;
    *kSize = m_characteristicCellSizeK;
}

} // namespace cvf
