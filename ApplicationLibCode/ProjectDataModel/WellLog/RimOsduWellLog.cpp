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

#include "RimOsduWellLog.h"

#include "RiaFieldHandleTools.h"

#include "RimOsduWellLogChannel.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include <QString>

CAF_PDM_SOURCE_INIT( RimOsduWellLog, "OsduWellLog" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOsduWellLog::RimOsduWellLog()
{
    CAF_PDM_InitObject( "OSDU Well Log", ":/LasFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "" );
    m_name.uiCapability()->setUiReadOnly( true );

    m_date.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogId, "WellLogId", "Well Log Id" );
    m_wellLogId.uiCapability()->setUiReadOnly( true );

    // Need to save channels for Osdu well logs.
    // This reverts settings from RimWellLog constructor.
    m_wellLogChannels.xmlCapability()->setIOReadable( true );
    m_wellLogChannels.xmlCapability()->setIOWritable( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOsduWellLog::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOsduWellLog::hasFlowData() const
{
    return RimWellPlotTools::hasFlowData( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>>
    RimOsduWellLog::findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString )
{
    std::vector<RimOsduWellLog*> wellLogFiles = wellPath.descendantsIncludingThisOfType<RimOsduWellLog>();
    for ( RimOsduWellLog* wellLogFile : wellLogFiles )
    {
        RigOsduWellLogData* fileData      = wellLogFile->wellLogData();
        std::vector<double> channelValues = fileData->values( channelName );
        if ( !channelValues.empty() )
        {
            if ( unitString )
            {
                *unitString = fileData->wellLogChannelUnitString( channelName );
            }
            std::vector<double> depthValues = fileData->depthValues();
            CVF_ASSERT( depthValues.size() == channelValues.size() );
            std::vector<std::pair<double, double>> depthValuePairs;
            for ( size_t i = 0; i < depthValues.size(); ++i )
            {
                depthValuePairs.push_back( std::make_pair( depthValues[i], channelValues[i] ) );
            }
            return depthValuePairs;
        }
    }
    return std::vector<std::pair<double, double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOsduWellLog::isDateValid( const QDateTime dateTime )
{
    return dateTime.isValid() && dateTime != DEFAULT_DATE_TIME;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimOsduWellLog::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLog::setName( const QString& name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOsduWellLog::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigOsduWellLogData* RimOsduWellLog::wellLogData()
{
    return m_wellLogData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLog::setWellLogData( RigOsduWellLogData* wellLogData )
{
    m_wellLogData = wellLogData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLog::setWellLogId( const QString& wellLogId )
{
    m_wellLogId = wellLogId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOsduWellLog::wellLogId() const
{
    return m_wellLogId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLog::addWellLogChannel( RimOsduWellLogChannel* channel )
{
    m_wellLogChannels.push_back( channel );
}
