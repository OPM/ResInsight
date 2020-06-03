/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemPartGrid.h"

#include "RigFemPart.h"

#include <array>
#include <cmath>
#include <limits.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartGrid::RigFemPartGrid()
    : m_femPart( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartGrid::~RigFemPartGrid()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartGrid::setFemPart( const RigFemPart* femPart )
{
    m_femPart = femPart;
    generateStructGridData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartGrid::generateStructGridData()
{
    //[X] 1. Calculate neighbors for each element
    //[X]    record the ones with 3 or fewer neighbors as possible grid corners
    //[X] 2. Loop over the possible corner cells,
    //[X]    find the one that corresponds to IJK = 000
    //[X]    by finding the one closest to origo // Does not work
    //[X]    by Determining what surfs correspond to NEG IJK surfaces in that element,
    //       and that none of those faces have a neighbor
    //[X] 4. Assign IJK = 000 to that element
    //[X]    Store IJK in elm idx array
    //[X] 5. Loop along POS I surfaces increment I for each element and assign IJK
    //[X]    when at end, go to POS J neighbor, increment J, repeat above.
    //[X]    etc for POS Z
    //[X]    Find max IJK as you go,
    //[ ]    also assert that there are no NEG I/NEG J/NEG Z neighbors when starting on a new row
    //[ ]    (Need to find min, and offset IJK values if there exists such)
    //[ ] 6. If IJK to elm idx is needed, allocate "grid" with maxI,maxJ,maxZ values
    //[ ]    Loop over elms, assign elmIdx to IJK address in grid

    int elmIdxForIJK_000 = findElmIdxForIJK000();

    CVF_ASSERT( elmIdxForIJK_000 != -1 ); // Debug. When we have run enough tests, remove

    if ( elmIdxForIJK_000 == -1 ) return;

    // Find the IJK faces based on the corner cell

    cvf::Vec3i ijkMainFaceIndices = findMainIJKFaces( elmIdxForIJK_000 );

    // assign ijk to cells
    {
        m_ijkPrElement.resize( m_femPart->elementCount(), cvf::Vec3i( -1, -1, -1 ) );

        int posIFaceIdx = ijkMainFaceIndices[0];
        int posJFaceIdx = ijkMainFaceIndices[1];
        int posKFaceIdx = ijkMainFaceIndices[2];

        m_elementIJKCounts = cvf::Vec3st( 0, 0, 0 );

        int        elmIdxInK  = elmIdxForIJK_000;
        cvf::Vec3f posKNormal = m_femPart->faceNormal( elmIdxInK, posKFaceIdx );
        int        kCoord     = 0;
        while ( true )
        {
            int        elmIdxInJ          = elmIdxInK;
            cvf::Vec3f startElmInKNormalJ = m_femPart->faceNormal( elmIdxInJ, posJFaceIdx );
            cvf::Vec3f startElmInKNormalI = m_femPart->faceNormal( elmIdxInJ, posIFaceIdx );

            int jCoord = 0;
            while ( true )
            {
                int        elmIdxInI          = elmIdxInJ;
                cvf::Vec3f startElmInJNormalI = m_femPart->faceNormal( elmIdxInI, posIFaceIdx );
                int        iCoord             = 0;
                while ( true )
                {
                    CVF_ASSERT( elmIdxInI >= 0 && size_t( elmIdxInI ) < m_ijkPrElement.size() );
                    // Assign ijk coordinate
                    m_ijkPrElement[elmIdxInI] = cvf::Vec3i( iCoord, jCoord, kCoord );

                    ++iCoord;

                    // Find neighbor and exit if at end
                    int neighborElmIdx = m_femPart->elementNeighbor( elmIdxInI, posIFaceIdx );
                    if ( neighborElmIdx == -1 ) break;

                    // Find the continuing face in the neighbor element (opposite of the neighbor face)
                    int neighborNegFaceIdx = m_femPart->neighborFace( elmIdxInI, posIFaceIdx );

                    RigElementType eType = m_femPart->elementType( neighborElmIdx );
                    posIFaceIdx          = RigFemTypes::oppositeFace( eType, neighborNegFaceIdx );

                    // Step to neighbor
                    elmIdxInI = neighborElmIdx;
                }

                // Scoped to show that nothing bleeds further to K-loop
                {
                    if ( iCoord > static_cast<int>( m_elementIJKCounts[0] ) ) m_elementIJKCounts[0] = iCoord;

                    ++jCoord;

                    // Find neighbor and exit if at end
                    int neighborElmIdx = m_femPart->elementNeighbor( elmIdxInJ, posJFaceIdx );
                    if ( neighborElmIdx == -1 ) break;

                    // Find the continuing face in the neighbor element (opposite of the neighbor face)
                    int neighborNegFaceIdx = m_femPart->neighborFace( elmIdxInJ, posJFaceIdx );

                    RigElementType eType = m_femPart->elementType( neighborElmIdx );
                    posJFaceIdx          = RigFemTypes::oppositeFace( eType, neighborNegFaceIdx );

                    // Now where is posIFace of the new J cell ?
                    posIFaceIdx = perpendicularFaceInDirection( startElmInJNormalI, neighborNegFaceIdx, neighborElmIdx );

                    // Step to neighbor
                    elmIdxInJ = neighborElmIdx;
                }
            }

            {
                if ( jCoord > static_cast<int>( m_elementIJKCounts[1] ) ) m_elementIJKCounts[1] = jCoord;

                ++kCoord;

                // Find neighbor and exit if at end
                int neighborElmIdx = m_femPart->elementNeighbor( elmIdxInK, posKFaceIdx );
                if ( neighborElmIdx == -1 ) break;

                // Find the continuing face in the neighbor element (opposite of the neighbor face)
                int neighborNegFaceIdx = m_femPart->neighborFace( elmIdxInK, posKFaceIdx );

                RigElementType eType = m_femPart->elementType( neighborElmIdx );
                posKFaceIdx          = RigFemTypes::oppositeFace( eType, neighborNegFaceIdx );

                // Now where is posJFace of the new K cell ?
                posJFaceIdx = perpendicularFaceInDirection( startElmInKNormalJ, neighborNegFaceIdx, neighborElmIdx );
                posIFaceIdx = perpendicularFaceInDirection( startElmInKNormalI, neighborNegFaceIdx, neighborElmIdx );

                // Step to neighbor
                elmIdxInK = neighborElmIdx;
            }
        }

        if ( kCoord > static_cast<int>( m_elementIJKCounts[2] ) ) m_elementIJKCounts[2] = kCoord;
    }

    m_elmIdxPrIJK.resize( m_elementIJKCounts[0], m_elementIJKCounts[1], m_elementIJKCounts[2] );

    for ( int elmIdx = 0; elmIdx < m_femPart->elementCount(); ++elmIdx )
    {
        size_t i, j, k;
        bool   validIndex = ijkFromCellIndex( elmIdx, &i, &j, &k );
        if ( validIndex )
        {
            m_elmIdxPrIJK.at( i, j, k ) = elmIdx;
        }
    }

    // IJK bounding box
    m_reservoirIJKBoundingBox.first  = cvf::Vec3st( INT_MAX, INT_MAX, INT_MAX );
    m_reservoirIJKBoundingBox.second = cvf::Vec3st( 0, 0, 0 );
    cvf::Vec3st& min                 = m_reservoirIJKBoundingBox.first;
    cvf::Vec3st& max                 = m_reservoirIJKBoundingBox.second;

    for ( int elmIdx = 0; elmIdx < m_femPart->elementCount(); ++elmIdx )
    {
        RigElementType elementType = m_femPart->elementType( elmIdx );
        size_t         i, j, k;
        bool           validIndex = ijkFromCellIndex( elmIdx, &i, &j, &k );
        if ( elementType == HEX8P && validIndex )
        {
            if ( i < min.x() ) min.x() = i;
            if ( j < min.y() ) min.y() = j;
            if ( k < min.z() ) min.z() = k;
            if ( i > max.x() ) max.x() = i;
            if ( j > max.y() ) max.y() = j;
            if ( k > max.z() ) max.z() = k;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPartGrid::findElmIdxForIJK000()
{
    const std::vector<int>& possibleGridCorners = m_femPart->possibleGridCornerElements();
    size_t                  possibleCornerCount = possibleGridCorners.size();

    for ( size_t pcIdx = 0; pcIdx < possibleCornerCount; ++pcIdx )
    {
        int        elmIdx             = possibleGridCorners[pcIdx];
        cvf::Vec3i ijkMainFaceIndices = findMainIJKFaces( elmIdx );

        if ( m_femPart->elementNeighbor( elmIdx, ijkMainFaceIndices[0] ) != -1 &&
             m_femPart->elementNeighbor( elmIdx, ijkMainFaceIndices[1] ) != -1 &&
             m_femPart->elementNeighbor( elmIdx, ijkMainFaceIndices[2] ) != -1 )
        {
            return elmIdx;
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RigFemPartGrid::findMainIJKFaces( int elementIndex ) const
{
    cvf::Vec3i ijkMainFaceIndices = cvf::Vec3i( -1, -1, -1 );

    RigElementType          eType     = m_femPart->elementType( elementIndex );
    int                     faceCount = RigFemTypes::elementFaceCount( eType );
    std::vector<cvf::Vec3f> normals( faceCount );
    for ( int faceIdx = 0; faceIdx < faceCount; ++faceIdx )
    {
        normals[faceIdx] = m_femPart->faceNormal( elementIndex, faceIdx );
    }

    // Record three independent main direction vectors for the element, and what face they are created from
    cvf::Vec3f mainElmDirections[3];
    int        mainElmDirOriginFaces[3];
    if ( eType == HEX8 || eType == HEX8P )
    {
        mainElmDirections[0]     = normals[0] - normals[1]; // To get a better "average" direction vector
        mainElmDirections[1]     = normals[2] - normals[3];
        mainElmDirections[2]     = normals[4] - normals[5];
        mainElmDirOriginFaces[0] = 0;
        mainElmDirOriginFaces[1] = 2;
        mainElmDirOriginFaces[2] = 4;
    }
    else
    {
        mainElmDirections[0] = cvf::Vec3f::ZERO;
        mainElmDirections[1] = cvf::Vec3f::ZERO;
        mainElmDirections[2] = cvf::Vec3f::ZERO;

        CVF_ASSERT( false );
    }

    mainElmDirections[0].normalize();
    mainElmDirections[1].normalize();
    mainElmDirections[2].normalize();

    // Match the element main directions with best XYZ match (IJK respectively)
    // Find the mainElmDirection with the largest component starting with Z
    // and use that for the corresponding IJK direction.
    // Find the Z (for K) first. Then select among the other two the Y (for J),
    // and select the remaining for I

    int mainElmDirectionIdxForIJK[3] = {-1, -1, -1};
    for ( int cIdx = 2; cIdx >= 0; --cIdx ) // Check Z first as it is more important
    {
        double maxAbsComp = -1.0;
        int    usedDir1   = -1;
        int    usedDir2   = -1;

        for ( int dIdx = 0; dIdx < 3; ++dIdx )
        {
            if ( dIdx == usedDir1 || dIdx == usedDir2 ) continue;

            float absComp = fabs( mainElmDirections[dIdx][cIdx] );
            if ( absComp > maxAbsComp )
            {
                maxAbsComp                      = absComp;
                mainElmDirectionIdxForIJK[cIdx] = dIdx;
            }
        }

        if ( usedDir1 == -1 )
            usedDir1 = mainElmDirectionIdxForIJK[cIdx];
        else
            usedDir2 = mainElmDirectionIdxForIJK[cIdx];
    }

    // Assign the correct face based on the main direction

    ijkMainFaceIndices[0] = ( mainElmDirections[mainElmDirectionIdxForIJK[0]] * cvf::Vec3f::X_AXIS > 0 )
                                ? mainElmDirOriginFaces[mainElmDirectionIdxForIJK[0]]
                                : RigFemTypes::oppositeFace( eType, mainElmDirOriginFaces[mainElmDirectionIdxForIJK[0]] );
    ijkMainFaceIndices[1] = ( mainElmDirections[mainElmDirectionIdxForIJK[1]] * cvf::Vec3f::Y_AXIS > 0 )
                                ? mainElmDirOriginFaces[mainElmDirectionIdxForIJK[1]]
                                : RigFemTypes::oppositeFace( eType, mainElmDirOriginFaces[mainElmDirectionIdxForIJK[1]] );
    ijkMainFaceIndices[2] = ( mainElmDirections[mainElmDirectionIdxForIJK[2]] * -cvf::Vec3f::Z_AXIS > 0 )
                                ? mainElmDirOriginFaces[mainElmDirectionIdxForIJK[2]]
                                : RigFemTypes::oppositeFace( eType, mainElmDirOriginFaces[mainElmDirectionIdxForIJK[2]] );

    return ijkMainFaceIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st> RigFemPartGrid::reservoirIJKBoundingBox() const
{
    return m_reservoirIJKBoundingBox;
}

//--------------------------------------------------------------------------------------------------
/// Find the face that is not perpFaceIdx or its opposite, and has normal closest to direction
//--------------------------------------------------------------------------------------------------

int RigFemPartGrid::perpendicularFaceInDirection( cvf::Vec3f direction, int perpFaceIdx, int elmIdx )
{
    RigElementType eType     = m_femPart->elementType( elmIdx );
    int            faceCount = RigFemTypes::elementFaceCount( eType );

    int oppFace = RigFemTypes::oppositeFace( eType, perpFaceIdx );

    double     minDiffSqLength = HUGE_VAL;
    cvf::Vec3f faceNormal;
    direction.normalize();
    int bestFace = -1;
    for ( int faceIdx = 0; faceIdx < faceCount; ++faceIdx )
    {
        if ( faceIdx == perpFaceIdx || faceIdx == oppFace ) continue;

        faceNormal = m_femPart->faceNormal( elmIdx, faceIdx );
        faceNormal.normalize();
        float diffSqLength = ( direction - faceNormal ).lengthSquared();
        if ( diffSqLength < minDiffSqLength )
        {
            bestFace        = faceIdx;
            minDiffSqLength = diffSqLength;
        }
    }

    return bestFace;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointCountI() const
{
    return m_elementIJKCounts[0] + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointCountJ() const
{
    return m_elementIJKCounts[1] + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointCountK() const
{
    return m_elementIJKCounts[2] + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::isCellValid( size_t i, size_t j, size_t k ) const
{
    CVF_ASSERT( false );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::minCoordinate() const
{
    CVF_ASSERT( false );
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::maxCoordinate() const
{
    CVF_ASSERT( false );
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::cellIJKNeighbor( size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex ) const
{
    CVF_ASSERT( false );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::cellIndexFromIJK( size_t i, size_t j, size_t k ) const
{
    return m_elmIdxPrIJK.at( i, j, k );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::ijkFromCellIndex( size_t cellIndex, size_t* i, size_t* j, size_t* k ) const
{
    if ( cellIndex < m_ijkPrElement.size() )
    {
        int signed_i = m_ijkPrElement[cellIndex][0];
        int signed_j = m_ijkPrElement[cellIndex][1];
        int signed_k = m_ijkPrElement[cellIndex][2];

        if ( signed_i >= 0 && signed_j >= 0 && signed_k >= 0 )
        {
            *i = signed_i;
            *j = signed_j;
            *k = signed_k;
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartGrid::cellIJKFromCoordinate( const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k ) const
{
    CVF_ASSERT( false );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartGrid::cellCornerVertices( size_t cellIndex, cvf::Vec3d vertices[8] ) const
{
    const std::vector<cvf::Vec3f>& nodeCoords    = m_femPart->nodes().coordinates;
    const int*                     cornerIndices = m_femPart->connectivities( cellIndex );

    for ( size_t i = 0; i < 8; ++i )
    {
        vertices[i] = cvf::Vec3d( nodeCoords[cornerIndices[i]] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::cellCentroid( size_t cellIndex ) const
{
    std::array<cvf::Vec3d, 8> cellVertices;
    this->cellCornerVertices( cellIndex, cellVertices.data() );

    cvf::Vec3d centroid( 0.0, 0.0, 0.0 );
    for ( int i = 0; i < 8; ++i )
    {
        centroid += cellVertices[i];
    }
    return centroid / 8.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartGrid::cellMinMaxCordinates( size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate ) const
{
    CVF_ASSERT( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartGrid::gridPointIndexFromIJK( size_t i, size_t j, size_t k ) const
{
    CVF_ASSERT( false );
    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartGrid::gridPointCoordinate( size_t i, size_t j, size_t k ) const
{
    CVF_ASSERT( false );
    return cvf::Vec3d::ZERO;
}
