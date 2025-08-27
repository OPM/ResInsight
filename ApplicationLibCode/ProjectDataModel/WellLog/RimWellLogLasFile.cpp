/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogLasFile.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RiaDateStringParser.h"
#include "RiaFieldHandleTools.h"
#include "RiaQDateTimeTools.h"

#include "Well/RigWellLogLasFile.h"

#include "RimFileWellPath.h"
#include "RimTools.h"
#include "RimWellLogChannel.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include "Riu3DMainWindowTools.h"

#include <QFileInfo>
#include <QString>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RimWellLogLasFile, "WellLogLasFile", "WellLogFile" );

namespace caf
{
template <>
void caf::AppEnum<RimWellLogLasFile::WellFlowCondition>::setUp()
{
    addItem( RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR, "RESERVOIR", "Reservoir Volumes" );
    addItem( RimWellLogLasFile::WELL_FLOW_COND_STANDARD, "STANDARD", "Standard Volumes" );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogLasFile::RimWellLogLasFile()
{
    CAF_PDM_InitObject( "Well LAS File Info", ":/LasFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "" );
    m_wellName.uiCapability()->setUiReadOnly( true );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_wellName );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "" );
    m_name.uiCapability()->setUiReadOnly( true );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_name );

    m_date.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_wellFlowCondition,
                       "WellFlowCondition",
                       caf::AppEnum<RimWellLogLasFile::WellFlowCondition>( RimWellLogLasFile::WELL_FLOW_COND_STANDARD ),
                       "Well Flow Rates" );

    CAF_PDM_InitField( &m_invalidDateMessage, "InvalidDateMessage", QString( "Invalid or no date" ), "" );
    m_invalidDateMessage.uiCapability()->setUiReadOnly( true );
    m_invalidDateMessage.xmlCapability()->disableIO();

    m_wellLogDataFile     = nullptr;
    m_lasFileHasValidDate = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogLasFile* RimWellLogLasFile::readWellLogFile( const QString& logFilePath, QString* errorMessage )
{
    CAF_ASSERT( errorMessage );

    QFileInfo fi( logFilePath );

    RimWellLogLasFile* wellLogFile = nullptr;

    if ( fi.suffix().toUpper().compare( "LAS" ) == 0 || fi.suffix().toUpper().compare( "RMSWELL" ) == 0 )
    {
        wellLogFile = new RimWellLogLasFile();
        wellLogFile->setFileName( logFilePath );
        if ( !wellLogFile->readFile( errorMessage ) )
        {
            delete wellLogFile;
            wellLogFile = nullptr;
        }
    }

    return wellLogFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogLasFile::readFile( QString* errorMessage )
{
    if ( !m_wellLogDataFile.p() )
    {
        m_wellLogDataFile = new RigWellLogLasFile;
    }

    m_name = QFileInfo( m_fileName().path() ).fileName();

    if ( !m_wellLogDataFile->open( m_fileName().path(), errorMessage ) )
    {
        m_wellLogDataFile = nullptr;
        return false;
    }

    m_wellName = m_wellLogDataFile->wellName();

    QDateTime date        = RiaDateStringParser::parseDateString( m_wellLogDataFile->date() );
    m_lasFileHasValidDate = isDateValid( date );
    if ( m_lasFileHasValidDate )
    {
        m_date = date;
    }
    else if ( !isDateValid( m_date() ) )
    {
        RiaLogging::warning( QString( "The LAS-file '%1' contains no recognizable date. Please assign a date in the "
                                      "LAS-file property panel." )
                                 .arg( m_name() ) );

        m_date = DEFAULT_DATE_TIME;
    }

    updateChannelsFromWellLogData( m_wellLogDataFile.p() );

    auto fileWellPath = firstAncestorOrThisOfType<RimFileWellPath>();
    if ( fileWellPath )
    {
        if ( fileWellPath->filePath().isEmpty() ) // Has dummy wellpath
        {
            fileWellPath->setName( m_wellName );
        }
    }

    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
    {
        if ( wellPath->name().isEmpty() )
        {
            wellPath->setName( m_wellName );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogLasFile::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogLasFile::hasFlowData() const
{
    return RimWellPlotTools::hasFlowData( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RimWellLogLasFile::findMdAndChannelValuesForWellPath( const RimWellPath& wellPath,
                                                                                             const QString&     channelName,
                                                                                             QString*           unitString /*=nullptr*/ )
{
    std::vector<RimWellLogLasFile*> wellLogFiles = wellPath.descendantsIncludingThisOfType<RimWellLogLasFile>();
    for ( RimWellLogLasFile* wellLogFile : wellLogFiles )
    {
        RigWellLogLasFile*  fileData      = wellLogFile->wellLogData();
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
void RimWellLogLasFile::setupBeforeSave()
{
    m_wellFlowCondition.xmlCapability()->setIOWritable( hasFlowData() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogLasFile::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_fileName );
    uiOrdering.add( &m_date );
    m_date.uiCapability()->setUiReadOnly( m_lasFileHasValidDate );

    if ( !isDateValid( m_date() ) )
    {
        uiOrdering.add( &m_invalidDateMessage );
    }

    if ( hasFlowData() )
    {
        uiOrdering.add( &m_wellFlowCondition );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogLasFile::isDateValid( const QDateTime dateTime )
{
    return dateTime.isValid() && dateTime != DEFAULT_DATE_TIME;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogLasFile::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogLasFile::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogLasFile* RimWellLogLasFile::wellLogData()
{
    return m_wellLogDataFile.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogLasFile::WellFlowCondition RimWellLogLasFile::wellFlowRateCondition() const
{
    return m_wellFlowCondition();
}
