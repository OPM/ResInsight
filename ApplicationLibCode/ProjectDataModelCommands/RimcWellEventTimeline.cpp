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

#include "RimcWellEventTimeline.h"

#include "RimWellEventControl.h"
#include "RimWellEventPerf.h"
#include "RimWellEventState.h"
#include "RimWellEventTimeline.h"
#include "RimWellEventTubing.h"
#include "RimWellEventValve.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

#include <QDateTime>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addPerfEvent, "AddPerfEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addPerfEvent::RimcWellEventTimeline_addPerfEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Perforation Event", "", "", "Add a perforation event to the timeline" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableField( &m_wellName, "WellName", QString(), "", "", "", "Well Name" );
    CAF_PDM_InitScriptableField( &m_startMd, "StartMd", 0.0, "", "", "", "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMd, "EndMd", 0.0, "", "", "", "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", 0.216, "", "", "", "Diameter [m]" );
    CAF_PDM_InitScriptableField( &m_skinFactor, "SkinFactor", 0.0, "", "", "", "Skin Factor" );
    CAF_PDM_InitScriptableField( &m_state, "State", QString( "OPEN" ), "", "", "", "State (OPEN/SHUT)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addPerfEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    if ( m_wellName().isEmpty() )
    {
        return std::unexpected( QString( "Well name is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addPerforationEvent( m_wellName(), date );
    event->setStartMD( m_startMd() );
    event->setEndMD( m_endMd() );
    event->setDiameter( m_diameter() );
    event->setSkinFactor( m_skinFactor() );

    if ( m_state().toUpper() == "SHUT" )
    {
        event->setState( RimWellEventPerf::State::SHUT );
    }
    else
    {
        event->setState( RimWellEventPerf::State::OPEN );
    }

    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_addPerfEvent::classKeywordReturnedType() const
{
    return RimWellEventPerf::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addValveEvent, "AddValveEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addValveEvent::RimcWellEventTimeline_addValveEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Valve Event", "", "", "Add a valve event to the timeline" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableField( &m_wellName, "WellName", QString(), "", "", "", "Well Name" );
    CAF_PDM_InitScriptableField( &m_measuredDepth, "MeasuredDepth", 0.0, "", "", "", "Measured Depth" );
    CAF_PDM_InitScriptableField( &m_valveType, "ValveType", QString( "ICV" ), "", "", "", "Valve Type (ICV/ICD/AICD)" );
    CAF_PDM_InitScriptableField( &m_state, "State", QString( "OPEN" ), "", "", "", "State (OPEN/SHUT)" );
    CAF_PDM_InitScriptableField( &m_flowCoefficient, "FlowCoefficient", 0.7, "", "", "", "Flow Coefficient" );
    CAF_PDM_InitScriptableField( &m_area, "Area", 0.0001, "", "", "", "Area [m2]" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addValveEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addValveEvent( m_wellName, date );
    event->setMeasuredDepth( m_measuredDepth() );
    event->setFlowCoefficient( m_flowCoefficient() );
    event->setArea( m_area() );

    QString valveTypeUpper = m_valveType().toUpper();
    if ( valveTypeUpper == "ICD" )
    {
        event->setValveType( RimWellEventValve::ValveType::ICD );
    }
    else if ( valveTypeUpper == "AICD" )
    {
        event->setValveType( RimWellEventValve::ValveType::AICD );
    }
    else
    {
        event->setValveType( RimWellEventValve::ValveType::ICV );
    }

    if ( m_state().toUpper() == "SHUT" )
    {
        event->setState( RimWellEventValve::State::SHUT );
    }
    else
    {
        event->setState( RimWellEventValve::State::OPEN );
    }

    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_addValveEvent::classKeywordReturnedType() const
{
    return RimWellEventValve::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addStateEvent, "AddStateEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addStateEvent::RimcWellEventTimeline_addStateEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add State Event", "", "", "Add a well state event to the timeline" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableField( &m_wellName, "WellName", QString(), "", "", "", "Well Name" );
    CAF_PDM_InitScriptableField( &m_wellState, "WellState", QString( "OPEN" ), "", "", "", "Well State (OPEN/SHUT/STOP)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addStateEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addStateEvent( m_wellName(), date );

    QString stateUpper = m_wellState().toUpper();
    if ( stateUpper == "SHUT" )
    {
        event->setWellState( RimWellEventState::WellState::SHUT );
    }
    else if ( stateUpper == "STOP" )
    {
        event->setWellState( RimWellEventState::WellState::STOP );
    }
    else
    {
        event->setWellState( RimWellEventState::WellState::OPEN );
    }

    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_addStateEvent::classKeywordReturnedType() const
{
    return RimWellEventState::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addControlEvent, "AddControlEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addControlEvent::RimcWellEventTimeline_addControlEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Control Event", "", "", "Add a well control event to the timeline" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableField( &m_wellName, "WellName", QString(), "", "", "", "Well Name" );
    CAF_PDM_InitScriptableField( &m_controlMode, "ControlMode", QString( "ORAT" ), "", "", "", "Control Mode (ORAT/WRAT/GRAT/LRAT/BHP/THP)" );
    CAF_PDM_InitScriptableField( &m_controlValue, "ControlValue", 0.0, "", "", "", "Control Value" );
    CAF_PDM_InitScriptableField( &m_bhpLimit, "BhpLimit", 0.0, "", "", "", "BHP Limit [bar]" );
    CAF_PDM_InitScriptableField( &m_oilRate, "OilRate", 0.0, "", "", "", "Oil Rate [Sm3/day]" );
    CAF_PDM_InitScriptableField( &m_waterRate, "WaterRate", 0.0, "", "", "", "Water Rate [Sm3/day]" );
    CAF_PDM_InitScriptableField( &m_gasRate, "GasRate", 0.0, "", "", "", "Gas Rate [Sm3/day]" );
    CAF_PDM_InitScriptableField( &m_isProducer, "IsProducer", true, "", "", "", "Is Producer" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addControlEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addControlEvent( m_wellName(), date );
    event->setControlValue( m_controlValue() );
    event->setBhpLimit( m_bhpLimit() );
    event->setOilRate( m_oilRate() );
    event->setWaterRate( m_waterRate() );
    event->setGasRate( m_gasRate() );
    event->setIsProducer( m_isProducer() );

    QString modeUpper = m_controlMode().toUpper();
    if ( modeUpper == "WRAT" )
    {
        event->setControlMode( RimWellEventControl::ControlMode::WRAT );
    }
    else if ( modeUpper == "GRAT" )
    {
        event->setControlMode( RimWellEventControl::ControlMode::GRAT );
    }
    else if ( modeUpper == "LRAT" )
    {
        event->setControlMode( RimWellEventControl::ControlMode::LRAT );
    }
    else if ( modeUpper == "BHP" )
    {
        event->setControlMode( RimWellEventControl::ControlMode::BHP );
    }
    else if ( modeUpper == "THP" )
    {
        event->setControlMode( RimWellEventControl::ControlMode::THP );
    }
    else if ( modeUpper == "RESV" )
    {
        event->setControlMode( RimWellEventControl::ControlMode::RESV );
    }
    else
    {
        event->setControlMode( RimWellEventControl::ControlMode::ORAT );
    }

    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_addControlEvent::classKeywordReturnedType() const
{
    return RimWellEventControl::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addTubingEvent, "AddTubingEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addTubingEvent::RimcWellEventTimeline_addTubingEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Tubing Event", "", "", "Add a tubing event to the timeline" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableField( &m_wellName, "WellName", QString(), "", "", "", "Well Name" );
    CAF_PDM_InitScriptableField( &m_startMd, "StartMd", 0.0, "", "", "", "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMd, "EndMd", 0.0, "", "", "", "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_innerDiameter, "InnerDiameter", 0.15, "", "", "", "Inner Diameter [m]" );
    CAF_PDM_InitScriptableField( &m_roughness, "Roughness", 1.0e-5, "", "", "", "Roughness [m]" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addTubingEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addTubingEvent( m_wellName(), date );
    event->setStartMD( m_startMd() );
    event->setEndMD( m_endMd() );
    event->setInnerDiameter( m_innerDiameter() );
    event->setRoughness( m_roughness() );

    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_addTubingEvent::classKeywordReturnedType() const
{
    return RimWellEventTubing::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_setTimestamp, "SetTimestamp" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_setTimestamp::RimcWellEventTimeline_setTimestamp( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Timestamp", "", "", "Apply well events up to a given timestamp" );

    CAF_PDM_InitScriptableField( &m_timestamp, "Timestamp", QString( "2024-01-01" ), "", "", "", "Timestamp (YYYY-MM-DD)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_setTimestamp::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    QDateTime date = QDateTime::fromString( m_timestamp(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_timestamp() ) );
    }

    bool success = timeline->applyEventsUpToDate( date );

    if ( !success )
    {
        return std::unexpected( QString( "No events were applied up to date: %1" ).arg( m_timestamp() ) );
    }

    return nullptr; // Return nullptr on success (no specific object to return)
}
