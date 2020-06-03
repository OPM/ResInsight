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

#include "RigCaseToCaseCellMapperTools.h"
#include "RigCaseToCaseCellMapper.h"

#include "RigFemPart.h"
#include "RigFemPartGrid.h"
#include "RigMainGrid.h"

//==================================================================================================
///
//==================================================================================================

class RigNeighborCornerFinder
{
public:
    RigNeighborCornerFinder( const RigMainGrid* mainGrid, size_t baseI, size_t baseJ, size_t baseK )
        : m_mainGrid( mainGrid )
        , m_baseI( baseI )
        , m_baseJ( baseJ )
        , m_baseK( baseK )
    {
    }

    const std::array<size_t, 8>* neighborIndices( int offsetI, int offsetJ, int offsetK )
    {
        if ( offsetI < 0 && m_baseI == 0 ) return nullptr;
        if ( offsetJ < 0 && m_baseJ == 0 ) return nullptr;
        if ( offsetK < 0 && m_baseK == 0 ) return nullptr;
        if ( offsetI > 0 && m_baseI == m_mainGrid->cellCountI() - 1 ) return nullptr;
        if ( offsetJ > 0 && m_baseJ == m_mainGrid->cellCountJ() - 1 ) return nullptr;
        if ( offsetK > 0 && m_baseK == m_mainGrid->cellCountK() - 1 ) return nullptr;

        size_t gridLocalCellIndex = m_mainGrid->cellIndexFromIJK( m_baseI + offsetI, m_baseJ + offsetJ, m_baseK + offsetK );
        const RigCell& cell       = m_mainGrid->globalCellArray()[gridLocalCellIndex];
        return &( cell.cornerIndices() );
    }

private:
    const RigMainGrid* m_mainGrid;
    size_t             m_baseI;
    size_t             m_baseJ;
    size_t             m_baseK;
};

//==================================================================================================
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Average of neighbor corresponding nodes
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseCellMapperTools::estimatedFemCellFromEclCell( const RigMainGrid* eclGrid,
                                                                size_t             reservoirCellIndex,
                                                                cvf::Vec3d         estimatedElmCorners[8] )
{
    CVF_TIGHT_ASSERT( reservoirCellIndex < eclGrid->cellCount() ); // Assume reservoirCellIdx == localGridCellIdx for
                                                                   // maingrid

    const std::vector<cvf::Vec3d>& eclNodes = eclGrid->nodes();

    size_t I, J, K;
    eclGrid->ijkFromCellIndex( reservoirCellIndex, &I, &J, &K );
    RigNeighborCornerFinder nbFinder( eclGrid, I, J, K );

    // Cell corner Averaging mapping: Local cell index in neighbor matching specific corner of this cell
    // N - Negative P - positive
    // 0 <- NI[1] NINJ[2] NJ[3] NK[4] NINK[5] NINJNK[6] NJNK[7]
    // 1 <- NJ[2] PINJ[3] PI[0] NK[5] NJNK[6] PINJNK[7] PINK[4]
    // 2 <- PI[3] PIPJ[0] PJ[1] NK[6] PINK[7] PIPJNK[4] PJNK[5]
    // 3 <- PJ[0] NIPJ[1] NI[2] NK[7] PJNK[4] NIPJNK[5] NINK[6]
    // 4 <- NI[5] NINJ[6] NJ[7] PK[0] NIPK[1] NINJPK[2] NJPK[3]
    // 5 <- NJ[6] PINJ[7] PI[4] PK[1] NJPK[2] PINJPK[3] PIPK[0]
    // 6 <- PI[7] PIPJ[4] PJ[5] PK[2] PIPK[3] PIPJPK[0] PJPK[1]
    // 7 <- PJ[4] NIPJ[5] NI[6] PK[3] PJPK[0] NIPJPK[1] NIPK[2]

    const std::array<size_t, 8>* IJK  = nbFinder.neighborIndices( 0, 0, 0 );
    const std::array<size_t, 8>* NI   = nbFinder.neighborIndices( -1, 0, 0 );
    const std::array<size_t, 8>* NJ   = nbFinder.neighborIndices( 0, -1, 0 );
    const std::array<size_t, 8>* PI   = nbFinder.neighborIndices( 1, 0, 0 );
    const std::array<size_t, 8>* PJ   = nbFinder.neighborIndices( 0, 1, 0 );
    const std::array<size_t, 8>* NK   = nbFinder.neighborIndices( 0, 0, -1 );
    const std::array<size_t, 8>* PK   = nbFinder.neighborIndices( 0, 0, 1 );
    const std::array<size_t, 8>* NINJ = nbFinder.neighborIndices( -1, -1, 0 );
    const std::array<size_t, 8>* PINJ = nbFinder.neighborIndices( 1, -1, 0 );

    const std::array<size_t, 8>* PIPJ = nbFinder.neighborIndices( 1, 1, 0 );
    const std::array<size_t, 8>* NIPJ = nbFinder.neighborIndices( -1, 1, 0 );
    const std::array<size_t, 8>* NINK = nbFinder.neighborIndices( -1, 0, -1 );
    const std::array<size_t, 8>* NJNK = nbFinder.neighborIndices( 0, -1, -1 );
    const std::array<size_t, 8>* PINK = nbFinder.neighborIndices( 1, 0, -1 );
    const std::array<size_t, 8>* PJNK = nbFinder.neighborIndices( 0, 1, -1 );
    const std::array<size_t, 8>* NIPK = nbFinder.neighborIndices( -1, 0, 1 );
    const std::array<size_t, 8>* NJPK = nbFinder.neighborIndices( 0, -1, 1 );
    const std::array<size_t, 8>* PIPK = nbFinder.neighborIndices( 1, 0, 1 );

    const std::array<size_t, 8>* PJPK   = nbFinder.neighborIndices( 0, 1, 1 );
    const std::array<size_t, 8>* NINJNK = nbFinder.neighborIndices( -1, -1, -1 );
    const std::array<size_t, 8>* PINJNK = nbFinder.neighborIndices( 1, -1, -1 );
    const std::array<size_t, 8>* PIPJNK = nbFinder.neighborIndices( 1, 1, -1 );
    const std::array<size_t, 8>* NIPJNK = nbFinder.neighborIndices( -1, 1, -1 );
    const std::array<size_t, 8>* NINJPK = nbFinder.neighborIndices( -1, -1, 1 );
    const std::array<size_t, 8>* PINJPK = nbFinder.neighborIndices( 1, -1, 1 );
    const std::array<size_t, 8>* PIPJPK = nbFinder.neighborIndices( 1, 1, 1 );
    const std::array<size_t, 8>* NIPJPK = nbFinder.neighborIndices( -1, 1, 1 );

    std::vector<size_t> contributingNodeIndicesPrCellCorner[8];

    if ( IJK ) contributingNodeIndicesPrCellCorner[0].push_back( ( *IJK )[0] );
    if ( NI ) contributingNodeIndicesPrCellCorner[0].push_back( ( *NI )[1] );
    if ( NINJ ) contributingNodeIndicesPrCellCorner[0].push_back( ( *NINJ )[2] );
    if ( NJ ) contributingNodeIndicesPrCellCorner[0].push_back( ( *NJ )[3] );
    if ( NK ) contributingNodeIndicesPrCellCorner[0].push_back( ( *NK )[4] );
    if ( NINK ) contributingNodeIndicesPrCellCorner[0].push_back( ( *NINK )[5] );
    if ( NINJNK ) contributingNodeIndicesPrCellCorner[0].push_back( ( *NINJNK )[6] );
    if ( NJNK ) contributingNodeIndicesPrCellCorner[0].push_back( ( *NJNK )[7] );

    if ( IJK ) contributingNodeIndicesPrCellCorner[1].push_back( ( *IJK )[1] );
    if ( NJ ) contributingNodeIndicesPrCellCorner[1].push_back( ( *NJ )[2] );
    if ( PINJ ) contributingNodeIndicesPrCellCorner[1].push_back( ( *PINJ )[3] );
    if ( PI ) contributingNodeIndicesPrCellCorner[1].push_back( ( *PI )[0] );
    if ( NK ) contributingNodeIndicesPrCellCorner[1].push_back( ( *NK )[5] );
    if ( NJNK ) contributingNodeIndicesPrCellCorner[1].push_back( ( *NJNK )[6] );
    if ( PINJNK ) contributingNodeIndicesPrCellCorner[1].push_back( ( *PINJNK )[7] );
    if ( PINK ) contributingNodeIndicesPrCellCorner[1].push_back( ( *PINK )[4] );

    if ( IJK ) contributingNodeIndicesPrCellCorner[2].push_back( ( *IJK )[2] );
    if ( PI ) contributingNodeIndicesPrCellCorner[2].push_back( ( *PI )[3] );
    if ( PIPJ ) contributingNodeIndicesPrCellCorner[2].push_back( ( *PIPJ )[0] );
    if ( PJ ) contributingNodeIndicesPrCellCorner[2].push_back( ( *PJ )[1] );
    if ( NK ) contributingNodeIndicesPrCellCorner[2].push_back( ( *NK )[6] );
    if ( PINK ) contributingNodeIndicesPrCellCorner[2].push_back( ( *PINK )[7] );
    if ( PIPJNK ) contributingNodeIndicesPrCellCorner[2].push_back( ( *PIPJNK )[4] );
    if ( PJNK ) contributingNodeIndicesPrCellCorner[2].push_back( ( *PJNK )[5] );

    if ( IJK ) contributingNodeIndicesPrCellCorner[3].push_back( ( *IJK )[3] );
    if ( PJ ) contributingNodeIndicesPrCellCorner[3].push_back( ( *PJ )[0] );
    if ( NIPJ ) contributingNodeIndicesPrCellCorner[3].push_back( ( *NIPJ )[1] );
    if ( NI ) contributingNodeIndicesPrCellCorner[3].push_back( ( *NI )[2] );
    if ( NK ) contributingNodeIndicesPrCellCorner[3].push_back( ( *NK )[7] );
    if ( PJNK ) contributingNodeIndicesPrCellCorner[3].push_back( ( *PJNK )[4] );
    if ( NIPJNK ) contributingNodeIndicesPrCellCorner[3].push_back( ( *NIPJNK )[5] );
    if ( NINK ) contributingNodeIndicesPrCellCorner[3].push_back( ( *NINK )[6] );

    // 4 <- NI[5] NINJ[6] NJ[7] PK[0] NIPK[1] NINJPK[2] NJPK[3]

    if ( IJK ) contributingNodeIndicesPrCellCorner[4].push_back( ( *IJK )[4] );
    if ( NI ) contributingNodeIndicesPrCellCorner[4].push_back( ( *NI )[5] );
    if ( NINJ ) contributingNodeIndicesPrCellCorner[4].push_back( ( *NINJ )[6] );
    if ( NJ ) contributingNodeIndicesPrCellCorner[4].push_back( ( *NJ )[7] );
    if ( PK ) contributingNodeIndicesPrCellCorner[4].push_back( ( *PK )[0] );
    if ( NIPK ) contributingNodeIndicesPrCellCorner[4].push_back( ( *NIPK )[1] );
    if ( NINJPK ) contributingNodeIndicesPrCellCorner[4].push_back( ( *NINJPK )[2] );
    if ( NJPK ) contributingNodeIndicesPrCellCorner[4].push_back( ( *NJPK )[3] );

    if ( IJK ) contributingNodeIndicesPrCellCorner[5].push_back( ( *IJK )[5] );
    if ( NJ ) contributingNodeIndicesPrCellCorner[5].push_back( ( *NJ )[6] );
    if ( PINJ ) contributingNodeIndicesPrCellCorner[5].push_back( ( *PINJ )[7] );
    if ( PI ) contributingNodeIndicesPrCellCorner[5].push_back( ( *PI )[4] );
    if ( PK ) contributingNodeIndicesPrCellCorner[5].push_back( ( *PK )[1] );
    if ( NJPK ) contributingNodeIndicesPrCellCorner[5].push_back( ( *NJPK )[2] );
    if ( PINJPK ) contributingNodeIndicesPrCellCorner[5].push_back( ( *PINJPK )[3] );
    if ( PIPK ) contributingNodeIndicesPrCellCorner[5].push_back( ( *PIPK )[0] );

    // 6 <- PI[7] PIPJ[4] PJ[5] PK[2] PIPK[3] PIPJPK[0] PJPK[1]

    if ( IJK ) contributingNodeIndicesPrCellCorner[6].push_back( ( *IJK )[6] );
    if ( PI ) contributingNodeIndicesPrCellCorner[6].push_back( ( *PI )[7] );
    if ( PIPJ ) contributingNodeIndicesPrCellCorner[6].push_back( ( *PIPJ )[4] );
    if ( PJ ) contributingNodeIndicesPrCellCorner[6].push_back( ( *PJ )[5] );
    if ( PK ) contributingNodeIndicesPrCellCorner[6].push_back( ( *PK )[2] );
    if ( PIPK ) contributingNodeIndicesPrCellCorner[6].push_back( ( *PIPK )[3] );
    if ( PIPJPK ) contributingNodeIndicesPrCellCorner[6].push_back( ( *PIPJPK )[0] );
    if ( PJPK ) contributingNodeIndicesPrCellCorner[6].push_back( ( *PJPK )[1] );

    if ( IJK ) contributingNodeIndicesPrCellCorner[7].push_back( ( *IJK )[7] );
    if ( PJ ) contributingNodeIndicesPrCellCorner[7].push_back( ( *PJ )[4] );
    if ( NIPJ ) contributingNodeIndicesPrCellCorner[7].push_back( ( *NIPJ )[5] );
    if ( NI ) contributingNodeIndicesPrCellCorner[7].push_back( ( *NI )[6] );
    if ( PK ) contributingNodeIndicesPrCellCorner[7].push_back( ( *PK )[3] );
    if ( PJPK ) contributingNodeIndicesPrCellCorner[7].push_back( ( *PJPK )[0] );
    if ( NIPJPK ) contributingNodeIndicesPrCellCorner[7].push_back( ( *NIPJPK )[1] );
    if ( NIPK ) contributingNodeIndicesPrCellCorner[7].push_back( ( *NIPK )[2] );

    // Average the nodes
    for ( size_t cornIdx = 0; cornIdx < 8; ++cornIdx )
    {
        estimatedElmCorners[cornIdx] = cvf::Vec3d::ZERO;
        size_t contribCount          = contributingNodeIndicesPrCellCorner[cornIdx].size();
        for ( size_t ctnIdx = 0; ctnIdx < contribCount; ++ctnIdx )
        {
            estimatedElmCorners[cornIdx] += eclNodes[contributingNodeIndicesPrCellCorner[cornIdx][ctnIdx]];
        }
        estimatedElmCorners[cornIdx] /= contribCount;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseCellMapperTools::rotateQuad( cvf::Vec3d quad[4], int idxToNewStart )
{
    if ( idxToNewStart == 0 ) return;
    cvf::Vec3d tmpQuad[4];
    tmpQuad[0] = quad[0];
    tmpQuad[1] = quad[1];
    tmpQuad[2] = quad[2];
    tmpQuad[3] = quad[3];

    quad[0] = tmpQuad[idxToNewStart];
    ++idxToNewStart;
    if ( idxToNewStart > 3 ) idxToNewStart = 0;
    quad[1] = tmpQuad[idxToNewStart];
    ++idxToNewStart;
    if ( idxToNewStart > 3 ) idxToNewStart = 0;
    quad[2] = tmpQuad[idxToNewStart];
    ++idxToNewStart;
    if ( idxToNewStart > 3 ) idxToNewStart = 0;
    quad[3] = tmpQuad[idxToNewStart];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseCellMapperTools::flipQuadWinding( cvf::Vec3d quad[4] )
{
    cvf::Vec3d temp = quad[1];
    quad[1]         = quad[3];
    quad[3]         = temp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigCaseToCaseCellMapperTools::quadVxClosestToXYOfPoint( const cvf::Vec3d point, const cvf::Vec3d quad[4] )
{
    double minSqDist               = HUGE_VAL;
    int    quadVxIdxClosestToPoint = cvf::UNDEFINED_INT;

    for ( int i = 0; i < 4; ++i )
    {
        cvf::Vec3d diff = quad[i] - point;
        diff[2]         = 0.0;

        double sqDist = diff.lengthSquared();
        if ( sqDist < minSqDist )
        {
            minSqDist               = sqDist;
            quadVxIdxClosestToPoint = i;
        }
    }

    return quadVxIdxClosestToPoint;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseToCaseCellMapperTools::elementCorners( const RigFemPart* femPart, int elmIdx, cvf::Vec3d elmCorners[8] )
{
    RigElementType elmType = femPart->elementType( elmIdx );
    if ( !( elmType == HEX8 || elmType == HEX8P ) ) return false;

    const std::vector<cvf::Vec3f>& nodeCoords    = femPart->nodes().coordinates;
    const int*                     cornerIndices = femPart->connectivities( elmIdx );

    elmCorners[0] = cvf::Vec3d( nodeCoords[cornerIndices[0]] );
    elmCorners[1] = cvf::Vec3d( nodeCoords[cornerIndices[1]] );
    elmCorners[2] = cvf::Vec3d( nodeCoords[cornerIndices[2]] );
    elmCorners[3] = cvf::Vec3d( nodeCoords[cornerIndices[3]] );
    elmCorners[4] = cvf::Vec3d( nodeCoords[cornerIndices[4]] );
    elmCorners[5] = cvf::Vec3d( nodeCoords[cornerIndices[5]] );
    elmCorners[6] = cvf::Vec3d( nodeCoords[cornerIndices[6]] );
    elmCorners[7] = cvf::Vec3d( nodeCoords[cornerIndices[7]] );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigCaseToCaseCellMapperTools::findMatchingPOSKFaceIdx( const cvf::Vec3d baseCell[8],
                                                           bool             isBaseCellNormalsOutwards,
                                                           const cvf::Vec3d c2[8] )
{
    int        faceNodeCount;
    const int* posKFace =
        RigFemTypes::localElmNodeIndicesForFace( HEX8, (int)( cvf::StructGridInterface::POS_K ), &faceNodeCount );

    double sign = isBaseCellNormalsOutwards ? 1.0 : -1.0;

    cvf::Vec3d posKnormal = sign * ( baseCell[posKFace[2]] - baseCell[posKFace[0]] ) ^
                            ( baseCell[posKFace[3]] - baseCell[posKFace[1]] );
    posKnormal.normalize();

    double minDiff  = HUGE_VAL;
    int    bestFace = -1;
    for ( int faceIdx = 5; faceIdx >= 0; --faceIdx ) // Backwards. might hit earlier more often
    {
        const int* face   = RigFemTypes::localElmNodeIndicesForFace( HEX8, faceIdx, &faceNodeCount );
        cvf::Vec3d normal = ( c2[face[2]] - c2[face[0]] ) ^ ( c2[face[3]] - c2[face[1]] );
        normal.normalize();
        double sqDiff = ( posKnormal - normal ).lengthSquared();
        if ( sqDiff < minDiff )
        {
            minDiff  = sqDiff;
            bestFace = faceIdx;
            if ( minDiff < 0.1 * 0.1 ) break; // This must be the one. Do not search further
        }
    }

    return bestFace;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseToCaseCellMapperTools::isEclFemCellsMatching( const cvf::Vec3d baseCell[8],
                                                          cvf::Vec3d       cell[8],
                                                          double           xyTolerance,
                                                          double           zTolerance )
{
    bool isMatching = true;

    for ( int i = 0; i < 4; ++i )
    {
        cvf::Vec3d diff = cell[i] - baseCell[i];

        if ( !( fabs( diff.x() ) < xyTolerance && fabs( diff.y() ) < xyTolerance && fabs( diff.z() ) < zTolerance ) )
        {
            isMatching = false;
            break;
        }
    }

    return isMatching;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseCellMapperTools::rotateCellTopologicallyToMatchBaseCell( const cvf::Vec3d* baseCell,
                                                                           bool        baseCellFaceNormalsIsOutwards,
                                                                           cvf::Vec3d* cell )
{
    int femDeepZFaceIdx = findMatchingPOSKFaceIdx( baseCell, baseCellFaceNormalsIsOutwards, cell );

    {
        cvf::Vec3d tmpFemCorners[8];
        tmpFemCorners[0] = cell[0];
        tmpFemCorners[1] = cell[1];
        tmpFemCorners[2] = cell[2];
        tmpFemCorners[3] = cell[3];
        tmpFemCorners[4] = cell[4];
        tmpFemCorners[5] = cell[5];
        tmpFemCorners[6] = cell[6];
        tmpFemCorners[7] = cell[7];

        int femShallowZFaceIdx = RigFemTypes::oppositeFace( HEX8, femDeepZFaceIdx );

        int        faceNodeCount;
        const int* localElmNodeIndicesForPOSKFace =
            RigFemTypes::localElmNodeIndicesForFace( HEX8, femDeepZFaceIdx, &faceNodeCount );
        const int* localElmNodeIndicesForNEGKFace =
            RigFemTypes::localElmNodeIndicesForFace( HEX8, femShallowZFaceIdx, &faceNodeCount );

        cell[0] = tmpFemCorners[localElmNodeIndicesForNEGKFace[0]];
        cell[1] = tmpFemCorners[localElmNodeIndicesForNEGKFace[1]];
        cell[2] = tmpFemCorners[localElmNodeIndicesForNEGKFace[2]];
        cell[3] = tmpFemCorners[localElmNodeIndicesForNEGKFace[3]];
        cell[4] = tmpFemCorners[localElmNodeIndicesForPOSKFace[0]];
        cell[5] = tmpFemCorners[localElmNodeIndicesForPOSKFace[1]];
        cell[6] = tmpFemCorners[localElmNodeIndicesForPOSKFace[2]];
        cell[7] = tmpFemCorners[localElmNodeIndicesForPOSKFace[3]];
    }

    cvf::Vec3d* femDeepestQuad = &( cell[4] );
    cvf::Vec3d* femShallowQuad = &( cell[0] );

    // Now the top/bottom have opposite winding. To make the comparisons and index rotations simpler
    // flip the winding of the top or bottom face depending on whether the eclipse grid is inside-out

    if ( baseCellFaceNormalsIsOutwards )
    {
        flipQuadWinding( femShallowQuad );
    }
    else
    {
        flipQuadWinding( femDeepestQuad );
    }

    // We now need to rotate the fem quads to be aligned with the ecl quads
    // Since the start point of the quad always is aligned with the opposite face-quad start
    // we can find the rotation for the top, and apply it to both top and bottom

    int femQuadStartIdx = quadVxClosestToXYOfPoint( baseCell[0], femShallowQuad );
    rotateQuad( femDeepestQuad, femQuadStartIdx );
    rotateQuad( femShallowQuad, femQuadStartIdx );
}

#if 0 // Inside Bounding box test
cvf::BoundingBox cellBBox;
for (int i = 0; i < 8 ; ++i) cellBBox.add(cellCorners[i]);

cvf::Vec3d cs = cellBBox.min();
cvf::Vec3d cl = cellBBox.max();
cvf::Vec3d es = elmBBox.min();
cvf::Vec3d el = elmBBox.max();

if (   ( (cs.x() + xyTolerance) >= es.x() && (cl.x() - xyTolerance) <= el.x())
    && ( (cs.y() + xyTolerance) >= es.y() && (cl.y() - xyTolerance) <= el.y())
    && ( (cs.z() + zTolerance ) >= es.z() && (cl.z() - zTolerance ) <= el.z()) )
{
    // Cell bb equal or inside Elm bb   
    isMatching = true;
}

if (   ( (es.x() + xyTolerance)  >= cs.x() && (el.x() - xyTolerance) <= cl.x())
    && ( (es.y() + xyTolerance)  >= cs.y() && (el.y() - xyTolerance) <= cl.y())
    && ( (es.z() + zTolerance )  >= cs.z() && (el.z() - zTolerance ) <= cl.z()) )
{
    // Elm bb equal or inside Cell bb   
    isMatching = true;
}
#endif

#if 0
    {
        const std::vector<cvf::Vec3d>& eclNodes = eclGrid->nodes();
        const RigCell& cell = eclGrid->cells()[reservoirCellIndex];
        const std::array<size_t, 8>& cornerIndices = cell.cornerIndices();
        int faceNodeCount;
        const int*  localElmNodeIndicesForTopZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, 4, &faceNodeCount);
        const int*  localElmNodeIndicesForBotZFace = RigFemTypes::localElmNodeIndicesForFace(HEX8, 5, &faceNodeCount);

        eclDeepestQuad[0] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[0]]];
        eclDeepestQuad[1] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[1]]];
        eclDeepestQuad[2] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[2]]];
        eclDeepestQuad[3] = eclNodes[cornerIndices[localElmNodeIndicesForTopZFace[3]]];

        eclShallowQuad[0] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[0]]];
        eclShallowQuad[1] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[1]]];
        eclShallowQuad[2] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[2]]];
        eclShallowQuad[3] = eclNodes[cornerIndices[localElmNodeIndicesForBotZFace[3]]];
    }
#endif

#if 0
// First search K=1 diagonally for a seed cell; A cell without collapsings, and without faults

size_t minIJCount = masterEclGrid->cellCountI();
if (minIJCount > masterEclGrid->cellCountJ())
minIJCount =  masterEclGrid->cellCountJ();

for (size_t ij = 0; ij < minIJCount; ++ij )
{
    size_t localCellIdx = masterEclGrid->cellIndexFromIJK(ij, ij, 0);
    size_t reservoirCellIdx = masterEclGrid->reservoirCellIndex(localCellIdx);

    cvf::Vec3d vertices[8];
    masterEclGrid->cellCornerVertices(localCellIdx, vertices);
    if (!isCellNormal(vertices))
        continue;

    const RigFault* fault = masterEclGrid->findFaultFromCellIndexAndCellFace(reservoirCellIdx, cvf::StructGridInterface::POS_I);

}
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigCaseToCaseCellMapperTools::calculateCellCenter( cvf::Vec3d elmCorners[8] )
{
    cvf::Vec3d avg( cvf::Vec3d::ZERO );

    size_t i;
    for ( i = 0; i < 8; i++ )
    {
        avg += elmCorners[i];
    }

    avg /= 8.0;

    return avg;
}
