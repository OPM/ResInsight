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

#include "RimWellLog.h"

#include "RimFileWellPath.h"
#include "RimTools.h"
#include "RimWellLogChannel.h"

#include "Well/RigWellLogData.h"

#include "RiaFieldHandleTools.h"
#include "RiaQDateTimeTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDateEditor.h"

#include <QString>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimWellLog, "WellLog" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QDateTime RimWellLog::DEFAULT_DATE_TIME = RiaQDateTimeTools::createUtcDateTime( QDate( 1900, 1, 1 ) );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLog::RimWellLog()
{
    CAF_PDM_InitScriptableObject( "Well File Info", ":/WellLogFile.svg" );

    CAF_PDM_InitFieldNoDefault( &m_date, "Date", "Date" );
    m_date.uiCapability()->setAttribute( caf::PdmUiDateEditor::Keys::DATE_FORMAT, RiaQDateTimeTools::dateFormatString() );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannels, "WellLogChannels", "" );
    m_wellLogChannels.registerKeywordAlias( "WellLogFileChannels" );

    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_wellLogChannels );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogChannel*> RimWellLog::wellLogChannels() const
{
    return m_wellLogChannels.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellLog::date() const
{
    return m_date;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLog::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
void RimWellLog::updateChannelsFromWellLogData( RigWellLogData* wellLogData )
{
    if ( !wellLogData )
    {
        m_wellLogChannels.deleteChildren();
        return;
    }

    QStringList wellLogNames = wellLogData->wellLogChannelNames();

    // Keep existing channel objects when the names are unchanged, to avoid stale entries in the project tree
    // when a file is read several times
    QStringList existingNames;
    for ( const auto* channel : wellLogChannels() )
    {
        existingNames.append( channel->name() );
    }
    if ( existingNames == wellLogNames ) return;

    m_wellLogChannels.deleteChildren();

    for ( int logIdx = 0; logIdx < wellLogNames.size(); logIdx++ )
    {
        RimWellLogChannel* wellLog = new RimWellLogChannel();
        wellLog->setName( wellLogNames[logIdx] );
        m_wellLogChannels.push_back( wellLog );
    }
}
