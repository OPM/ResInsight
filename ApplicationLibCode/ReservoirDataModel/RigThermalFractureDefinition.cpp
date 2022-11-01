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
void RigThermalFractureDefinition::appendPropertyValue( size_t propertyIndex, size_t nodeIndex, double value )
{
    CAF_ASSERT( propertyIndex < m_results.size() );

    m_results[propertyIndex].appendValue( nodeIndex, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureDefinition::getPropertyValue( size_t propertyIndex, size_t nodeIndex, size_t timeStepIndex ) const
{
    CAF_ASSERT( propertyIndex < m_results.size() );

    return m_results[propertyIndex].getValue( nodeIndex, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigThermalFractureDefinition::getPropertyIndex( const QString& name ) const
{
    for ( size_t i = 0; i < m_results.size(); i++ )
        if ( name == m_results[i].name() ) return i;

    return std::numeric_limits<size_t>::max();
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigThermalFractureDefinition::relativeCoordinates( size_t timeStepIndex ) const
{
    std::vector<cvf::Vec3d> relCoords;

    auto xIndex = getPropertyIndex( "XCoord" );
    auto yIndex = getPropertyIndex( "YCoord" );
    auto zIndex = getPropertyIndex( "ZCoord" );
    if ( xIndex == std::numeric_limits<size_t>::max() || yIndex == std::numeric_limits<size_t>::max() ||
         zIndex == std::numeric_limits<size_t>::max() )
    {
        return relCoords;
    }

    // The first node is the center node
    size_t     centerNodeIndex = 0;
    cvf::Vec3d centerNode( getPropertyValue( xIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( yIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( zIndex, centerNodeIndex, timeStepIndex ) );

    for ( size_t nodeIndex = 0; nodeIndex < numNodes(); nodeIndex++ )
    {
        cvf::Vec3d nodePos( getPropertyValue( xIndex, nodeIndex, timeStepIndex ),
                            getPropertyValue( yIndex, nodeIndex, timeStepIndex ),
                            getPropertyValue( zIndex, nodeIndex, timeStepIndex ) );
        relCoords.push_back( nodePos - centerNode );
    }

    return relCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigThermalFractureDefinition::centerPosition() const
{
    auto xIndex = getPropertyIndex( "XCoord" );
    auto yIndex = getPropertyIndex( "YCoord" );
    auto zIndex = getPropertyIndex( "ZCoord" );
    if ( xIndex == std::numeric_limits<size_t>::max() || yIndex == std::numeric_limits<size_t>::max() ||
         zIndex == std::numeric_limits<size_t>::max() )
    {
        return cvf::Vec3d::UNDEFINED;
    }

    // The first node is the center node
    size_t     centerNodeIndex = 0;
    size_t     timeStepIndex   = 0;
    cvf::Vec3d centerNode( getPropertyValue( xIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( yIndex, centerNodeIndex, timeStepIndex ),
                           getPropertyValue( zIndex, centerNodeIndex, timeStepIndex ) );
    return centerNode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigThermalFractureDefinition::getBoundingBox( size_t timeStepIndex ) const
{
    std::vector<cvf::Vec3d> coords;

    cvf::BoundingBox bb;

    auto xIndex = getPropertyIndex( "XCoord" );
    auto yIndex = getPropertyIndex( "YCoord" );
    auto zIndex = getPropertyIndex( "ZCoord" );
    if ( xIndex == std::numeric_limits<size_t>::max() || yIndex == std::numeric_limits<size_t>::max() ||
         zIndex == std::numeric_limits<size_t>::max() )
    {
        return bb;
    }

    for ( size_t nodeIndex = 0; nodeIndex < numNodes(); nodeIndex++ )
    {
        cvf::Vec3d nodePos( getPropertyValue( xIndex, nodeIndex, timeStepIndex ),
                            getPropertyValue( yIndex, nodeIndex, timeStepIndex ),
                            getPropertyValue( zIndex, nodeIndex, timeStepIndex ) );
        bb.add( nodePos );
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureDefinition::minDepth( size_t timeStepIndex ) const
{
    return getBoundingBox( timeStepIndex ).min().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureDefinition::maxDepth( size_t timeStepIndex ) const
{
    return getBoundingBox( timeStepIndex ).max().z();
}
