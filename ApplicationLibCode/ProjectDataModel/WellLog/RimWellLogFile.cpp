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

#include "RimWellLogFile.h"

#include "RimFileWellPath.h"
#include "RimTools.h"
#include "RimWellLogFileChannel.h"

#include "RiaFieldHandleTools.h"
#include "RiaQDateTimeTools.h"

#include "cafPdmUiDateEditor.h"

#include <QString>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimWellLogFile, "WellLogFileInterface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QDateTime RimWellLogFile::DEFAULT_DATE_TIME = RiaQDateTimeTools::createUtcDateTime( QDate( 1900, 1, 1 ) );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFile::RimWellLogFile()
{
    CAF_PDM_InitObject( "Well File Info", ":/LasFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_fileName, "FileName", "Filename" );
    m_fileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_date, "Date", "Date" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannelNames, "WellLogFileChannels", "" );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_wellLogChannelNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFile::~RimWellLogFile()
{
    m_wellLogChannelNames.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::setFileName( const QString& fileName )
{
    m_fileName = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogFile::fileName() const
{
    return m_fileName().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFileChannel*> RimWellLogFile::wellLogChannels() const
{
    std::vector<RimWellLogFileChannel*> channels;
    for ( const auto& channel : m_wellLogChannelNames )
    {
        channels.push_back( channel );
    }
    return channels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellLogFile::date() const
{
    return m_date;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_date )
    {
        // Due to a possible bug in QDateEdit/PdmUiDateEditor, convert m_date to a QDateTime having UTC timespec
        m_date = RiaQDateTimeTools::createUtcDateTime( m_date().date(), m_date().time() );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( caf::PdmUiDateEditorAttribute* attrib = dynamic_cast<caf::PdmUiDateEditorAttribute*>( attribute ) )
    {
        attrib->dateFormat = RiaQDateTimeTools::dateFormatString();
    }
}
