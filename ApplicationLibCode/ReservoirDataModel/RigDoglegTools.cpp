/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RigDoglegTools.h"

#include <algorithm>
#include <cmath>

#include "cvfMath.h"

// Using the spherical trigonometry formula (most accurate)
double RigDoglegTools::calculateDoglegAngle( const WellSurveyPoint& p1, const WellSurveyPoint& p2 )
{
    double inc1 = cvf::Math::toRadians( p1.inclination );
    double inc2 = cvf::Math::toRadians( p2.inclination );
    double azi1 = cvf::Math::toRadians( p1.azimuth );
    double azi2 = cvf::Math::toRadians( p2.azimuth );

    // Dogleg angle using spherical trigonometry (clamp to avoid numerical instability)
    double cosDogleg = std::clamp( std::cos( inc2 - inc1 ) - std::sin( inc1 ) * std::sin( inc2 ) * ( 1 - std::cos( azi2 - azi1 ) ), -1.0, 1.0 );
    return cvf::Math::toDegrees( std::acos( cosDogleg ) );
}

// Calculate dogleg severity (degrees per 100 feet or per 30 meters)
double RigDoglegTools::calculateDoglegSeverity( const WellSurveyPoint& p1, const WellSurveyPoint& p2, double normalizationDistance )
{
    double courseLength = std::abs( p2.measuredDepth - p1.measuredDepth );
    if ( courseLength == 0.0 ) return 0.0;

    double doglegAngle = calculateDoglegAngle( p1, p2 );
    return ( doglegAngle * normalizationDistance ) / courseLength;
}

// Process entire wellbore trajectory
std::vector<double> RigDoglegTools::calculateTrajectoryDogleg( const std::vector<WellSurveyPoint>& trajectory, double normalizationDistance )
{
    if ( trajectory.size() < 2 ) return {};

    // First point has no dogleg (no previous point to compare)
    std::vector<double> dogleValues;
    dogleValues.push_back( 0.0 );

    // Calculate dogleg for each subsequent point
    for ( size_t i = 1; i < trajectory.size(); ++i )
    {
        double severity = calculateDoglegSeverity( trajectory[i - 1], trajectory[i], normalizationDistance );
        dogleValues.push_back( severity );
    }

    return dogleValues;
}
