/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Equinor ASA
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

#include "RigTriangleMeshData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTriangleMeshData::RigTriangleMeshData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTriangleMeshData::~RigTriangleMeshData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigTriangleMeshData::propertyNames() const
{
    return m_propertyNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> RigTriangleMeshData::geometry() const
{
    return std::make_pair( m_vertices, m_triangleIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RigTriangleMeshData::propertyValues( const QString& property ) const
{
    for ( size_t propertyIdx = 0; propertyIdx < m_propertyNames.size(); propertyIdx++ )
    {
        if ( m_propertyNames[propertyIdx] == property )
        {
            return m_propertyValues[propertyIdx];
        }
    }

    return std::vector<float>(); // return empty vector in case property was not found
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTriangleMeshData::setGeometryData( const std::vector<cvf::Vec3d>& nodeCoord, const std::vector<unsigned>& connectivities )
{
    m_vertices       = nodeCoord;
    m_triangleIndices = connectivities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTriangleMeshData::setPropertyData( const std::vector<QString>& propertyNames, std::vector<std::vector<float>>& propertyValues )
{
    m_propertyNames  = propertyNames;
    m_propertyValues = propertyValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTriangleMeshData::addPropertyData( const QString& propertyName, std::vector<float>& propertyValues )
{
    m_propertyNames.push_back( propertyName );
    m_propertyValues.push_back( propertyValues );
}
