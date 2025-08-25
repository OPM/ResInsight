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

#include "Well/RigImportedWellLogData.h"

#include "RimWellPath.h"

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
    RimWellPath* wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
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
    return m_wellLogData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimImportedWellLog::setWellLogData( RigImportedWellLogData* wellLogData )
{
    m_wellLogData = wellLogData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>>
    RimImportedWellLog::findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString )
{
    if ( unitString ) *unitString = QString();

    if ( !m_wellLogData.notNull() ) return {};

    std::vector<double> depths = m_wellLogData->depthValues();
    std::vector<double> values = m_wellLogData->values( channelName );

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
caf::PdmFieldHandle* RimImportedWellLog::userDescriptionField()
{
    return &m_name;
}