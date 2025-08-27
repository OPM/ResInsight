/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimWellLogCsvFile.h"

#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"

#include "Well/RigWellLogCsvFile.h"

#include "RimFileWellPath.h"
#include "RimTools.h"
#include "RimWellLogChannel.h"

#include <QFileInfo>
#include <QString>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RimWellLogCsvFile, "WellLogCsvFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCsvFile::RimWellLogCsvFile()
{
    CAF_PDM_InitObject( "Well CSV File Info", ":/LasFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "" );
    m_wellName.uiCapability()->setUiReadOnly( true );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_wellName );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "" );
    m_name.uiCapability()->setUiReadOnly( true );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_name );

    m_wellLogDataFile = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCsvFile::readFile( QString* errorMessage )
{
    if ( !m_wellLogDataFile.p() )
    {
        m_wellLogDataFile = new RigWellLogCsvFile;
    }

    m_name = QFileInfo( m_fileName().path() ).fileName();

    auto wellPath = firstAncestorOrThisOfType<RimFileWellPath>();
    if ( !wellPath )
    {
        RiaLogging::error( "No well path found" );
        return false;
    }

    if ( !m_wellLogDataFile->open( m_fileName().path(), wellPath->wellPathGeometry(), errorMessage ) )
    {
        m_wellLogDataFile = nullptr;
        RiaLogging::error( "Failed to open file." );

        return false;
    }

    updateChannelsFromWellLogData( m_wellLogDataFile.p() );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCsvFile::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RimWellLogCsvFile::findMdAndChannelValuesForWellPath( const RimWellPath& wellPath,
                                                                                             const QString&     channelName,
                                                                                             QString*           unitString /*=nullptr*/ )
{
    std::vector<RimWellLogCsvFile*> wellLogFiles = wellPath.descendantsIncludingThisOfType<RimWellLogCsvFile>();
    for ( RimWellLogCsvFile* wellLogFile : wellLogFiles )
    {
        RigWellLogCsvFile* fileData = wellLogFile->wellLogData();
        if ( fileData )
        {
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
    }
    return std::vector<std::pair<double, double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogCsvFile* RimWellLogCsvFile::wellLogData()
{
    return m_wellLogDataFile.p();
}
