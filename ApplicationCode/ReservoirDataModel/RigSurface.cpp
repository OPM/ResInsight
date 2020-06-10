/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RigSurface.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurface::RigSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurface::~RigSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<unsigned>& RigSurface::triangleIndices()
{
    return m_triangleIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigSurface::vertices()
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSurface::setTriangleData( const std::vector<unsigned>& tringleIndices, const std::vector<cvf::Vec3d>& vertices )
{
    m_triangleIndices = tringleIndices;
    m_vertices        = vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSurface::addVerticeResult( const QString resultName, const std::vector<float>& resultValues )
{
    m_verticeResults[resultName] = resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RigSurface::propertyValues( const QString& propertyName ) const
{
    auto it = m_verticeResults.find( propertyName );
    if ( it != m_verticeResults.end() )
    {
        return it->second;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigSurface::propertyNames() const
{
    std::vector<QString> names;

    for ( const auto& propertyResult : m_verticeResults )
    {
        names.push_back( propertyResult.first );
    }

    return names;
}
