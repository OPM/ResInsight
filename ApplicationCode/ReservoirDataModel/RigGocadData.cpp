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

#include "RigGocadData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGocadData::RigGocadData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGocadData::~RigGocadData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigGocadData::propertyNames()
{
    return m_propertyNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> RigGocadData::gocadGeometry()
{
    return std::make_pair( m_vertices, m_tringleIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RigGocadData::propertyValues( const QString& property )
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
void RigGocadData::setGeometryData( const std::vector<cvf::Vec3d>& nodeCoord, const std::vector<unsigned>& connectivities )
{
    m_vertices       = nodeCoord;
    m_tringleIndices = connectivities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGocadData::addPropertyData( const std::vector<QString>&      propertyNames,
                                    std::vector<std::vector<float>>& propertyValues )
{
    m_propertyNames  = propertyNames;
    m_propertyValues = propertyValues;
}
