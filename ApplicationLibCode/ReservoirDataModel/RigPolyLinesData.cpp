/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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
#include "RigPolyLinesData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigPolyLinesData::RigPolyLinesData()
    : m_showLines( true )
    , m_showSpheres( true )
    , m_lineThickness( 4 )
    , m_sphereRadiusFactor( 0.1 )
    , m_lockToZPlane( false )
    , m_lockedZValue( 0.0 )
    , m_closePolyline( true )
{
    m_sphereColor.set( 200, 200, 200 );
    m_lineColor.set( 200, 200, 200 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigPolyLinesData::~RigPolyLinesData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<cvf::Vec3d>>& RigPolyLinesData::polyLines() const
{
    return m_polylines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPolyLinesData::setPolyLines( const std::vector<std::vector<cvf::Vec3d>>& polyLines )
{
    m_polylines = polyLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPolyLinesData::setPolyLine( const std::vector<cvf::Vec3d>& polyline )
{
    m_polylines = { polyline };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPolyLinesData::addPolyLine( const std::vector<cvf::Vec3d>& polyline )
{
    m_polylines.push_back( polyline );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPolyLinesData::setVisibility( bool showLines, bool showSpheres )
{
    m_showLines   = showLines;
    m_showSpheres = showSpheres;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPolyLinesData::setLineAppearance( int lineThickness, cvf::Color3f color, bool closePolyline )
{
    m_lineThickness = lineThickness;
    m_lineColor     = color;
    m_closePolyline = closePolyline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPolyLinesData::setSphereAppearance( double radiusFactor, cvf::Color3f color )
{
    m_sphereRadiusFactor = radiusFactor;
    m_sphereColor        = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigPolyLinesData::showLines() const
{
    return m_showLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigPolyLinesData::showSpheres() const
{
    return m_showSpheres;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigPolyLinesData::lineThickness() const
{
    return m_lineThickness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RigPolyLinesData::lineColor() const
{
    return m_lineColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigPolyLinesData::closePolyline() const
{
    return m_closePolyline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RigPolyLinesData::sphereColor() const
{
    return m_sphereColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigPolyLinesData::sphereRadiusFactor() const
{
    return m_sphereRadiusFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigPolyLinesData::lockedZValue() const
{
    return m_lockedZValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigPolyLinesData::lockToZPlane() const
{
    return m_lockToZPlane;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPolyLinesData::setZPlaneLock( bool lockToZ, double lockZValue )
{
    m_lockToZPlane = lockToZ;
    m_lockedZValue = lockZValue;
}
