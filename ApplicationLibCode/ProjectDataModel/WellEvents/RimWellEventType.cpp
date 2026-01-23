/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimWellEventType.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

namespace caf
{
template <>
void caf::AppEnum<RimWellEventType::WellType>::setUp()
{
    addItem( RimWellEventType::WellType::OIL_PRODUCER, "OIL_PRODUCER", "Oil Producer" );
    addItem( RimWellEventType::WellType::GAS_PRODUCER, "GAS_PRODUCER", "Gas Producer" );
    addItem( RimWellEventType::WellType::WATER_PRODUCER, "WATER_PRODUCER", "Water Producer" );
    addItem( RimWellEventType::WellType::WATER_INJECTOR, "WATER_INJECTOR", "Water Injector" );
    addItem( RimWellEventType::WellType::GAS_INJECTOR, "GAS_INJECTOR", "Gas Injector" );
    setDefault( RimWellEventType::WellType::OIL_PRODUCER );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellEventType, "WellEventType" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventType::RimWellEventType()
{
    CAF_PDM_InitScriptableObject( "Well Type Event", "", "", "WellEventType" );

    CAF_PDM_InitScriptableField( &m_wellType, "WellType", caf::AppEnum<WellType>( WellType::OIL_PRODUCER ), "Well Type" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventType::~RimWellEventType()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventType::WellType RimWellEventType::wellType() const
{
    return m_wellType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventType::setWellType( WellType type )
{
    m_wellType = type;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventType::isProducer() const
{
    return m_wellType() == WellType::OIL_PRODUCER || m_wellType() == WellType::GAS_PRODUCER || m_wellType() == WellType::WATER_PRODUCER;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventType::isInjector() const
{
    return m_wellType() == WellType::WATER_INJECTOR || m_wellType() == WellType::GAS_INJECTOR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventType::eventType() const
{
    return EventType::WTYPE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventType::generateScheduleKeyword( const QString& wellName ) const
{
    // Generate a comment about well type - actual well type specification
    // typically appears in WELSPECS which is defined once at simulation start
    QString typeStr;
    switch ( m_wellType() )
    {
        case WellType::OIL_PRODUCER:
            typeStr = "OIL PRODUCER";
            break;
        case WellType::GAS_PRODUCER:
            typeStr = "GAS PRODUCER";
            break;
        case WellType::WATER_PRODUCER:
            typeStr = "WATER PRODUCER";
            break;
        case WellType::WATER_INJECTOR:
            typeStr = "WATER INJECTOR";
            break;
        case WellType::GAS_INJECTOR:
            typeStr = "GAS INJECTOR";
            break;
    }

    QString line = QString( "-- %1 WELL TYPE: %2\n" ).arg( wellName ).arg( typeStr );

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventType::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_wellType );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventType::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    QString typeStr;
    switch ( m_wellType() )
    {
        case WellType::OIL_PRODUCER:
            typeStr = "OIL PROD";
            break;
        case WellType::GAS_PRODUCER:
            typeStr = "GAS PROD";
            break;
        case WellType::WATER_PRODUCER:
            typeStr = "WAT PROD";
            break;
        case WellType::WATER_INJECTOR:
            typeStr = "WAT INJ";
            break;
        case WellType::GAS_INJECTOR:
            typeStr = "GAS INJ";
            break;
    }

    setUiName( QString( "WTYPE %1: %2" ).arg( m_eventDate().toString( "yyyy-MM-dd" ) ).arg( typeStr ) );
}
