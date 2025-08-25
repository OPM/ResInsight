/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RigImportedWellLogData.h"

#include <QStringList>

#include <cmath>

const double RigImportedWellLogData::MISSING_VALUE = -9999.0;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigImportedWellLogData::RigImportedWellLogData()
    : RigWellLogData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigImportedWellLogData::~RigImportedWellLogData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RigImportedWellLogData::wellLogChannelNames() const
{
    QStringList channelNames;
    for ( const auto& channelPair : m_channelData )
    {
        channelNames << channelPair.first;
    }
    return channelNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigImportedWellLogData::depthValues() const
{
    return m_depthValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigImportedWellLogData::tvdMslValues() const
{
    // Not supported for imported data
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigImportedWellLogData::tvdRkbValues() const
{
    // Not supported for imported data
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigImportedWellLogData::values( const QString& name ) const
{
    auto it = m_channelData.find( name );
    if ( it != m_channelData.end() )
    {
        return it->second;
    }
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigImportedWellLogData::wellLogChannelUnitString( const QString& wellLogChannelName ) const
{
    // For imported data, assume dimensionless units
    Q_UNUSED( wellLogChannelName )
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigImportedWellLogData::depthUnitString() const
{
    return "M";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigImportedWellLogData::hasTvdMslChannel() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigImportedWellLogData::hasTvdRkbChannel() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigImportedWellLogData::getMissingValue() const
{
    return MISSING_VALUE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigImportedWellLogData::setChannelData( const QString& channelName, const std::vector<double>& values )
{
    m_channelData[channelName] = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigImportedWellLogData::setDepthValues( const std::vector<double>& depthValues )
{
    m_depthValues = depthValues;
}