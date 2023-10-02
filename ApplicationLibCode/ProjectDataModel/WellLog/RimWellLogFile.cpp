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

#include <QString>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimWellLogFile, "WellLogFileInterface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFile::RimWellLogFile()
{
    CAF_PDM_InitObject( "Well File Info", ":/LasFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_fileName, "FileName", "Filename" );
    m_fileName.uiCapability()->setUiReadOnly( true );

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
