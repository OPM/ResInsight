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

#include "RimImportedWellLog.h"

#include "RimImportedWellLogData.h"
#include "RimWellLogChannel.h"
#include "RimWellPath.h"

#include "Well/RigImportedWellLogData.h"

#include "RiaFieldHandleTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QString>

CAF_PDM_SOURCE_INIT( RimImportedWellLog, "ImportedWellLog" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimImportedWellLog::RimImportedWellLog()
{
    CAF_PDM_InitScriptableObject( "Imported Well Log", ":/LasFile16x16.png", "", "ImportedWellLog" );

    CAF_PDM_InitScriptableField( &m_name, "Name", QString(), "Name" );
    m_name.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogData, "WellLogData", "Well Log Data" );
    m_wellLogData.uiCapability()->setUiTreeChildrenHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLog::setName( const QString& name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimImportedWellLog::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimImportedWellLog::wellName() const
{
    if ( RimWellPath* wellPath = firstAncestorOrThisOfType<RimWellPath>() )

    {
        return wellPath->name();
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogData* RimImportedWellLog::wellLogData()
{
    return m_cachedRigData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLog::setWellLogData( RimImportedWellLogData* wellLogData )
{
    m_wellLogData = wellLogData;
    // Clear cached Rig data when setting new PDM data
    m_cachedRigData = nullptr;

    if ( wellLogData )
    {
        // Create temporary Rig data to update channels
        m_cachedRigData = wellLogData->createRigData();
        updateChannelsFromWellLogData( m_cachedRigData.p() );
    }
    else
    {
        m_wellLogChannels.deleteChildren();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>>
    RimImportedWellLog::findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString )
{
    if ( unitString ) *unitString = QString();

    RigWellLogData* wld = wellLogData();
    if ( !wld ) return {};

    std::vector<double> depths = wld->depthValues();
    std::vector<double> values = wld->values( channelName );

    std::vector<std::pair<double, double>> depthValuePairs;
    if ( depths.size() == values.size() )
    {
        for ( size_t i = 0; i < depths.size(); ++i )
        {
            depthValuePairs.emplace_back( depths[i], values[i] );
        }
    }

    return depthValuePairs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLog::initAfterRead()
{
    // Create cached Rig data from PDM data when loading from project file
    if ( m_wellLogData() && !m_cachedRigData.notNull() )
    {
        m_cachedRigData = m_wellLogData()->createRigData();
        updateChannelsFromWellLogData( m_cachedRigData.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimImportedWellLog::userDescriptionField()
{
    return &m_name;
}
