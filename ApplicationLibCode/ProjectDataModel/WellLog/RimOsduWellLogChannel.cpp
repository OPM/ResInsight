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

#include "RimOsduWellLogChannel.h"

#include "RiaFieldHandleTools.h"

CAF_PDM_SOURCE_INIT( RimOsduWellLogChannel, "OsduWellLogChannel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOsduWellLogChannel::RimOsduWellLogChannel()
{
    CAF_PDM_InitObject( "OSDU Well Log Channel" );

    CAF_PDM_InitFieldNoDefault( &m_id, "Id", "Id" );
    m_id.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_topDepth, "TopDepth", "Top Depth" );
    m_topDepth.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_baseDepth, "BaseDepth", "Base Depth" );
    m_baseDepth.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_interpreterName, "InterpreterName", "Interpreter Name" );
    m_interpreterName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_quality, "Quality", "Quality" );
    m_quality.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_unit, "Unit", "Unit" );
    m_unit.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_depthUnit, "DepthUnit", "Depth Unit" );
    m_depthUnit.uiCapability()->setUiReadOnly( true );

    // Need to save the name for Osdu well log channels.
    // This reverts settings from RimWellLogChannel constructor.
    nameField()->xmlCapability()->setIOReadable( true );
    nameField()->xmlCapability()->setIOWritable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setId( const QString& id )
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setTopDepth( double topDepth )
{
    m_topDepth = topDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setBaseDepth( double baseDepth )
{
    m_baseDepth = baseDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setInterpreterName( const QString& interpreterName )
{
    m_interpreterName = interpreterName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setQuality( const QString& quality )
{
    m_quality = quality;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setUnit( const QString& unit )
{
    m_unit = unit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellLogChannel::setDepthUnit( const QString& depthUnit )
{
    m_depthUnit = depthUnit;
}
