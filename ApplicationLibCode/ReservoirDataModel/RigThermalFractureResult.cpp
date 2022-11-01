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

#include "RigThermalFractureResult.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigThermalFractureResult::RigThermalFractureResult( const QString& name, const QString& unit )
    : m_name( name )
    , m_unit( unit )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigThermalFractureResult::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigThermalFractureResult::unit() const
{
    return m_unit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigThermalFractureResult::appendValue( size_t nodeIndex, double value )
{
    if ( nodeIndex >= m_parameterValues.size() )
        m_parameterValues.push_back( { value } );
    else
        m_parameterValues[nodeIndex].push_back( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigThermalFractureResult::getValue( size_t nodeIndex, size_t timeStepIndex ) const
{
    return m_parameterValues[nodeIndex][timeStepIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigThermalFractureResult::numNodes() const
{
    return m_parameterValues.size();
}
