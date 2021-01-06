/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaRftPltCurveDefinition.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaRftPltCurveDefinition::RiaRftPltCurveDefinition( const RifDataSourceForRftPlt& address,
                                                    const QString&                wellName,
                                                    const QDateTime&              timeStep )
    : m_curveAddress( address )
    , m_wellName( wellName )
    , m_timeStep( timeStep )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifDataSourceForRftPlt& RiaRftPltCurveDefinition::address() const
{
    return m_curveAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiaRftPltCurveDefinition::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QDateTime& RiaRftPltCurveDefinition::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaRftPltCurveDefinition::operator<( const RiaRftPltCurveDefinition& other ) const
{
    if ( m_curveAddress == other.m_curveAddress )
    {
        if ( m_wellName == other.m_wellName )
        {
            return m_timeStep < other.m_timeStep;
        }
        return m_wellName < other.m_wellName;
    }
    return m_curveAddress < other.m_curveAddress;
}
