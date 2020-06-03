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

#include "RigFishbonesGeometry.h"

#include "RimFishbonesMultipleSubs.h"

#include "RigWellPath.h"
#include "RimWellPath.h"
#include "cvfAssert.h"
#include "cvfMatrix4.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFisbonesGeometry::RigFisbonesGeometry( RimFishbonesMultipleSubs* fishbonesSub )
    : m_fishbonesSub( fishbonesSub )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<cvf::Vec3d, double>> RigFisbonesGeometry::coordsForLateral( size_t subIndex, size_t lateralIndex ) const
{
    CVF_ASSERT( lateralIndex < m_fishbonesSub->lateralLengths().size() );

    bool found = false;
    for ( auto& sub : m_fishbonesSub->installedLateralIndices() )
    {
        if ( sub.subIndex == subIndex )
        {
            auto it = std::find( sub.lateralIndices.begin(), sub.lateralIndices.end(), lateralIndex );
            if ( it != sub.lateralIndices.end() )
            {
                found = true;
                break;
            }
        }
    }
    CVF_ASSERT( found );

    cvf::Vec3d position;
    cvf::Vec3d lateralInitialDirection;
    cvf::Mat4d buildAngleRotationMatrix;

    computeLateralPositionAndOrientation( subIndex, lateralIndex, &position, &lateralInitialDirection, &buildAngleRotationMatrix );

    return computeCoordsAlongLateral( m_fishbonesSub->measuredDepth( subIndex ),
                                      m_fishbonesSub->lateralLengths()[lateralIndex],
                                      position,
                                      lateralInitialDirection,
                                      buildAngleRotationMatrix );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFisbonesGeometry::computeLateralPositionAndOrientation( size_t      subIndex,
                                                                size_t      lateralIndex,
                                                                cvf::Vec3d* startCoord,
                                                                cvf::Vec3d* startDirection,
                                                                cvf::Mat4d* buildAngleMatrix ) const
{
    RimWellPath* wellPath = nullptr;
    m_fishbonesSub->firstAncestorOrThisOfTypeAsserted( wellPath );

    RigWellPath* rigWellPath = wellPath->wellPathGeometry();
    CVF_ASSERT( rigWellPath );

    double measuredDepth = m_fishbonesSub->measuredDepth( subIndex );

    cvf::Vec3d position = rigWellPath->interpolatedPointAlongWellPath( measuredDepth );

    cvf::Mat4d buildAngleMat;
    cvf::Vec3d lateralDirection;

    {
        cvf::Vec3d lateralInitialDirection = cvf::Vec3d::Z_AXIS;
        cvf::Vec3d p1                      = cvf::Vec3d::UNDEFINED;
        cvf::Vec3d p2                      = cvf::Vec3d::UNDEFINED;
        rigWellPath->twoClosestPoints( position, &p1, &p2 );

        CVF_ASSERT( !p1.isUndefined() && !p2.isUndefined() );

        cvf::Vec3d alongWellPath = ( p2 - p1 ).getNormalized();

        if ( RigFisbonesGeometry::closestMainAxis( alongWellPath ) == cvf::Vec3d::Z_AXIS )
        {
            // Use Y-AXIS if well path is heading close to Z-AXIS
            lateralInitialDirection = cvf::Vec3d::Y_AXIS;
        }

        {
            double initialRotationAngle  = m_fishbonesSub->rotationAngle( subIndex );
            double lateralOffsetDegrees = 360.0 / m_fishbonesSub->lateralLengths().size();

            double lateralOffsetRadians = cvf::Math::toRadians( initialRotationAngle + lateralOffsetDegrees * lateralIndex );

            cvf::Mat4d lateralOffsetMatrix = cvf::Mat4d::fromRotation( alongWellPath, lateralOffsetRadians );

            lateralInitialDirection = lateralInitialDirection.getTransformedVector( lateralOffsetMatrix );
        }

        cvf::Vec3d rotationAxis;
        rotationAxis.cross( alongWellPath, lateralInitialDirection );

        double     exitAngleRadians      = cvf::Math::toRadians( m_fishbonesSub->exitAngle() );
        cvf::Mat4d lateralRotationMatrix = cvf::Mat4d::fromRotation( rotationAxis, exitAngleRadians );

        lateralDirection = alongWellPath.getTransformedVector( lateralRotationMatrix );

        double buildAngleRadians = cvf::Math::toRadians( m_fishbonesSub->buildAngle() );
        buildAngleMat            = cvf::Mat4d::fromRotation( rotationAxis, buildAngleRadians );
    }

    *startCoord       = position;
    *startDirection   = lateralDirection;
    *buildAngleMatrix = buildAngleMat;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<cvf::Vec3d, double>>
    RigFisbonesGeometry::computeCoordsAlongLateral( double            startMeasuredDepth,
                                                    double            lateralLength,
                                                    const cvf::Vec3d& startCoord,
                                                    const cvf::Vec3d& startDirection,
                                                    const cvf::Mat4d& buildAngleMatrix )
{
    std::vector<std::pair<cvf::Vec3d, double>> coords;

    cvf::Vec3d lateralDirection( startDirection );

    // Compute coordinates along the lateral by modifying the lateral direction by the build angle for
    // every unit vector along the lateral
    cvf::Vec3d accumulatedPosition = startCoord;
    double     measuredDepth       = startMeasuredDepth;

    double accumulatedLength = 0.0;
    while ( accumulatedLength < lateralLength )
    {
        coords.push_back( std::make_pair( accumulatedPosition, measuredDepth ) );

        double delta = 1.0;

        if ( lateralLength - accumulatedLength < 1.0 )
        {
            delta = lateralLength - accumulatedLength;
        }

        accumulatedPosition += delta * lateralDirection;

        // Modify the lateral direction by the build angle for each unit vector
        lateralDirection = lateralDirection.getTransformedVector( buildAngleMatrix );

        accumulatedLength += delta;
        measuredDepth += delta;
    }

    coords.push_back( std::make_pair( accumulatedPosition, measuredDepth ) );

    return coords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFisbonesGeometry::closestMainAxis( const cvf::Vec3d& vec )
{
    size_t maxComponent = 0;
    double maxValue     = cvf::Math::abs( vec.x() );
    if ( cvf::Math::abs( vec.y() ) > maxValue )
    {
        maxComponent = 1;
        maxValue     = cvf::Math::abs( vec.y() );
    }

    if ( cvf::Math::abs( vec.z() ) > maxValue )
    {
        maxComponent = 2;
    }

    if ( maxComponent == 0 )
    {
        return cvf::Vec3d::X_AXIS;
    }
    else if ( maxComponent == 1 )
    {
        return cvf::Vec3d::Y_AXIS;
    }
    else
    {
        return cvf::Vec3d::Z_AXIS;
    }
}
