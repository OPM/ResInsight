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

#include "RigStimPlanModelTools.h"

#include "RiaLogging.h"

#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigMainGrid.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellLogExtractor.h"
#include "RigWellPathIntersectionTools.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"
#include "cvfPlane.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigStimPlanModelTools::calculateTSTDirection( RigEclipseCaseData* eclipseCaseData,
                                                         const cvf::Vec3d&   anchorPosition,
                                                         double              boundingBoxHorizontal,
                                                         double              boundingBoxVertical )
{
    cvf::Vec3d defaultDirection = cvf::Vec3d( 0.0, 0.0, -1.0 );

    if ( !eclipseCaseData ) return defaultDirection;

    RigMainGrid* mainGrid = eclipseCaseData->mainGrid();
    if ( !mainGrid ) return defaultDirection;

    cvf::Vec3d boundingBoxSize( boundingBoxHorizontal, boundingBoxHorizontal, boundingBoxVertical );

    // Find upper face of cells close to the anchor point
    cvf::BoundingBox    boundingBox( anchorPosition - boundingBoxSize, anchorPosition + boundingBoxSize );
    std::vector<size_t> closeCells;
    mainGrid->findIntersectingCells( boundingBox, &closeCells );

    // The stratigraphic thickness is the averge of normals of the top face
    cvf::Vec3d direction = cvf::Vec3d::ZERO;

    int numContributingCells = 0;
    for ( size_t globalCellIndex : closeCells )
    {
        const RigCell& cell = mainGrid->globalCellArray()[globalCellIndex];

        if ( !cell.isInvalid() )
        {
            direction += cell.faceNormalWithAreaLength( cvf::StructGridInterface::NEG_K ).getNormalized();
            numContributingCells++;
        }
    }

    RiaLogging::info( QString( "TST contributing cells: %1/%2" ).arg( numContributingCells ).arg( closeCells.size() ) );
    if ( numContributingCells == 0 )
    {
        // No valid close cells found: just point straight up
        return defaultDirection;
    }

    direction = ( direction / static_cast<double>( numContributingCells ) ).getNormalized();

    // A surface has normals in both directions: if the normal points upwards we flip it to
    // make it point downwards. This necessary when finding the TST start and end points later.
    if ( direction.z() > 0.0 )
    {
        direction *= -1.0;
    }

    // Calculate an adjusted TST direction to improve the zone thickness in the well log plot.
    // Using average of TST and TVD (default direction) in 3D.
    direction = ( direction + defaultDirection ) / 2.0;

    return direction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStimPlanModelTools::calculateFormationDip( const cvf::Vec3d& direction )
{
    // Formation dip is inclination of a plane from horizontal.
    return cvf::Math::toDegrees( cvf::GeometryTools::getAngle( direction, -cvf::Vec3d::Z_AXIS ) );
}

std::tuple<const RigFault*, double, cvf::Vec3d, double>
    RigStimPlanModelTools::findClosestFaultBarrier( RigEclipseCaseData* eclipseCaseData,
                                                    const cvf::Vec3d&   position,
                                                    const cvf::Vec3d&   directionToBarrier )
{
    std::vector<WellPathCellIntersectionInfo> intersections =
        RigStimPlanModelTools::generateBarrierIntersections( eclipseCaseData, position, directionToBarrier );

    RiaLogging::info( QString( "Intersections: %1" ).arg( intersections.size() ) );

    double shortestDistance = std::numeric_limits<double>::max();

    RigMainGrid*    mainGrid = eclipseCaseData->mainGrid();
    cvf::Vec3d      barrierPosition;
    double          barrierDip = 0.0;
    const RigFault* foundFault = nullptr;
    for ( const WellPathCellIntersectionInfo& intersection : intersections )
    {
        // Find the closest cell face which is a fault
        double          distance = position.pointDistance( intersection.startPoint );
        const RigFault* fault    = mainGrid->findFaultFromCellIndexAndCellFace( intersection.globCellIndex,
                                                                             intersection.intersectedCellFaceIn );
        if ( fault && distance < shortestDistance )
        {
            foundFault       = fault;
            shortestDistance = distance;
            barrierPosition  = intersection.startPoint;

            const RigCell& cell       = mainGrid->globalCellArray()[intersection.globCellIndex];
            cvf::Vec3d     faceNormal = cell.faceNormalWithAreaLength( intersection.intersectedCellFaceIn );
            barrierDip                = RigStimPlanModelTools::calculateFormationDip( faceNormal );
        }
    }

    return std::make_tuple( foundFault, shortestDistance, barrierPosition, barrierDip );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RigStimPlanModelTools::generateBarrierIntersections( RigEclipseCaseData* eclipseCaseData,
                                                         const cvf::Vec3d&   position,
                                                         const cvf::Vec3d&   directionToBarrier )
{
    double                                    randoDistance    = 10000.0;
    cvf::Vec3d                                forwardPosition  = position + ( directionToBarrier * randoDistance );
    cvf::Vec3d                                backwardPosition = position + ( directionToBarrier * -randoDistance );
    std::vector<WellPathCellIntersectionInfo> intersections =
        generateBarrierIntersectionsBetweenPoints( eclipseCaseData, position, forwardPosition );
    std::vector<WellPathCellIntersectionInfo> backwardIntersections =
        generateBarrierIntersectionsBetweenPoints( eclipseCaseData, position, backwardPosition );

    // Merge the intersections for the search for closest
    intersections.insert( intersections.end(), backwardIntersections.begin(), backwardIntersections.end() );
    return intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RigStimPlanModelTools::generateBarrierIntersectionsBetweenPoints( RigEclipseCaseData* eclipseCaseData,
                                                                      const cvf::Vec3d&   startPosition,
                                                                      const cvf::Vec3d&   endPosition )
{
    // Create a fake well path from the anchor point to
    // a point far away in the direction barrier direction
    std::vector<cvf::Vec3d> pathCoords;
    pathCoords.push_back( startPosition );
    pathCoords.push_back( endPosition );

    RigSimulationWellCoordsAndMD helper( pathCoords );
    return RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCaseData,
                                                                             "",
                                                                             helper.wellPathPoints(),
                                                                             helper.measuredDepths() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigStimPlanModelTools::findThicknessTargetPoints( RigEclipseCaseData* eclipseCaseData,
                                                       const cvf::Vec3d&   position,
                                                       const cvf::Vec3d&   direction,
                                                       double              extractionDepthTop,
                                                       double              extractionDepthBottom,
                                                       cvf::Vec3d&         topPosition,
                                                       cvf::Vec3d&         bottomPosition )
{
    if ( !eclipseCaseData ) return false;

    RiaLogging::info( QString( "Position:  %1" ).arg( RigStimPlanModelTools::vecToString( position ) ) );
    RiaLogging::info( QString( "Direction: %1" ).arg( RigStimPlanModelTools::vecToString( direction ) ) );

    // Create a "fake" well path which from top to bottom of formation
    // passing through the point and with the given direction

    const cvf::BoundingBox& allCellsBoundingBox = eclipseCaseData->mainGrid()->boundingBox();

    RiaLogging::info( QString( "All cells bounding box: %1 %2" )
                          .arg( RigStimPlanModelTools::vecToString( allCellsBoundingBox.min() ) )
                          .arg( RigStimPlanModelTools::vecToString( allCellsBoundingBox.max() ) ) );
    cvf::BoundingBox geometryBoundingBox( allCellsBoundingBox );

    // Use smaller depth bounding box for extraction if configured
    if ( extractionDepthTop > 0.0 && extractionDepthBottom > 0.0 && extractionDepthTop < extractionDepthBottom )
    {
        cvf::Vec3d bbMin( allCellsBoundingBox.min().x(), allCellsBoundingBox.min().y(), -extractionDepthBottom );
        cvf::Vec3d bbMax( allCellsBoundingBox.max().x(), allCellsBoundingBox.max().y(), -extractionDepthTop );
        geometryBoundingBox = cvf::BoundingBox( bbMin, bbMax );
    }

    if ( !geometryBoundingBox.contains( position ) )
    {
        //
        RiaLogging::error( "Anchor position is outside the grid bounding box. Unable to compute direction." );
        return false;
    }

    // Create plane on top and bottom of formation
    cvf::Plane topPlane;
    topPlane.setFromPointAndNormal( geometryBoundingBox.max(), cvf::Vec3d::Z_AXIS );

    cvf::Plane bottomPlane;
    bottomPlane.setFromPointAndNormal( geometryBoundingBox.min(), cvf::Vec3d::Z_AXIS );

    // Find and add point on top plane
    cvf::Vec3d abovePlane = position + ( direction * -10000.0 );
    if ( !topPlane.intersect( position, abovePlane, &topPosition ) )
    {
        RiaLogging::error( "Unable to compute top position of thickness direction vector." );
        return false;
    }

    RiaLogging::info( QString( "Top: %1" ).arg( RigStimPlanModelTools::vecToString( topPosition ) ) );

    // Find and add point on bottom plane
    cvf::Vec3d belowPlane = position + ( direction * 10000.0 );
    if ( !bottomPlane.intersect( position, belowPlane, &bottomPosition ) )
    {
        RiaLogging::error( "Unable to compute bottom position of thickness direction vector." );
        return false;
    }

    RiaLogging::info( QString( "Bottom: %1" ).arg( RigStimPlanModelTools::vecToString( bottomPosition ) ) );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigStimPlanModelTools::vecToString( const cvf::Vec3d& vec )
{
    return QString( "[%1, %2, %3]" ).arg( vec.x() ).arg( vec.y() ).arg( vec.z() );
}
