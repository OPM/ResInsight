/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Ceetron Solutions AS
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
#include "RigWbsParameter.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter::RigWbsParameter( const QString&                                                  name,
                                  const std::vector<std::pair<RigWbsParameter::Source, QString>>& sources )
    : m_name( name )
    , m_currentSource( INVALID )
    , m_validSources( sources )
{
    if ( !m_validSources.empty() )
    {
        m_currentSource = m_validSources.front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWbsParameter::addSource( Source source, const QString& entryName )
{
    if ( m_currentSource == INVALID )
    {
        m_currentSource = source;
    }
    m_validSources.push_back( std::make_pair( source, entryName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter::Source RigWbsParameter::currentSource() const
{
    return m_currentSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWbsParameter::setCurrentSource( Source source ) {}
