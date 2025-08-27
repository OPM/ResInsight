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
#include "RigWellPathIntersectionTools.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"

#include <array>
#include <map>

//==================================================================================================
///
//==================================================================================================

RigEclipseWellLogExtractor::RigEclipseWellLogExtractor( gsl::not_null<const RigEclipseCaseData*> aCase,
                                                        gsl::not_null<const RigWellPath*>        wellpath,
                                                        const std::string&                       wellCaseErrorMsgName )
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

        std::array<cvf::Vec3d, 8> hexCorners;
        for ( const auto& globalCellIndex : closeCellIndices )
        {
            const RigCell& cell = m_caseData->mainGrid()->cell( globalCellIndex );

            if ( cell.isInvalid() || cell.subGrid() != nullptr ) continue;

            m_caseData->mainGrid()->cellCornerVertices( globalCellIndex, hexCorners );

            RigHexIntersectionTools::lineHexCellIntersection( p1, p2, hexCorners.data(), globalCellIndex, &intersections );
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

            std::array<cvf::Vec3d, 8> hexCorners;
            for ( const auto& globalCellIndex : closeCellIndices )
            {
                const RigCell& cell = m_caseData->mainGrid()->cell( globalCellIndex );

                if ( cell.isInvalid() ) continue;

                m_caseData->mainGrid()->cellCornerVertices( globalCellIndex, hexCorners );

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
                        RiaLogging::debug( txt );
                    }
                }
            }
        }
    }

    populateReturnArrays( uniqueIntersections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::curveData( const RigResultAccessor* resultAccessor, std::vector<double>* values )
{
    CVF_TIGHT_ASSERT( values );
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
    std::array<cvf::Vec3d, 8> hexCorners;
    m_caseData->mainGrid()->cellCornerVertices( cellIndex, hexCorners );

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
