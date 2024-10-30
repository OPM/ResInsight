/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RigOsduWellLogData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigOsduWellLogData::RigOsduWellLogData()
    : RigWellLogData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigOsduWellLogData::~RigOsduWellLogData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RigOsduWellLogData::wellLogChannelNames() const
{
    QStringList channelNames;
    for ( const auto& channelPair : m_values )
    {
        channelNames << channelPair.first;
    }

    return channelNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigOsduWellLogData::depthValues() const
{
    return values( m_depthLogName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigOsduWellLogData::tvdMslValues() const
{
    return values( m_tvdMslLogName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigOsduWellLogData::tvdRkbValues() const
{
    // Not supported
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigOsduWellLogData::setValues( const QString& name, const std::vector<double>& values )
{
    m_values[name] = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigOsduWellLogData::values( const QString& name ) const
{
    if ( auto it = m_values.find( name ); it != m_values.end() ) return it->second;
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigOsduWellLogData::hasTvdMslChannel() const
{
    return !m_tvdMslLogName.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigOsduWellLogData::hasTvdRkbChannel() const
{
    return !m_tvdRkbLogName.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigOsduWellLogData::depthUnitString() const
{
    return wellLogChannelUnitString( m_tvdMslLogName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigOsduWellLogData::wellLogChannelUnitString( const QString& wellLogChannelName ) const
{
    auto unit = m_units.find( wellLogChannelName );
    if ( unit != m_units.end() )
        return unit->second;
    else
        return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigOsduWellLogData::getMissingValue() const
{
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigOsduWellLogData::finalizeData()
{
    for ( auto [columnName, values] : m_values )
    {
        if ( columnName.toUpper() == "TVDMSL" || columnName.toUpper().contains( "TVD" ) )
        {
            m_tvdMslLogName = columnName;
        }
        else if ( columnName.toUpper() == "DEPTH" )
        {
            m_depthLogName = "DEPTH";
        }
    }
}
