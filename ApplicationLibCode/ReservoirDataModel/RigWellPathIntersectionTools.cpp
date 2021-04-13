/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RigWellPathIntersectionTools.h"

#include "RiaLogging.h"

#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellLogExtractionTools.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( const RigEclipseCaseData*      caseData,
                                                                      const QString&                 wellPathName,
                                                                      const std::vector<cvf::Vec3d>& pathCoords,
                                                                      const std::vector<double>&     pathMds )
{
    std::vector<WellPathCellIntersectionInfo> intersectionInfos;

    if ( pathCoords.size() < 2 ) return intersectionInfos;

    cvf::ref<RigWellPath> dummyWellPath = new RigWellPath( pathCoords, pathMds );

    std::string errorIdName = ( wellPathName + " " + caseData->ownerCase()->caseUserDescription() ).toStdString();

    cvf::ref<RigEclipseWellLogExtractor> extractor =
        new RigEclipseWellLogExtractor( caseData, dummyWellPath.p(), errorIdName );

    return extractor->cellIntersectionInfosAlongWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t>
    RigWellPathIntersectionTools::findIntersectedGlobalCellIndicesForWellPath( const RigEclipseCaseData* caseData,
                                                                               const RigWellPath*        wellPath )
{
    std::set<size_t> globalCellIndices;

    if ( caseData )
    {
        cvf::ref<RigEclipseWellLogExtractor> extractor =
            new RigEclipseWellLogExtractor( caseData, wellPath, caseData->ownerCase()->caseUserDescription().toStdString() );

        std::vector<WellPathCellIntersectionInfo> intersections = extractor->cellIntersectionInfosAlongWellPath();
        for ( const auto& intersection : intersections )
        {
            globalCellIndices.insert( intersection.globCellIndex );
        }
    }

    return globalCellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RigWellPathIntersectionTools::findIntersectedGlobalCellIndices( const RigEclipseCaseData* caseData,
                                                                                 const std::vector<cvf::Vec3d>& coords,
                                                                                 const std::vector<double>& measuredDepths )
{
    std::set<size_t> globalCellIndices;

    if ( caseData )
    {
        cvf::ref<RigWellPath> dummyWellPath;

        if ( measuredDepths.size() == coords.size() )
        {
            dummyWellPath = new RigWellPath( coords, measuredDepths );
        }
        else
        {
            RigSimulationWellCoordsAndMD helper( coords );

            dummyWellPath = new RigWellPath( helper.wellPathPoints(), helper.measuredDepths() );
        }

        globalCellIndices = findIntersectedGlobalCellIndicesForWellPath( caseData, dummyWellPath.p() );
    }

    return globalCellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPathIntersectionTools::calculateLengthInCell( const std::array<cvf::Vec3d, 8>& hexCorners,
                                                                const cvf::Vec3d&                startPoint,
                                                                const cvf::Vec3d&                endPoint )
{
    cvf::Vec3d vec = endPoint - startPoint;
    cvf::Vec3d iAxisDirection;
    cvf::Vec3d jAxisDirection;
    cvf::Vec3d kAxisDirection;

    RigCellGeometryTools::findCellLocalXYZ( hexCorners, iAxisDirection, jAxisDirection, kAxisDirection );

    cvf::Mat3d localCellCoordinateSystem( iAxisDirection.x(),
                                          jAxisDirection.x(),
                                          kAxisDirection.x(),
                                          iAxisDirection.y(),
                                          jAxisDirection.y(),
                                          kAxisDirection.y(),
                                          iAxisDirection.z(),
                                          jAxisDirection.z(),
                                          kAxisDirection.z() );

    auto signedVector = vec.getTransformedVector( localCellCoordinateSystem.getInverted() );

    return { std::fabs( signedVector.x() ), std::fabs( signedVector.y() ), std::fabs( signedVector.z() ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPathIntersectionTools::calculateLengthInCell( const RigMainGrid* grid,
                                                                size_t             cellIndex,
                                                                const cvf::Vec3d&  startPoint,
                                                                const cvf::Vec3d&  endPoint )
{
    std::array<cvf::Vec3d, 8> hexCorners;
    grid->cellCornerVertices( cellIndex, hexCorners.data() );

    return calculateLengthInCell( hexCorners, startPoint, endPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> RigWellPathIntersectionTools::buildContinuousIntersections(
    const std::vector<WellPathCellIntersectionInfo>& originalIntersections,
    const cvf::StructGridInterface*                  grid )
{
    std::vector<WellPathCellIntersectionInfo> intersectionsNoGap;

    if ( originalIntersections.empty() ) return intersectionsNoGap;

    for ( size_t i = 0; i < originalIntersections.size() - 1; i++ )
    {
        const WellPathCellIntersectionInfo& current = originalIntersections[i];
        const WellPathCellIntersectionInfo& next    = originalIntersections[i + 1];

        double distance           = std::fabs( current.endMD - next.startMD );
        double gapInGridThreshold = 0.1;
        if ( distance > gapInGridThreshold )
        {
            WellPathCellIntersectionInfo extraIntersection;

            bool showDebugInfo = false;
            if ( showDebugInfo )
            {
                QString ijkTextCurrent;
                {
                    size_t i = 0, j = 0, k = 0;
                    if ( grid )
                    {
                        grid->ijkFromCellIndex( current.globCellIndex, &i, &j, &k );
                    }
                    ijkTextCurrent = QString( "(%1 %2 %3)" ).arg( i + 1 ).arg( j + 1 ).arg( k + 1 );
                }
                QString ijkTextNext;
                {
                    size_t i = 0, j = 0, k = 0;
                    if ( grid )
                    {
                        grid->ijkFromCellIndex( next.globCellIndex, &i, &j, &k );
                    }
                    ijkTextNext = QString( "(%1 %2 %3)" ).arg( i + 1 ).arg( j + 1 ).arg( k + 1 );
                }

                QString text = QString( "Gap detected : Distance diff : %1, epsilon = %2\n Global Cell Index 1 : %3, "
                                        "IJK=%4, endMD : %5\n Global Cell Index 2 : %6, IJK=%7, startMD : %8" )
                                   .arg( distance )
                                   .arg( gapInGridThreshold )
                                   .arg( current.globCellIndex )
                                   .arg( ijkTextCurrent )
                                   .arg( current.endMD )
                                   .arg( next.globCellIndex )
                                   .arg( ijkTextNext )
                                   .arg( next.startMD );

                RiaLogging::info( text );
            }

            extraIntersection.globCellIndex = std::numeric_limits<size_t>::max();
            extraIntersection.startPoint    = current.endPoint;
            extraIntersection.endPoint      = next.startPoint;
            extraIntersection.startMD       = current.endMD;
            extraIntersection.endMD         = next.startMD;
            extraIntersection.intersectedCellFaceIn =
                cvf::StructGridInterface::oppositeFace( current.intersectedCellFaceOut );
            extraIntersection.intersectedCellFaceOut = cvf::StructGridInterface::oppositeFace( next.intersectedCellFaceIn );
            extraIntersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;

            intersectionsNoGap.push_back( extraIntersection );
        }

        intersectionsNoGap.push_back( current );
    }

    if ( !originalIntersections.empty() )
    {
        intersectionsNoGap.push_back( originalIntersections.back() );
    }

    return intersectionsNoGap;
}
