/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 - Equinor ASA
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

#include "RigTracer.h"
#include "RigTracerPoint.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTracer::RigTracer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTracer::RigTracer( const RigTracer& other )
    : cvf::Object()
{
    for ( auto p : other.m_points )
    {
        appendPoint( p.position(), p.direction(), p.phaseType() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTracer::~RigTracer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTracer::appendPoint( cvf::Vec3d position, cvf::Vec3d direction, RiaDefines::PhaseType dominantPhase )
{
    m_points.push_back( RigTracerPoint( position, direction, dominantPhase ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigTracerPoint>& RigTracer::tracerPoints() const
{
    return m_points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigTracer::size() const
{
    return m_points.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigTracer::totalDistance() const
{
    if ( m_points.size() < 2 ) return 0.0;

    cvf::Vec3d sp = m_points.front().position();

    double distance = 0.0;
    for ( size_t i = 1; i < m_points.size(); i++ )
    {
        distance += sp.pointDistance( m_points[i].position() );
        sp = m_points[i].position();
    }

    return distance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTracer::reverse()
{
    if ( m_points.size() == 0 ) return;

    std::reverse( m_points.begin(), m_points.end() );

    for ( auto& p : m_points )
    {
        p.reverse();
    }
}
