/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimWellEventWellSpec.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellEventWellSpec, "WellEventWellSpec" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventWellSpec::RimWellEventWellSpec()
{
    CAF_PDM_InitScriptableObject( "Well Specification Event", "", "", "WellEventWellSpec" );

    CAF_PDM_InitScriptableField( &m_groupName, "GroupName", QString(), "Group Name" );
    CAF_PDM_InitScriptableField( &m_allowCrossFlow, "AllowCrossFlow", true, "Allow Cross-Flow" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_referenceDepth, "ReferenceDepth", "Reference Depth" );
    CAF_PDM_InitScriptableField( &m_wellType,
                                 "WellType",
                                 caf::AppEnum<RimWellPathCompletionSettings::WellType>( RimWellPathCompletionSettings::OIL ),
                                 "Preferred Fluid Phase" );

    CAF_PDM_InitField( &m_baselineGroupName, "BaselineGroupName", QString(), "Baseline Group Name" );
    CAF_PDM_InitField( &m_baselineAllowCrossFlow, "BaselineAllowCrossFlow", true, "Baseline Allow Cross-Flow" );
    CAF_PDM_InitFieldNoDefault( &m_baselineReferenceDepth, "BaselineReferenceDepth", "Baseline Reference Depth" );
    CAF_PDM_InitField( &m_baselineWellType,
                       "BaselineWellType",
                       caf::AppEnum<RimWellPathCompletionSettings::WellType>( RimWellPathCompletionSettings::OIL ),
                       "Baseline Preferred Fluid Phase" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventWellSpec::~RimWellEventWellSpec()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellSpecData RimWellEventWellSpec::wellSpecData() const
{
    return { m_groupName(), m_allowCrossFlow(), m_referenceDepth(), m_wellType() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellSpecData RimWellEventWellSpec::baselineData() const
{
    return { m_baselineGroupName(), m_baselineAllowCrossFlow(), m_baselineReferenceDepth(), m_baselineWellType() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventWellSpec::setWellSpecData( const RimWellSpecData& data )
{
    m_groupName      = data.groupName;
    m_allowCrossFlow = data.allowCrossFlow;
    m_referenceDepth = data.referenceDepth;
    m_wellType       = data.wellType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventWellSpec::setBaselineData( const RimWellSpecData& data )
{
    m_baselineGroupName      = data.groupName;
    m_baselineAllowCrossFlow = data.allowCrossFlow;
    m_baselineReferenceDepth = data.referenceDepth;
    m_baselineWellType       = data.wellType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventWellSpec::eventType() const
{
    return EventType::WELLSPEC;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventWellSpec::generateScheduleKeyword( const QString& wellName ) const
{
    return QString( "-- %1 WELLSPEC\n" ).arg( wellName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventWellSpec::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );
    uiOrdering.add( &m_groupName );
    uiOrdering.add( &m_allowCrossFlow );
    uiOrdering.add( &m_referenceDepth );
    uiOrdering.add( &m_wellType );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventWellSpec::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "WELLSPEC %1" ).arg( m_eventDate().toString( "yyyy-MM-dd" ) ) );
    uiTreeOrdering.skipRemainingChildren();
}
