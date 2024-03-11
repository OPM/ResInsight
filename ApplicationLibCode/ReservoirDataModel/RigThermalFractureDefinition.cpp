/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include "RigThermalFractureDefinition.h"

#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"

#include "cafAssert.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigThermalFractureDefinition::RigThermalFractureDefinition()
    : m_unitSystem( RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigThermalFractureDefinition::~RigThermalFractureDefinition()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureDefinition::setName( const QString& name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigThermalFractureDefinition::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureDefinition::setUnitSystem( RiaDefines::EclipseUnitSystem unitSystem )
{
    m_unitSystem = unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RigThermalFractureDefinition::unitSystem() const
{
    return m_unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigThermalFractureDefinition::numNodes() const
{
    if ( m_results.empty() ) return 0u;

    return m_results[0].numNodes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigThermalFractureDefinition::timeSteps() const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureDefinition::addTimeStep( double timeStep )
{
    m_timeSteps.push_back( timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigThermalFractureDefinition::numTimeSteps() const
{
    return m_timeSteps.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureDefinition::addProperty( const QString& name, const QString& unit )
{
    m_results.push_back( RigThermalFractureResult( name, unit ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RigThermalFractureDefinition::getPropertyNamesUnits() const
{
    std::vector<std::pair<QString, QString>> namesAndUnits;
    for ( auto r : m_results )
        namesAndUnits.push_back( std::make_pair( r.name(), r.unit() ) );

    return namesAndUnits;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureDefinition::appendPropertyValue( int propertyIndex, int nodeIndex, double value )
{
    CAF_ASSERT( propertyIndex >= 0 );
    CAF_ASSERT( propertyIndex < static_cast<int>( m_results.size() ) );

    m_results[propertyIndex].appendValue( nodeIndex, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureDefinition::getPropertyValue( int propertyIndex, int nodeIndex, int timeStepIndex ) const
{
    return m_results[propertyIndex].getValue( nodeIndex, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigThermalFractureDefinition::getPropertyIndex( const QString& name ) const
{
    for ( size_t i = 0; i < m_results.size(); i++ )
        if ( name == m_results[i].name() ) return static_cast<int>( i );

    return -1;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigThermalFractureDefinition::relativeCoordinates( int timeStepIndex ) const
{
    std::vector<cvf::Vec3d> relCoords;

    int xIndex = getPropertyIndex( "XCoord" );
    int yIndex = getPropertyIndex( "YCoord" );
    int zIndex = getPropertyIndex( "ZCoord" );
    if ( xIndex == -1 || yIndex == -1 || zIndex == -1 )
    {
        return relCoords;
    }

    // The first node is the center node
    int        centerNodeIndex = 0;
    cvf::Vec3d centerNode( getPropertyValue( xIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( yIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( zIndex, centerNodeIndex, timeStepIndex ) );

    for ( size_t nodeIndex = 0; nodeIndex < numNodes(); nodeIndex++ )
    {
        cvf::Vec3d nodePos( getPropertyValue( xIndex, static_cast<int>( nodeIndex ), timeStepIndex ),
                            getPropertyValue( yIndex, static_cast<int>( nodeIndex ), timeStepIndex ),
                            getPropertyValue( zIndex, static_cast<int>( nodeIndex ), timeStepIndex ) );
        relCoords.push_back( nodePos - centerNode );
    }

    return relCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigThermalFractureDefinition::centerPosition() const
{
    int xIndex = getPropertyIndex( "XCoord" );
    int yIndex = getPropertyIndex( "YCoord" );
    int zIndex = getPropertyIndex( "ZCoord" );
    if ( xIndex == -1 || yIndex == -1 || zIndex == -1 )
    {
        return cvf::Vec3d::UNDEFINED;
    }

    // The first node is the center node
    int        centerNodeIndex = 0;
    int        timeStepIndex   = 0;
    cvf::Vec3d centerNode( getPropertyValue( xIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( yIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( zIndex, centerNodeIndex, timeStepIndex ) );
    return centerNode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigThermalFractureDefinition::getBoundingBox( int timeStepIndex ) const
{
    std::vector<cvf::Vec3d> coords;

    cvf::BoundingBox bb;

    int xIndex = getPropertyIndex( "XCoord" );
    int yIndex = getPropertyIndex( "YCoord" );
    int zIndex = getPropertyIndex( "ZCoord" );
    if ( xIndex == -1 || yIndex == -1 || zIndex == -1 )
    {
        return bb;
    }

    for ( size_t nodeIndex = 0; nodeIndex < numNodes(); nodeIndex++ )
    {
        cvf::Vec3d nodePos( getPropertyValue( xIndex, static_cast<int>( nodeIndex ), timeStepIndex ),
                            getPropertyValue( yIndex, static_cast<int>( nodeIndex ), timeStepIndex ),
                            getPropertyValue( zIndex, static_cast<int>( nodeIndex ), timeStepIndex ) );
        bb.add( nodePos );
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureDefinition::minDepth( int timeStepIndex ) const
{
    return getBoundingBox( timeStepIndex ).min().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureDefinition::maxDepth( int timeStepIndex ) const
{
    return getBoundingBox( timeStepIndex ).max().z();
}
