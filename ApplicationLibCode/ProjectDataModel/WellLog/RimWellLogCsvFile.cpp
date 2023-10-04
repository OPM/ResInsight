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

// #include "RiaGuiApplication.h"
// #include "RiaLogging.h"

// #include "RiaDateStringParser.h"
#include "RiaFieldHandleTools.h"
// #include "RiaQDateTimeTools.h"

#include "RigWellLogCsvFile.h"

#include "RimFileWellPath.h"
#include "RimTools.h"
#include "RimWellLogFileChannel.h"
// #include "RimWellPathCollection.h"
// #include "RimWellPlotTools.h"

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
RimWellLogCsvFile::~RimWellLogCsvFile()
{
    m_wellLogChannelNames.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RimWellLogCsvFile* RimWellLogCsvFile::readWellLogFile( const QString& logFilePath, QString* errorMessage )
// {
//     CAF_ASSERT( errorMessage );

//     QFileInfo fi( logFilePath );

//     RimWellLogCsvFile* wellLogFile = nullptr;

//     // if ( fi.suffix().toUpper().compare( "LAS" ) == 0 )
//     // {
//     //     wellLogFile = new RimWellLogCsvFile();
//     //     wellLogFile->setFileName( logFilePath );
//     //     if ( !wellLogFile->readFile( errorMessage ) )
//     //     {
//     //         delete wellLogFile;
//     //         wellLogFile = nullptr;
//     //     }
//     // }

//     return wellLogFile;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCsvFile::readFile( QString* errorMessage )
{
    // if ( !m_wellLogDataFile.p() )
    // {
    //     m_wellLogDataFile = new RigWellLogCsvFile;
    // }

    // m_name = QFileInfo( m_fileName().path() ).fileName();

    // if ( !m_wellLogDataFile->open( m_fileName().path(), errorMessage ) )
    // {
    //     m_wellLogDataFile = nullptr;
    //     return false;
    // }

    // m_wellName = m_wellLogDataFile->wellName();

    // QDateTime date        = RiaDateStringParser::parseDateString( m_wellLogDataFile->date() );
    // m_lasFileHasValidDate = isDateValid( date );
    // if ( m_lasFileHasValidDate )
    // {
    //     m_date = date;
    // }
    // else if ( !isDateValid( m_date() ) )
    // {
    //     RiaLogging::warning( QString( "The LAS-file '%1' contains no recognizable date. Please assign a date in the "
    //                                   "LAS-file property panel." )
    //                              .arg( m_name() ) );

    //     m_date = DEFAULT_DATE_TIME;
    // }

    // m_wellLogChannelNames.deleteChildren();

    // QStringList wellLogNames = m_wellLogDataFile->wellLogChannelNames();
    // for ( int logIdx = 0; logIdx < wellLogNames.size(); logIdx++ )
    // {
    //     RimWellLogFileChannel* wellLog = new RimWellLogFileChannel();
    //     wellLog->setName( wellLogNames[logIdx] );
    //     m_wellLogChannelNames.push_back( wellLog );
    // }

    // auto wellPath = firstAncestorOrThisOfType<RimFileWellPath>();
    // if ( wellPath )
    // {
    //     if ( wellPath->filePath().isEmpty() ) // Has dummy wellpath
    //     {
    //         wellPath->setName( m_wellName );
    //     }
    // }

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
        RigWellLogCsvFile*  fileData      = wellLogFile->wellLogFileData();
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
RigWellLogCsvFile* RimWellLogCsvFile::wellLogFileData()
{
    return m_wellLogDataFile.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimWellLogCsvFile::setupBeforeSave()
// {
//     m_wellFlowCondition.xmlCapability()->setIOWritable( hasFlowData() );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimWellLogCsvFile::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
// {
//     uiOrdering.add( &m_fileName );
//     uiOrdering.add( &m_date );
//     m_date.uiCapability()->setUiReadOnly( m_lasFileHasValidDate );

//     if ( !isDateValid( m_date() ) )
//     {
//         uiOrdering.add( &m_invalidDateMessage );
//     }

//     if ( hasFlowData() )
//     {
//         uiOrdering.add( &m_wellFlowCondition );
//     }

//     uiOrdering.skipRemainingFields( true );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimWellLogCsvFile::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
// {
//     if ( changedField == &m_date )
//     {
//         // Due to a possible bug in QDateEdit/PdmUiDateEditor, convert m_date to a QDateTime having UTC timespec
//         m_date = RiaQDateTimeTools::createUtcDateTime( m_date().date(), m_date().time() );
//     }
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimWellLogCsvFile::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute*
// attribute )
// {
//     caf::PdmUiDateEditorAttribute* attrib = dynamic_cast<caf::PdmUiDateEditorAttribute*>( attribute );
//     if ( attrib != nullptr )
//     {
//         attrib->dateFormat = RiaQDateTimeTools::dateFormatString();
//     }
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// bool RimWellLogCsvFile::isDateValid( const QDateTime dateTime )
// {
//     return dateTime.isValid() && dateTime != DEFAULT_DATE_TIME;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellLogCsvFile::date() const
{
    return QDateTime();
}
