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

#include "RimImportedWellLogData.h"

#include "RimWellLogChannelData.h"

#include "Well/RigImportedWellLogData.h"

CAF_PDM_SOURCE_INIT( RimImportedWellLogData, "ImportedWellLogData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimImportedWellLogData::RimImportedWellLogData()
{
    CAF_PDM_InitObject( "Imported Well Log Data", "", "", "Imported Well Log Data" );

    CAF_PDM_InitField( &m_depthValues, "DepthValues", {}, "Depth Values" );
    CAF_PDM_InitField( &m_tvdMslValues, "TvdMslValues", {}, "TVD MSL Values" );
    CAF_PDM_InitField( &m_tvdRkbValues, "TvdRkbValues", {}, "TVD RKB Values" );
    CAF_PDM_InitFieldNoDefault( &m_channelDataObjects, "ChannelDataObjects", "Channel Data Objects" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLogData::setDepthValues( const std::vector<double>& depths )
{
    m_depthValues = depths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLogData::setTvdMslValues( const std::vector<double>& tvdMsl )
{
    m_tvdMslValues = tvdMsl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLogData::setTvdRkbValues( const std::vector<double>& tvdRkb )
{
    m_tvdRkbValues = tvdRkb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLogData::setChannelData( const QString& name, const std::vector<double>& values )
{
    // Check if channel already exists
    for ( RimWellLogChannelData* channelData : m_channelDataObjects )
    {
        if ( channelData->name() == name )
        {
            channelData->setValues( values );
            return;
        }
    }

    // Create new channel data object
    RimWellLogChannelData* channelData = new RimWellLogChannelData();
    channelData->setName( name );
    channelData->setValues( values );
    m_channelDataObjects.push_back( channelData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimImportedWellLogData::depthValues() const
{
    return m_depthValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimImportedWellLogData::tvdMslValues() const
{
    return m_tvdMslValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimImportedWellLogData::tvdRkbValues() const
{
    return m_tvdRkbValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimImportedWellLogData::channelNames() const
{
    std::vector<QString> names;
    for ( const RimWellLogChannelData* channelData : m_channelDataObjects )
    {
        names.push_back( channelData->name() );
    }
    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimImportedWellLogData::channelValues( const QString& name ) const
{
    for ( const RimWellLogChannelData* channelData : m_channelDataObjects )
    {
        if ( channelData->name() == name )
        {
            return channelData->values();
        }
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimImportedWellLogData::hasTvdMslValues() const
{
    return !m_tvdMslValues().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimImportedWellLogData::hasTvdRkbValues() const
{
    return !m_tvdRkbValues().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigImportedWellLogData> RimImportedWellLogData::createRigData() const
{
    cvf::ref<RigImportedWellLogData> rigData = new RigImportedWellLogData();

    // Set depth values
    rigData->setDepthValues( m_depthValues() );

    // Set TVD values if present
    if ( hasTvdMslValues() )
    {
        rigData->setTvdMslValues( m_tvdMslValues() );
    }
    if ( hasTvdRkbValues() )
    {
        rigData->setTvdRkbValues( m_tvdRkbValues() );
    }

    // Set channel data
    for ( const RimWellLogChannelData* channelData : m_channelDataObjects )
    {
        rigData->setChannelData( channelData->name(), channelData->values() );
    }

    return rigData;
}
