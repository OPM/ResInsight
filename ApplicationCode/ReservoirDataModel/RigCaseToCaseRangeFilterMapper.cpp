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

#include "RigCaseToCaseRangeFilterMapper.h"
#include "RigCaseToCaseCellMapper.h"
#include "RigCaseToCaseCellMapperTools.h"

#include "RigFemPart.h"
#include "RigFemPartGrid.h"
#include "RigMainGrid.h"

#include "RimCellRangeFilter.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseRangeFilterMapper::convertRangeFilterEclToFem( RimCellRangeFilter* srcFilter,
                                                                 const RigMainGrid*  srcEclGrid,
                                                                 RimCellRangeFilter* dstFilter,
                                                                 const RigFemPart*   dstFemPart )
{
    convertRangeFilter( srcFilter, dstFilter, srcEclGrid, dstFemPart, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseRangeFilterMapper::convertRangeFilterFemToEcl( RimCellRangeFilter* srcFilter,
                                                                 const RigFemPart*   srcFemPart,
                                                                 RimCellRangeFilter* dstFilter,
                                                                 const RigMainGrid*  dstEclGrid )
{
    convertRangeFilter( srcFilter, dstFilter, dstEclGrid, srcFemPart, false );
}

struct RigRangeEndPoints
{
    RigRangeEndPoints()
        : StartI( cvf::UNDEFINED_SIZE_T )
        , StartJ( cvf::UNDEFINED_SIZE_T )
        , StartK( cvf::UNDEFINED_SIZE_T )
        , EndI( cvf::UNDEFINED_SIZE_T )
        , EndJ( cvf::UNDEFINED_SIZE_T )
        , EndK( cvf::UNDEFINED_SIZE_T )
    {
    }

    size_t StartI;
    size_t StartJ;
    size_t StartK;
    size_t EndI;
    size_t EndJ;
    size_t EndK;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

void RigCaseToCaseRangeFilterMapper::convertRangeFilter( const RimCellRangeFilter* srcFilter,
                                                         RimCellRangeFilter*       dstFilter,
                                                         const RigMainGrid*        eclGrid,
                                                         const RigFemPart*         femPart,
                                                         bool                      femIsDestination )
{
    CVF_ASSERT( srcFilter && eclGrid && dstFilter && femPart );
    CVF_ASSERT( srcFilter->gridIndex() == 0 ); // LGR not supported yet

    RigRangeEndPoints src;
    // Convert the (start, count) range filter vars to end point cell ijk
    {
        src.StartI = srcFilter->startIndexI() - 1;
        src.StartJ = srcFilter->startIndexJ() - 1;
        src.StartK = srcFilter->startIndexK() - 1;

        // Needs to subtract one more to have the end idx being
        // the last cell in the selection, not the first outside
        src.EndI = src.StartI + srcFilter->cellCountI() - 1;
        src.EndJ = src.StartJ + srcFilter->cellCountJ() - 1;
        src.EndK = src.StartK + srcFilter->cellCountK() - 1;
    }

    // Clamp the src end points to be inside the src model
    {
        size_t maxIIndex;
        size_t maxJIndex;
        size_t maxKIndex;

        // Clamp end
        if ( femIsDestination )
        {
            maxIIndex = eclGrid->cellCountI() - 1;
            maxJIndex = eclGrid->cellCountJ() - 1;
            maxKIndex = eclGrid->cellCountK() - 1;
        }
        else
        {
            maxIIndex = femPart->getOrCreateStructGrid()->cellCountI() - 1;
            maxJIndex = femPart->getOrCreateStructGrid()->cellCountJ() - 1;
            maxKIndex = femPart->getOrCreateStructGrid()->cellCountK() - 1;
        }
        src.EndI = CVF_MIN( src.EndI, maxIIndex );
        src.EndJ = CVF_MIN( src.EndJ, maxJIndex );
        src.EndK = CVF_MIN( src.EndK, maxKIndex );
    }

    // When using femPart as source we need to clamp the fem srcRange filter
    // to the extents of the ecl grid within the fem part before
    // doing the mapping. If not, the range filter corners will most likely be outside
    // the ecl grid, resulting in an undefined conversion.
    if ( !femIsDestination )
    {
        RigRangeEndPoints eclMaxMin;
        eclMaxMin.StartI = 0;
        eclMaxMin.StartJ = 0;
        eclMaxMin.StartK = 0;
        eclMaxMin.EndI   = eclGrid->cellCountI() - 1;
        eclMaxMin.EndJ   = eclGrid->cellCountJ() - 1;
        eclMaxMin.EndK   = eclGrid->cellCountK() - 1;
        RigRangeEndPoints eclExtInFem;

        convertRangeFilterEndPoints( eclMaxMin, eclExtInFem, eclGrid, femPart, true );

        src.StartI = CVF_MAX( src.StartI, eclExtInFem.StartI );
        src.StartJ = CVF_MAX( src.StartJ, eclExtInFem.StartJ );
        src.StartK = CVF_MAX( src.StartK, eclExtInFem.StartK );
        src.EndI   = CVF_MIN( src.EndI, eclExtInFem.EndI );
        src.EndJ   = CVF_MIN( src.EndJ, eclExtInFem.EndJ );
        src.EndK   = CVF_MIN( src.EndK, eclExtInFem.EndK );
    }

    RigRangeEndPoints dst;

    convertRangeFilterEndPoints( src, dst, eclGrid, femPart, femIsDestination );

    // Populate the dst range filter with new data

    if ( dst.StartI != cvf::UNDEFINED_SIZE_T && dst.StartJ != cvf::UNDEFINED_SIZE_T && dst.StartK != cvf::UNDEFINED_SIZE_T &&
         dst.EndI != cvf::UNDEFINED_SIZE_T && dst.EndJ != cvf::UNDEFINED_SIZE_T && dst.EndK != cvf::UNDEFINED_SIZE_T )
    {
        dstFilter->startIndexI = static_cast<int>( dst.StartI + 1 );
        dstFilter->startIndexJ = static_cast<int>( dst.StartJ + 1 );
        dstFilter->startIndexK = static_cast<int>( dst.StartK + 1 );

        dstFilter->cellCountI = static_cast<int>( dst.EndI - ( dst.StartI - 1 ) );
        dstFilter->cellCountJ = static_cast<int>( dst.EndJ - ( dst.StartJ - 1 ) );
        dstFilter->cellCountK = static_cast<int>( dst.EndK - ( dst.StartK - 1 ) );
    }
    else
    {
        dstFilter->startIndexI = 1;
        dstFilter->startIndexJ = 1;
        dstFilter->startIndexK = 1;

        dstFilter->cellCountI = 0;
        dstFilter->cellCountJ = 0;
        dstFilter->cellCountK = 0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseRangeFilterMapper::convertRangeFilterEndPoints( const RigRangeEndPoints& src,
                                                                  RigRangeEndPoints&       dst,
                                                                  const RigMainGrid*       eclGrid,
                                                                  const RigFemPart*        femPart,
                                                                  bool                     femIsDestination )
{
    {
        struct RangeFilterCorner
        {
            RangeFilterCorner()
                : cellMatchType( APPROX_ON_COLLAPSED )
            {
            }
            cvf::Vec3st   ijk;
            CellMatchType cellMatchType;
        };

        RangeFilterCorner rangeFilterMatches[8];

        cvf::Vec3st srcRangeCube[8];
        srcRangeCube[0] = cvf::Vec3st( src.StartI, src.StartJ, src.StartK );
        srcRangeCube[1] = cvf::Vec3st( src.EndI, src.StartJ, src.StartK );
        srcRangeCube[2] = cvf::Vec3st( src.EndI, src.EndJ, src.StartK );
        srcRangeCube[3] = cvf::Vec3st( src.StartI, src.EndJ, src.StartK );
        srcRangeCube[4] = cvf::Vec3st( src.StartI, src.StartJ, src.EndK );
        srcRangeCube[5] = cvf::Vec3st( src.EndI, src.StartJ, src.EndK );
        srcRangeCube[6] = cvf::Vec3st( src.EndI, src.EndJ, src.EndK );
        srcRangeCube[7] = cvf::Vec3st( src.StartI, src.EndJ, src.EndK );

        bool foundExactMatch = false;
        int  cornerIdx       = 0;
        int  diagIdx         = 6; // Index to diagonal corner

        for ( cornerIdx = 0; cornerIdx < 4; ++cornerIdx )
        {
            diagIdx = ( cornerIdx < 2 ) ? cornerIdx + 6 : cornerIdx + 2;

            if ( femIsDestination )
            {
                rangeFilterMatches[cornerIdx].cellMatchType =
                    findBestFemCellFromEclCell( eclGrid,
                                                srcRangeCube[cornerIdx][0],
                                                srcRangeCube[cornerIdx][1],
                                                srcRangeCube[cornerIdx][2],
                                                femPart,
                                                &( rangeFilterMatches[cornerIdx].ijk[0] ),
                                                &( rangeFilterMatches[cornerIdx].ijk[1] ),
                                                &( rangeFilterMatches[cornerIdx].ijk[2] ) );

                rangeFilterMatches[diagIdx].cellMatchType =
                    findBestFemCellFromEclCell( eclGrid,
                                                srcRangeCube[diagIdx][0],
                                                srcRangeCube[diagIdx][1],
                                                srcRangeCube[diagIdx][2],
                                                femPart,
                                                &( rangeFilterMatches[diagIdx].ijk[0] ),
                                                &( rangeFilterMatches[diagIdx].ijk[1] ),
                                                &( rangeFilterMatches[diagIdx].ijk[2] ) );
            }
            else
            {
                rangeFilterMatches[cornerIdx].cellMatchType =
                    findBestEclCellFromFemCell( femPart,
                                                srcRangeCube[cornerIdx][0],
                                                srcRangeCube[cornerIdx][1],
                                                srcRangeCube[cornerIdx][2],
                                                eclGrid,
                                                &( rangeFilterMatches[cornerIdx].ijk[0] ),
                                                &( rangeFilterMatches[cornerIdx].ijk[1] ),
                                                &( rangeFilterMatches[cornerIdx].ijk[2] ) );

                rangeFilterMatches[diagIdx].cellMatchType =
                    findBestEclCellFromFemCell( femPart,
                                                srcRangeCube[diagIdx][0],
                                                srcRangeCube[diagIdx][1],
                                                srcRangeCube[diagIdx][2],
                                                eclGrid,
                                                &( rangeFilterMatches[diagIdx].ijk[0] ),
                                                &( rangeFilterMatches[diagIdx].ijk[1] ),
                                                &( rangeFilterMatches[diagIdx].ijk[2] ) );
            }

            if ( rangeFilterMatches[cornerIdx].cellMatchType == EXACT && rangeFilterMatches[diagIdx].cellMatchType == EXACT )
            {
                foundExactMatch = true;
                break;
            }
        }

        // Get the start and end IJK from the matched corners
        if ( foundExactMatch )
        {
            // Populate dst range filter from the diagonal that matches exact
            dst.StartI = CVF_MIN( rangeFilterMatches[cornerIdx].ijk[0], rangeFilterMatches[diagIdx].ijk[0] );
            dst.StartJ = CVF_MIN( rangeFilterMatches[cornerIdx].ijk[1], rangeFilterMatches[diagIdx].ijk[1] );
            dst.StartK = CVF_MIN( rangeFilterMatches[cornerIdx].ijk[2], rangeFilterMatches[diagIdx].ijk[2] );
            dst.EndI   = CVF_MAX( rangeFilterMatches[cornerIdx].ijk[0], rangeFilterMatches[diagIdx].ijk[0] );
            dst.EndJ   = CVF_MAX( rangeFilterMatches[cornerIdx].ijk[1], rangeFilterMatches[diagIdx].ijk[1] );
            dst.EndK   = CVF_MAX( rangeFilterMatches[cornerIdx].ijk[2], rangeFilterMatches[diagIdx].ijk[2] );
        }
        else
        {
            // Look at the matches for each "face" of the range filter cube,
            // and use first exact match to determine the position of that "face"
            size_t faceIJKs[6] = {cvf::UNDEFINED_SIZE_T,
                                  cvf::UNDEFINED_SIZE_T,
                                  cvf::UNDEFINED_SIZE_T,
                                  cvf::UNDEFINED_SIZE_T,
                                  cvf::UNDEFINED_SIZE_T,
                                  cvf::UNDEFINED_SIZE_T};
            for ( int faceIdx = 0; faceIdx < 6; ++faceIdx )
            {
                auto gridAxis = cvf::StructGridInterface::gridAxisFromFace( cvf::StructGridInterface::FaceType( faceIdx ) );

                int ijOrk = 0;
                if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_I ) ijOrk = 0;
                if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_J ) ijOrk = 1;
                if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_K ) ijOrk = 2;

                cvf::ubyte surfCorners[4];
                cvf::StructGridInterface::cellFaceVertexIndices( (cvf::StructGridInterface::FaceType)faceIdx, surfCorners );
                bool foundAcceptedMatch = false;
                for ( int cIdx = 0; cIdx < 4; ++cIdx )
                {
                    if ( rangeFilterMatches[surfCorners[cIdx]].cellMatchType == EXACT )
                    {
                        foundAcceptedMatch = true;

                        faceIJKs[faceIdx] = rangeFilterMatches[surfCorners[cIdx]].ijk[ijOrk];
                        break;
                    }
                }

                if ( !foundAcceptedMatch )
                {
                    // Take first match that is not related to a collapsed eclipse cell
                    for ( int cIdx = 0; cIdx < 4; ++cIdx )
                    {
                        if ( rangeFilterMatches[surfCorners[cIdx]].cellMatchType == APPROX )
                        {
                            foundAcceptedMatch = true;

                            faceIJKs[faceIdx] = rangeFilterMatches[surfCorners[cIdx]].ijk[ijOrk];
                            break;
                        }
                    }

                    if ( !foundAcceptedMatch )
                    {
                        // Only collapsed cell hits in this "face"
                        // Todo: then use opposite face - range filter thickness
                        // For now, just select the first
                        faceIJKs[faceIdx] = rangeFilterMatches[surfCorners[0]].ijk[ijOrk];
                    }
                }
            }

#ifdef DEBUG
            for ( int faceIdx = 0; faceIdx < 6; ++faceIdx )
            {
                CVF_TIGHT_ASSERT( faceIJKs[faceIdx] != cvf::UNDEFINED_SIZE_T );
            }
#endif

            dst.EndI   = faceIJKs[cvf::StructGridInterface::POS_I];
            dst.StartI = faceIJKs[cvf::StructGridInterface::NEG_I];
            dst.EndJ   = faceIJKs[cvf::StructGridInterface::POS_J];
            dst.StartJ = faceIJKs[cvf::StructGridInterface::NEG_J];
            dst.EndK   = faceIJKs[cvf::StructGridInterface::POS_K];
            dst.StartK = faceIJKs[cvf::StructGridInterface::NEG_K];
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Return 0 for collapsed cell 1 for
//--------------------------------------------------------------------------------------------------
RigCaseToCaseRangeFilterMapper::CellMatchType
    RigCaseToCaseRangeFilterMapper::findBestFemCellFromEclCell( const RigMainGrid* masterEclGrid,
                                                                size_t             ei,
                                                                size_t             ej,
                                                                size_t             ek,
                                                                const RigFemPart*  dependentFemPart,
                                                                size_t*            fi,
                                                                size_t*            fj,
                                                                size_t*            fk )
{
    // Find tolerance

    double cellSizeI, cellSizeJ, cellSizeK;
    masterEclGrid->characteristicCellSizes( &cellSizeI, &cellSizeJ, &cellSizeK );

    double xyTolerance = cellSizeI * 0.01;
    double zTolerance  = cellSizeK * 0.01;

    bool isEclFaceNormalsOutwards = masterEclGrid->isFaceNormalsOutwards();

    size_t cellIdx = masterEclGrid->cellIndexFromIJK( ei, ej, ek );

    bool isCollapsedCell = masterEclGrid->globalCellArray()[cellIdx].isCollapsedCell();

    cvf::Vec3d geoMechConvertedEclCell[8];
    RigCaseToCaseCellMapperTools::estimatedFemCellFromEclCell( masterEclGrid, cellIdx, geoMechConvertedEclCell );

    cvf::BoundingBox elmBBox;
    for ( int i = 0; i < 8; ++i )
        elmBBox.add( geoMechConvertedEclCell[i] );

    std::vector<size_t> closeElements;
    dependentFemPart->findIntersectingCells( elmBBox, &closeElements );

    cvf::Vec3d elmCorners[8];
    int        elmIdxToBestMatch        = -1;
    double     sqDistToClosestElmCenter = HUGE_VAL;
    cvf::Vec3d convEclCellCenter        = RigCaseToCaseCellMapperTools::calculateCellCenter( geoMechConvertedEclCell );

    bool foundExactMatch = false;

    for ( size_t ccIdx = 0; ccIdx < closeElements.size(); ++ccIdx )
    {
        int elmIdx = static_cast<int>( closeElements[ccIdx] );

        RigCaseToCaseCellMapperTools::elementCorners( dependentFemPart, elmIdx, elmCorners );

        cvf::Vec3d cellCenter = RigCaseToCaseCellMapperTools::calculateCellCenter( elmCorners );
        double     sqDist     = ( cellCenter - convEclCellCenter ).lengthSquared();
        if ( sqDist < sqDistToClosestElmCenter )
        {
            elmIdxToBestMatch        = elmIdx;
            sqDistToClosestElmCenter = sqDist;
        }

        RigCaseToCaseCellMapperTools::rotateCellTopologicallyToMatchBaseCell( geoMechConvertedEclCell,
                                                                              isEclFaceNormalsOutwards,
                                                                              elmCorners );

        foundExactMatch = RigCaseToCaseCellMapperTools::isEclFemCellsMatching( geoMechConvertedEclCell,
                                                                               elmCorners,
                                                                               xyTolerance,
                                                                               zTolerance );

        if ( foundExactMatch )
        {
            elmIdxToBestMatch = elmIdx;
            break;
        }
    }

    if ( elmIdxToBestMatch != -1 )
    {
        bool validIndex = dependentFemPart->getOrCreateStructGrid()->ijkFromCellIndex( elmIdxToBestMatch, fi, fj, fk );
        CVF_ASSERT( validIndex );
    }
    else
    {
        ( *fi ) = cvf::UNDEFINED_SIZE_T;
        ( *fj ) = cvf::UNDEFINED_SIZE_T;
        ( *fk ) = cvf::UNDEFINED_SIZE_T;
    }

    if ( foundExactMatch ) return EXACT;
    if ( isCollapsedCell ) return APPROX_ON_COLLAPSED;
    return APPROX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseToCaseRangeFilterMapper::CellMatchType
    RigCaseToCaseRangeFilterMapper::findBestEclCellFromFemCell( const RigFemPart*  dependentFemPart,
                                                                size_t             fi,
                                                                size_t             fj,
                                                                size_t             fk,
                                                                const RigMainGrid* masterEclGrid,
                                                                size_t*            ei,
                                                                size_t*            ej,
                                                                size_t*            ek )
{
    // Find tolerance

    double cellSizeI, cellSizeJ, cellSizeK;
    masterEclGrid->characteristicCellSizes( &cellSizeI, &cellSizeJ, &cellSizeK );

    double xyTolerance = cellSizeI * 0.4;
    double zTolerance  = cellSizeK * 0.4;

    bool isEclFaceNormalsOutwards = masterEclGrid->isFaceNormalsOutwards();

    int elementIdx = static_cast<int>( dependentFemPart->getOrCreateStructGrid()->cellIndexFromIJK( fi, fj, fk ) );

    cvf::Vec3d elmCorners[8];
    RigCaseToCaseCellMapperTools::elementCorners( dependentFemPart, elementIdx, elmCorners );

    cvf::BoundingBox elmBBox;
    for ( int i = 0; i < 8; ++i )
        elmBBox.add( elmCorners[i] );

    std::vector<size_t> closeCells;
    masterEclGrid->findIntersectingCells( elmBBox,
                                          &closeCells ); // This might actually miss the exact one, but we have no other
                                                         // alternative yet.

    size_t     globCellIdxToBestMatch    = cvf::UNDEFINED_SIZE_T;
    double     sqDistToClosestCellCenter = HUGE_VAL;
    cvf::Vec3d elmCenter                 = RigCaseToCaseCellMapperTools::calculateCellCenter( elmCorners );

    bool       foundExactMatch = false;
    cvf::Vec3d rotatedElm[8];

    for ( size_t ccIdx = 0; ccIdx < closeCells.size(); ++ccIdx )
    {
        size_t     cellIdx = closeCells[ccIdx];
        cvf::Vec3d geoMechConvertedEclCell[8];
        RigCaseToCaseCellMapperTools::estimatedFemCellFromEclCell( masterEclGrid, cellIdx, geoMechConvertedEclCell );

        cvf::Vec3d cellCenter = RigCaseToCaseCellMapperTools::calculateCellCenter( geoMechConvertedEclCell );
        double     sqDist     = ( cellCenter - elmCenter ).lengthSquared();
        if ( sqDist < sqDistToClosestCellCenter )
        {
            globCellIdxToBestMatch    = cellIdx;
            sqDistToClosestCellCenter = sqDist;
        }

        rotatedElm[0] = elmCorners[0];
        rotatedElm[1] = elmCorners[1];
        rotatedElm[2] = elmCorners[2];
        rotatedElm[3] = elmCorners[3];
        rotatedElm[4] = elmCorners[4];
        rotatedElm[5] = elmCorners[5];
        rotatedElm[6] = elmCorners[6];
        rotatedElm[7] = elmCorners[7];

        RigCaseToCaseCellMapperTools::rotateCellTopologicallyToMatchBaseCell( geoMechConvertedEclCell,
                                                                              isEclFaceNormalsOutwards,
                                                                              rotatedElm );

        foundExactMatch = RigCaseToCaseCellMapperTools::isEclFemCellsMatching( geoMechConvertedEclCell,
                                                                               rotatedElm,
                                                                               xyTolerance,
                                                                               zTolerance );

        if ( foundExactMatch )
        {
            globCellIdxToBestMatch = cellIdx;
            break;
        }
    }

    bool isCollapsedCell = false;
    if ( globCellIdxToBestMatch != cvf::UNDEFINED_SIZE_T )
    {
        masterEclGrid->ijkFromCellIndex( globCellIdxToBestMatch, ei, ej, ek );
        isCollapsedCell = masterEclGrid->globalCellArray()[globCellIdxToBestMatch].isCollapsedCell();
    }
    else
    {
        ( *ei ) = cvf::UNDEFINED_SIZE_T;
        ( *ej ) = cvf::UNDEFINED_SIZE_T;
        ( *ek ) = cvf::UNDEFINED_SIZE_T;
    }

    if ( foundExactMatch ) return EXACT;
    if ( isCollapsedCell ) return APPROX_ON_COLLAPSED;
    return APPROX;
}
