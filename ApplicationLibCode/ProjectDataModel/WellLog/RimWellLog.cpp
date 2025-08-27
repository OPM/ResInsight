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
    CAF_PDM_InitObject( "Well File Info", ":/LasFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_date, "Date", "Date" );

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
    m_wellLogChannels.deleteChildren();

    if ( !wellLogData ) return;

    QStringList wellLogNames = wellLogData->wellLogChannelNames();
    for ( int logIdx = 0; logIdx < wellLogNames.size(); logIdx++ )
    {
        RimWellLogChannel* wellLog = new RimWellLogChannel();
        wellLog->setName( wellLogNames[logIdx] );
        m_wellLogChannels.push_back( wellLog );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLog::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( caf::PdmUiDateEditorAttribute* attrib = dynamic_cast<caf::PdmUiDateEditorAttribute*>( attribute ) )
    {
        attrib->dateFormat = RiaQDateTimeTools::dateFormatString();
    }
}
