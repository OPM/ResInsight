/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigEclipseWellLogExtractor.h"

#include "RiaLogging.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigWellLogExtractionTools.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"
#include "RigWellPathIntersectionTools.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"

#include "cafAssert.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <map>
#include <tuple>

//==================================================================================================
///
//==================================================================================================

RigEclipseWellLogExtractor::RigEclipseWellLogExtractor( const RigEclipseCaseData* aCase,
                                                        const RigWellPath*        wellpath,
                                                        const std::string&        wellCaseErrorMsgName )
    : RigWellLogExtractor( wellpath, wellCaseErrorMsgName )
    , m_caseData( aCase )
{
    calculateIntersection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::calculateIntersection()
{
    std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo> uniqueIntersections;

    bool isCellFaceNormalsOut = m_caseData->mainGrid()->isFaceNormalsOutwards();

    if ( m_wellPathGeometry->wellPathPoints().empty() ) return;

    double tolerance = computeLengthThreshold();

    for ( size_t wpp = 0; wpp < m_wellPathGeometry->wellPathPoints().size() - 1; ++wpp )
    {
        std::vector<HexIntersectionInfo> intersections;
        cvf::Vec3d                       p1 = m_wellPathGeometry->wellPathPoints()[wpp];
        cvf::Vec3d                       p2 = m_wellPathGeometry->wellPathPoints()[wpp + 1];

        cvf::BoundingBox bb;

        bb.add( p1 );
        bb.add( p2 );

        std::vector<size_t> closeCellIndices = findCloseCellIndices( bb );

        for ( const auto& globalCellIndex : closeCellIndices )
        {
            const RigCell& cell = m_caseData->mainGrid()->cell( globalCellIndex );

            if ( cell.isInvalid() || cell.subGrid() != nullptr ) continue;

            std::array<cvf::Vec3d, 8> hexCorners = m_caseData->mainGrid()->cellCornerVertices( globalCellIndex );
            RigHexIntersectionTools::lineHexCellIntersection( p1, p2, hexCorners, globalCellIndex, &intersections );
        }

        if ( !isCellFaceNormalsOut )
        {
            for ( auto& intersection : intersections )
            {
                intersection.m_isIntersectionEntering = !intersection.m_isIntersectionEntering;
            }
        }

        // Now, with all the intersections of this piece of line, we need to
        // sort them in order, and set the measured depth and corresponding cell index

        // Inserting the intersections in this map will remove identical intersections
        // and sort them according to MD, CellIdx, Leave/enter

        double md1 = m_wellPathGeometry->measuredDepths()[wpp];
        double md2 = m_wellPathGeometry->measuredDepths()[wpp + 1];

        insertIntersectionsInMap( intersections, p1, md1, p2, md2, tolerance, &uniqueIntersections );
    }

    if ( uniqueIntersections.empty() && m_caseData->mainGrid()->isRadial() )
    {
        uniqueIntersections = radialGridAxisIntersections( tolerance );
    }

    if ( uniqueIntersections.empty() && m_wellPathGeometry->wellPathPoints().size() > 1 )
    {
        // When entering this function, all well path points are either completely outside the grid
        // or all well path points are inside one cell

        cvf::Vec3d firstPoint = m_wellPathGeometry->wellPathPoints().front();
        cvf::Vec3d lastPoint  = m_wellPathGeometry->wellPathPoints().back();

        {
            cvf::BoundingBox bb;
            bb.add( firstPoint );

            std::vector<size_t> closeCellIndices = findCloseCellIndices( bb );

            for ( const auto& globalCellIndex : closeCellIndices )
            {
                const RigCell& cell = m_caseData->mainGrid()->cell( globalCellIndex );

                if ( cell.isInvalid() ) continue;

                std::array<cvf::Vec3d, 8> hexCorners = m_caseData->mainGrid()->cellCornerVertices( globalCellIndex );
                if ( RigHexIntersectionTools::isPointInCell( firstPoint, hexCorners ) )
                {
                    if ( RigHexIntersectionTools::isPointInCell( lastPoint, hexCorners ) )
                    {
                        {
                            // Mark the first well path point as entering the cell

                            bool                      isEntering = true;
                            HexIntersectionInfo       info( firstPoint, isEntering, cvf::StructGridInterface::NO_FACE, globalCellIndex );
                            RigMDCellIdxEnterLeaveKey enterLeaveKey( m_wellPathGeometry->measuredDepths().front(),
                                                                     globalCellIndex,
                                                                     isEntering,
                                                                     tolerance );

                            uniqueIntersections.insert( std::make_pair( enterLeaveKey, info ) );
                        }

                        {
                            // Mark the last well path point as leaving cell

                            bool                      isEntering = false;
                            HexIntersectionInfo       info( lastPoint, isEntering, cvf::StructGridInterface::NO_FACE, globalCellIndex );
                            RigMDCellIdxEnterLeaveKey enterLeaveKey( m_wellPathGeometry->measuredDepths().back(),
                                                                     globalCellIndex,
                                                                     isEntering,
                                                                     tolerance );

                            uniqueIntersections.insert( std::make_pair( enterLeaveKey, info ) );
                        }
                    }
                    else
                    {
                        QString txt = "Detected two points assumed to be in the same cell, but they are in two different cells";
                        RiaLogging::debug( txt.toStdString() );
                    }
                }
            }
        }
    }

    populateReturnArrays( uniqueIntersections );
}

//--------------------------------------------------------------------------------------------------
/// A well in a radial grid is located on the grid axis, inside the inner radius or on a face between angular cells, and is not
/// intersected by any cell. Use the innermost cell column and place the cells along the well path by depth. Requires the innermost
/// ring to consist of a single 360 degree cell for every layer, as picking one of several angular sectors would be arbitrary.
//--------------------------------------------------------------------------------------------------
std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo> RigEclipseWellLogExtractor::radialGridAxisIntersections( double tolerance )
{
    std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo> uniqueIntersections;

    const RigMainGrid* mainGrid = m_caseData->mainGrid();

    // Only the innermost ring can safely represent the well's position when its cells span the full 360 degrees for every layer.
    // If the ring is split into several angular sectors, picking a single sector would be an arbitrary and potentially wrong choice.
    if ( mainGrid->cellCountJ() != 1 ) return uniqueIntersections;

    const std::vector<double>& wellPathMds  = m_wellPathGeometry->measuredDepths();
    const std::vector<double>  wellPathTvds = m_wellPathGeometry->trueVerticalDepths();
    if ( wellPathMds.size() < 2 ) return uniqueIntersections;

    const auto [minTvdIt, maxTvdIt] = std::minmax_element( wellPathTvds.begin(), wellPathTvds.end() );
    const double minTvd             = *minTvdIt;
    const double maxTvd             = *maxTvdIt;

    std::vector<std::tuple<double, double, size_t>> cellDepths;
    for ( size_t k = 0; k < mainGrid->cellCountK(); k++ )
    {
        const size_t cellIndex = mainGrid->cellIndexFromIJK( 0, 0, k );
        if ( mainGrid->cell( cellIndex ).isInvalid() ) continue;

        double topTvd    = std::numeric_limits<double>::infinity();
        double bottomTvd = -std::numeric_limits<double>::infinity();
        for ( const auto& corner : mainGrid->cellCornerVertices( cellIndex ) )
        {
            topTvd    = std::min( topTvd, -corner.z() );
            bottomTvd = std::max( bottomTvd, -corner.z() );
        }

        if ( bottomTvd <= minTvd || topTvd >= maxTvd ) continue;

        cellDepths.emplace_back( std::max( topTvd, minTvd ), std::min( bottomTvd, maxTvd ), cellIndex );
    }

    if ( cellDepths.empty() ) return uniqueIntersections;

    // MD interpolation requires unique TVD values ordered along the well path. K is not guaranteed to increase with depth, and
    // neighbour cells share their boundary TVD.
    std::vector<double> boundaryTvds;
    for ( const auto& [topTvd, bottomTvd, cellIndex] : cellDepths )
    {
        boundaryTvds.push_back( topTvd );
        boundaryTvds.push_back( bottomTvd );
    }
    std::sort( boundaryTvds.begin(), boundaryTvds.end() );
    boundaryTvds.erase( std::unique( boundaryTvds.begin(), boundaryTvds.end() ), boundaryTvds.end() );

    const std::vector<double> boundaryMds = RigWellPathGeometryTools::interpolateMdFromTvd( wellPathMds, wellPathTvds, boundaryTvds );
    if ( boundaryMds.size() != boundaryTvds.size() ) return uniqueIntersections;

    auto mdAtTvd = [&]( double tvd )
    {
        const auto it = std::lower_bound( boundaryTvds.begin(), boundaryTvds.end(), tvd );
        return boundaryMds[std::distance( boundaryTvds.begin(), it )];
    };

    for ( const auto& [topTvd, bottomTvd, cellIndex] : cellDepths )
    {
        for ( bool isEntering : { true, false } )
        {
            const double     md    = mdAtTvd( isEntering ? topTvd : bottomTvd );
            const cvf::Vec3d point = m_wellPathGeometry->interpolatedPointAlongWellPath( md );

            HexIntersectionInfo       info( point, isEntering, cvf::StructGridInterface::NO_FACE, cellIndex );
            RigMDCellIdxEnterLeaveKey enterLeaveKey( md, cellIndex, isEntering, tolerance );
            uniqueIntersections.insert( std::make_pair( enterLeaveKey, info ) );
        }
    }

    return uniqueIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::curveData( const RigResultAccessor* resultAccessor, std::vector<double>* values )
{
    CAF_ASSERT( values );
    values->resize( intersections().size() );

    for ( size_t cpIdx = 0; cpIdx < intersections().size(); ++cpIdx )
    {
        size_t                             cellIdx  = intersectedCellsGlobIdx()[cpIdx];
        cvf::StructGridInterface::FaceType cellFace = intersectedCellFaces()[cpIdx];
        ( *values )[cpIdx]                          = resultAccessor->cellFaceScalarGlobIdx( cellIdx, cellFace );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigEclipseWellLogExtractor::findCloseCellIndices( const cvf::BoundingBox& bb )
{
    return m_caseData->mainGrid()->findIntersectingCells( bb );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigEclipseWellLogExtractor::calculateLengthInCell( size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint ) const
{
    std::array<cvf::Vec3d, 8> hexCorners = m_caseData->mainGrid()->cellCornerVertices( cellIndex );

    return RigWellPathIntersectionTools::calculateLengthInCell( hexCorners, startPoint, endPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseWellLogExtractor::computeLengthThreshold() const
{
    // Default length tolerance for most common grid sizes
    double tolerance = 0.01;

    // For grids with very thin z-layers, reduce the tolerance to be able to find the intersections
    // If not, the intersection will be considered as non-valid cell edge intersection and discarded
    // https://github.com/OPM/ResInsight/issues/9244

    auto gridCellResult = const_cast<RigCaseCellResultsData*>( m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) );

    auto resultAdr = RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" );
    if ( gridCellResult && gridCellResult->hasResultEntry( resultAdr ) )
    {
        double averageDZ = 0.1;
        gridCellResult->meanCellScalarValues( resultAdr, averageDZ );

        const double scaleFactor = 0.05;
        tolerance                = std::min( tolerance, averageDZ * scaleFactor );
    }

    return tolerance;
}
