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

#include "CompletionExportCommands/RicScheduleDataGenerator.h"
#include "RimEclipseCase.h"
#include "RimKeywordEvent.h"
#include "RimProject.h"
#include "RimWellEventControl.h"
#include "RimWellEventKeyword.h"
#include "RimWellEventPerf.h"
#include "RimWellEventState.h"
#include "RimWellEventTimeline.h"
#include "RimWellEventTubing.h"
#include "RimWellEventValve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimcDataContainerString.h"

#include "cafAppEnum.h"
#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

#include <QDateTime>

#include <algorithm>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addPerfEvent, "AddPerfEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addPerfEvent::RimcWellEventTimeline_addPerfEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Perforation Event", "", "", "Add a perforation event to the timeline" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableField( &m_startMd, "StartMd", 0.0, "", "", "", "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMd, "EndMd", 0.0, "", "", "", "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", 0.216, "", "", "", "Diameter [m]" );
    CAF_PDM_InitScriptableField( &m_skinFactor, "SkinFactor", 0.0, "", "", "", "Skin Factor" );
    auto defaultState = RimWellEventPerf::State::OPEN;
    CAF_PDM_InitScriptableField( &m_state, "State", defaultState, "", "", "", "State" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addPerfEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    if ( !m_wellPath() )
    {
        return std::unexpected( QString( "Well path is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addPerforationEvent( m_wellPath(), date );
    event->setStartMD( m_startMd() );
    event->setEndMD( m_endMd() );
    event->setDiameter( m_diameter() );
    event->setSkinFactor( m_skinFactor() );
    event->setState( m_state() );

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
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableField( &m_measuredDepth, "MeasuredDepth", 0.0, "", "", "", "Measured Depth" );
    auto defaultValveType = RimWellEventValve::ValveType::ICV;
    CAF_PDM_InitScriptableField( &m_valveType, "ValveType", defaultValveType, "", "", "", "Valve Type" );
    auto defaultState = RimWellEventValve::State::OPEN;
    CAF_PDM_InitScriptableField( &m_state, "State", defaultState, "", "", "", "State" );
    CAF_PDM_InitScriptableField( &m_flowCoefficient, "FlowCoefficient", 0.7, "", "", "", "Flow Coefficient" );
    CAF_PDM_InitScriptableField( &m_area, "Area", 0.0001, "", "", "", "Area [m2]" );
    CAF_PDM_InitScriptableField( &m_aicdStrength, "AicdStrength", 0.00021, "", "", "", "AICD Strength" );
    CAF_PDM_InitScriptableField( &m_aicdDensityCalibFluid, "AicdDensityCalibFluid", 1000.0, "", "", "", "AICD Density of Calibration Fluid [kg/m3]" );
    CAF_PDM_InitScriptableField( &m_aicdViscosityCalibFluid, "AicdViscosityCalibFluid", 1.0, "", "", "", "AICD Viscosity of Calibration Fluid [cP]" );
    CAF_PDM_InitScriptableField( &m_aicdVolFlowExp, "AicdVolFlowExp", 2.1, "", "", "", "AICD Volume Flow Rate Exponent" );
    CAF_PDM_InitScriptableField( &m_aicdViscFuncExp, "AicdViscFuncExp", 0.5, "", "", "", "AICD Viscosity Function Exponent" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addValveEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    if ( !m_wellPath() )
    {
        return std::unexpected( QString( "Well path is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addValveEvent( m_wellPath(), date );
    event->setMeasuredDepth( m_measuredDepth() );
    event->setFlowCoefficient( m_flowCoefficient() );
    event->setArea( m_area() );
    event->setValveType( m_valveType() );
    event->setState( m_state() );
    event->setAicdStrength( m_aicdStrength() );
    event->setAicdDensityCalibFluid( m_aicdDensityCalibFluid() );
    event->setAicdViscosityCalibFluid( m_aicdViscosityCalibFluid() );
    event->setAicdVolFlowExp( m_aicdVolFlowExp() );
    event->setAicdViscFuncExp( m_aicdViscFuncExp() );

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
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    auto defaultWellState = RimWellEventState::WellState::OPEN;
    CAF_PDM_InitScriptableField( &m_wellState, "WellState", defaultWellState, "", "", "", "Well State" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addStateEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    if ( !m_wellPath() )
    {
        return std::unexpected( QString( "Well path is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addStateEvent( m_wellPath(), date );
    event->setWellState( m_wellState() );

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
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    auto defaultControlMode = RimWellEventControl::ControlMode::ORAT;
    CAF_PDM_InitScriptableField( &m_controlMode, "ControlMode", defaultControlMode, "", "", "", "Control Mode" );
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

    if ( !m_wellPath() )
    {
        return std::unexpected( QString( "Well path is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addControlEvent( m_wellPath(), date );
    event->setControlValue( m_controlValue() );
    event->setBhpLimit( m_bhpLimit() );
    event->setOilRate( m_oilRate() );
    event->setWaterRate( m_waterRate() );
    event->setGasRate( m_gasRate() );
    event->setIsProducer( m_isProducer() );
    event->setControlMode( m_controlMode() );

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
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
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

    if ( !m_wellPath() )
    {
        return std::unexpected( QString( "Well path is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    auto* event = timeline->addTubingEvent( m_wellPath(), date );
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

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addWellKeywordEvent, "AddWellKeywordEventInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addWellKeywordEvent::RimcWellEventTimeline_addWellKeywordEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Well Keyword Event", "", "", "Add a well keyword event to the timeline" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableField( &m_keywordName, "KeywordName", QString(), "", "", "", "Keyword Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_itemNames, "ItemNames", "", "", "", "Item Names" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_itemTypes, "ItemTypes", "", "", "", "Item Types" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_itemValues, "ItemValues", "", "", "", "Item Values" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addWellKeywordEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    if ( !m_wellPath() )
    {
        return std::unexpected( QString( "Well path is required" ) );
    }

    if ( m_keywordName().isEmpty() )
    {
        return std::unexpected( QString( "Keyword name is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    // Validate array lengths
    if ( m_itemNames().size() != m_itemTypes().size() || m_itemNames().size() != m_itemValues().size() )
    {
        return std::unexpected( QString( "Item arrays must have same length" ) );
    }

    // Create event
    auto* event = timeline->addWellKeywordEvent( m_wellPath(), date, m_keywordName() );

    // Add items with type conversion
    for ( size_t i = 0; i < m_itemNames().size(); ++i )
    {
        QString type  = m_itemTypes()[i].toUpper();
        QString name  = m_itemNames()[i];
        QString value = m_itemValues()[i];

        if ( type == "STRING" )
        {
            event->addStringItem( name, value );
        }
        else if ( type == "INT" )
        {
            bool ok;
            int  intValue = value.toInt( &ok );
            if ( !ok )
            {
                return std::unexpected( QString( "Invalid integer value for '%1': %2" ).arg( name ).arg( value ) );
            }
            event->addIntItem( name, intValue );
        }
        else if ( type == "DOUBLE" )
        {
            bool   ok;
            double doubleValue = value.toDouble( &ok );
            if ( !ok )
            {
                return std::unexpected( QString( "Invalid double value for '%1': %2" ).arg( name ).arg( value ) );
            }
            event->addDoubleItem( name, doubleValue );
        }
        else
        {
            return std::unexpected( QString( "Unknown item type: %1. Must be STRING, INT, or DOUBLE" ).arg( type ) );
        }
    }

    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_addWellKeywordEvent::classKeywordReturnedType() const
{
    return RimWellEventKeyword::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_addKeywordEvent, "AddKeywordEventInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_addKeywordEvent::RimcWellEventTimeline_addKeywordEvent( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Keyword Event", "", "", "Add a schedule-level keyword event to the timeline (not tied to a well)" );

    CAF_PDM_InitScriptableField( &m_eventDate, "EventDate", QString( "2024-01-01" ), "", "", "", "Event Date (YYYY-MM-DD)" );
    CAF_PDM_InitScriptableField( &m_keywordName, "KeywordName", QString(), "", "", "", "Keyword Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_itemNames, "ItemNames", "", "", "", "Item Names" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_itemTypes, "ItemTypes", "", "", "", "Item Types" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_itemValues, "ItemValues", "", "", "", "Item Values" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_addKeywordEvent::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    if ( m_keywordName().isEmpty() )
    {
        return std::unexpected( QString( "Keyword name is required" ) );
    }

    QDateTime date = QDateTime::fromString( m_eventDate(), Qt::ISODate );
    if ( !date.isValid() )
    {
        return std::unexpected( QString( "Invalid date format: %1. Expected YYYY-MM-DD" ).arg( m_eventDate() ) );
    }

    // Validate array lengths
    if ( m_itemNames().size() != m_itemTypes().size() || m_itemNames().size() != m_itemValues().size() )
    {
        return std::unexpected( QString( "Item arrays must have same length" ) );
    }

    // Create event (note: no wellPath parameter)
    auto* event = timeline->addKeywordEvent( date, m_keywordName() );

    // Add items with type conversion
    for ( size_t i = 0; i < m_itemNames().size(); ++i )
    {
        QString type  = m_itemTypes()[i].toUpper();
        QString name  = m_itemNames()[i];
        QString value = m_itemValues()[i];

        if ( type == "STRING" )
        {
            event->addStringItem( name, value );
        }
        else if ( type == "INT" )
        {
            bool ok;
            int  intValue = value.toInt( &ok );
            if ( !ok )
            {
                return std::unexpected( QString( "Invalid integer value for '%1': %2" ).arg( name ).arg( value ) );
            }
            event->addIntItem( name, intValue );
        }
        else if ( type == "DOUBLE" )
        {
            bool   ok;
            double doubleValue = value.toDouble( &ok );
            if ( !ok )
            {
                return std::unexpected( QString( "Invalid double value for '%1': %2" ).arg( name ).arg( value ) );
            }
            event->addDoubleItem( name, doubleValue );
        }
        else
        {
            return std::unexpected( QString( "Unknown item type: %1. Must be STRING, INT, or DOUBLE" ).arg( type ) );
        }
    }

    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_addKeywordEvent::classKeywordReturnedType() const
{
    return RimKeywordEvent::classKeywordStatic();
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

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellEventTimeline, RimcWellEventTimeline_generateSchedule, "GenerateSchedule" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellEventTimeline_generateSchedule::RimcWellEventTimeline_generateSchedule( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Generate Schedule", "", "", "Generate Eclipse schedule text for all wells in the collection" );

    CAF_PDM_InitScriptableField( &m_eclipseCaseId, "EclipseCaseId", -1, "", "", "", "Eclipse Case ID" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellEventTimeline_generateSchedule::execute()
{
    auto timeline = self<RimWellEventTimeline>();

    // Get the parent well path collection
    RimWellPathCollection* wellPathCollection = timeline->firstAncestorOrThisOfType<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return std::unexpected( QString( "Could not find parent well path collection" ) );
    }

    // Find the Eclipse case by ID
    RimProject* project = wellPathCollection->firstAncestorOrThisOfType<RimProject>();
    if ( !project )
    {
        return std::unexpected( QString( "Could not find project" ) );
    }

    RimEclipseCase* eclipseCase = project->eclipseCaseFromCaseId( m_eclipseCaseId() );
    if ( !eclipseCase )
    {
        return std::unexpected( QString( "Eclipse case with ID %1 not found" ).arg( m_eclipseCaseId() ) );
    }

    // Get the last applied timestamp (from set_timestamp call)
    QDateTime lastTimestamp = timeline->lastAppliedTimestamp();

    // If no timestamp was set, use all events
    std::vector<QDateTime>    dates;
    std::vector<RimWellPath*> wellPathsWithEvents;

    if ( lastTimestamp.isValid() )
    {
        // Filter to only include dates up to the last applied timestamp
        dates = timeline->getAllEventDates();
        dates.erase( std::remove_if( dates.begin(), dates.end(), [&lastTimestamp]( const QDateTime& date ) { return date > lastTimestamp; } ),
                     dates.end() );

        // Get only well paths that have events up to the last applied timestamp
        wellPathsWithEvents = timeline->getWellPathsWithEventsUpToDate( lastTimestamp );
    }
    else
    {
        // No timestamp set - include all events and wells
        dates               = timeline->getAllEventDates();
        wellPathsWithEvents = timeline->getWellPathsWithEvents();
    }

    if ( dates.empty() )
    {
        return std::unexpected( QString( "No events found in timeline" ) );
    }

    if ( wellPathsWithEvents.empty() )
    {
        return std::unexpected( QString( "No well paths with events found" ) );
    }

    QString scheduleText = RicScheduleDataGenerator::generateSchedule( *timeline, *eclipseCase, wellPathsWithEvents, dates );

    // Return the schedule text in a data container
    auto* dataObject           = new RimcDataContainerString();
    dataObject->m_stringValues = { scheduleText };

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellEventTimeline_generateSchedule::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}
