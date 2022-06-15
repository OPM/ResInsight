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
void RigThermalFractureDefinition::appendPropertyValue( int propertyIndex, int nodeIndex, int timeStepIndex, double value )
{
    CAF_ASSERT( propertyIndex >= 0 );
    CAF_ASSERT( propertyIndex < static_cast<int>( m_results.size() ) );

    m_results[propertyIndex].appendValue( nodeIndex, timeStepIndex, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureDefinition::getPropertyValue( int propertyIndex, int nodeIndex, int timeStepIndex ) const
{
    return m_results[propertyIndex].getValue( nodeIndex, timeStepIndex );
}
