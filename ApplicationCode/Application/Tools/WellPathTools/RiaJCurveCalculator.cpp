/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaJCurveCalculator.h"
#include "RiaArcCurveCalculator.h"
#include "RiaOffshoreSphericalCoords.h"
#include "cvfMatrix3.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaJCurveCalculator::RiaJCurveCalculator( cvf::Vec3d p1, double azi1, double inc1, double r1, cvf::Vec3d p2 )
    : m_c1( cvf::Vec3d::UNDEFINED )
    , m_n1( cvf::Vec3d::UNDEFINED )
    , m_radius( std::numeric_limits<double>::infinity() )
    , m_curveStatus( OK )
{
    cvf::Vec3d t1( RiaOffshoreSphericalCoords::unitVectorFromAziInc( azi1, inc1 ) );

    cvf::Vec3d p1p2 = p2 - p1;

    cvf::Vec3d tr1       = p1p2 - ( p1p2.dot( t1 ) ) * t1;
    double     tr1Length = tr1.length();
    if ( tr1Length < 1e-9 )
    {
        // p2 is on the p1 + t12 line. Degenerates to a line.
        m_curveStatus      = OK_STRAIGHT_LINE;
        m_firstArcEndpoint = p2;
        m_endAzi           = azi1;
        m_endInc           = inc1;

        return;
    }

    tr1 /= tr1Length;

    cvf::Vec3d c1   = p1 + r1 * tr1;
    cvf::Vec3d p2c1 = c1 - p2;

    double p2c1Length = p2c1.length();
    if ( p2c1Length < r1 || r1 == std::numeric_limits<double>::infinity() )
    {
        // Radius is too big. We can not get to point 2 using the requested radius.
        m_curveStatus = FAILED_RADIUS_TOO_LARGE;

        RiaArcCurveCalculator arc( p1, t1, p2 );
        if ( arc.curveStatus() == RiaArcCurveCalculator::OK || arc.curveStatus() == RiaArcCurveCalculator::OK_STRAIGHT_LINE )
        {
            m_c1               = arc.center();
            m_n1               = arc.normal();
            m_firstArcEndpoint = p2;
            m_endAzi           = arc.endAzimuth();
            m_endInc           = arc.endInclination();
            m_radius           = arc.radius();
        }
        else
        {
            m_firstArcEndpoint = p2;
            m_endAzi           = azi1;
            m_endInc           = inc1;
        }
        return;
    }

    double d = sqrt( p2c1Length * p2c1Length - r1 * r1 );

    double     betha = asin( r1 / p2c1Length );
    cvf::Vec3d tp2c1 = p2c1 / p2c1Length;
    cvf::Vec3d nc1   = t1 ^ tr1;

    cvf::Vec3d tp11p2 = -tp2c1.getTransformedVector( cvf::Mat3d::fromRotation( nc1, betha ) );

    m_firstArcEndpoint = p2 - d * tp11p2;
    m_c1               = c1;
    m_n1               = nc1;

    RiaOffshoreSphericalCoords endTangent( tp11p2 );
    m_endAzi = endTangent.azi();
    m_endInc = endTangent.inc();
}
