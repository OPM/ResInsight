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

#include "RimWellEventTimeline.h"

#include "RimWellEventControl.h"
#include "RimWellEventKeyword.h"
#include "RimWellEventPerf.h"
#include "RimWellEventState.h"
#include "RimWellEventTubing.h"
#include "RimWellEventType.h"
#include "RimWellEventValve.h"

#include "RimDiameterRoughnessInterval.h"
#include "RimDiameterRoughnessIntervalCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimValveTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathValve.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

#include <algorithm>
#include <set>

CAF_PDM_SOURCE_INIT( RimWellEventTimeline, "WellEventTimeline" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventTimeline::RimWellEventTimeline()
{
    CAF_PDM_InitScriptableObject( "Well Event Timeline", "", "", "WellEventTimeline" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_events, "Events", "Events" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventTimeline::~RimWellEventTimeline()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellEvent*> RimWellEventTimeline::events() const
{
    return m_events.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimWellEventTimeline::getAllEventDates() const
{
    std::set<QDateTime> uniqueDates;

    for ( const auto& event : m_events )
    {
        if ( event && event->eventDate().isValid() )
        {
            uniqueDates.insert( event->eventDate() );
        }
    }

    return std::vector<QDateTime>( uniqueDates.begin(), uniqueDates.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellEvent*> RimWellEventTimeline::getEventsAtDate( const QDateTime& date ) const
{
    std::vector<RimWellEvent*> result;

    for ( auto& event : m_events )
    {
        if ( event && event->eventDate() == date )
        {
            result.push_back( event );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellEvent*> RimWellEventTimeline::getEventsByType( RimWellEvent::EventType type ) const
{
    std::vector<RimWellEvent*> result;

    for ( auto& event : m_events )
    {
        if ( event && event->eventType() == type )
        {
            result.push_back( event );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellEvent*> RimWellEventTimeline::getEventsUpToDate( const QDateTime& date ) const
{
    std::vector<RimWellEvent*> result;

    for ( auto& event : m_events )
    {
        if ( event && event->eventDate().isValid() && event->eventDate() <= date )
        {
            result.push_back( event );
        }
    }

    // Sort by date
    std::sort( result.begin(), result.end(), []( const RimWellEvent* a, const RimWellEvent* b ) { return a->eventDate() < b->eventDate(); } );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventPerf* RimWellEventTimeline::addPerforationEvent( const QString& wellName, const QDateTime& date )
{
    auto* event = new RimWellEventPerf();
    event->setWellName( wellName );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventValve* RimWellEventTimeline::addValveEvent( const QString& wellName, const QDateTime& date )
{
    auto* event = new RimWellEventValve();
    event->setWellName( wellName );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventTubing* RimWellEventTimeline::addTubingEvent( const QString& wellName, const QDateTime& date )
{
    auto* event = new RimWellEventTubing();
    event->setWellName( wellName );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventState* RimWellEventTimeline::addStateEvent( const QString& wellName, const QDateTime& date )
{
    auto* event = new RimWellEventState();
    event->setWellName( wellName );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventType* RimWellEventTimeline::addTypeEvent( const QString& wellName, const QDateTime& date )
{
    auto* event = new RimWellEventType();
    event->setWellName( wellName );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventControl* RimWellEventTimeline::addControlEvent( const QString& wellName, const QDateTime& date )
{
    auto* event = new RimWellEventControl();
    event->setWellName( wellName );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeyword* RimWellEventTimeline::addKeywordEvent( const QString& wellName, const QDateTime& date, const QString& keywordName )
{
    auto* event = new RimWellEventKeyword();
    event->setWellName( wellName );
    event->setEventDate( date );
    event->setKeywordName( keywordName );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTimeline::addEvent( RimWellEvent* event )
{
    if ( event )
    {
        m_events.push_back( event );
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTimeline::removeEvent( RimWellEvent* event )
{
    m_events.removeChild( event );
    delete event;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTimeline::clearAllEvents()
{
    m_events.deleteChildren();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellEventTimeline::eventCount() const
{
    return m_events.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventTimeline::applyEventsUpToDate( const QDateTime& date )
{
    // Get events up to the specified date (already sorted by date)
    std::vector<RimWellEvent*> eventsToApply = getEventsUpToDate( date );

    if ( eventsToApply.empty() )
    {
        return true; // Nothing to apply is not an error
    }

    // Get the well path collection from the parent hierarchy
    auto* wellPathCollection = firstAncestorOrThisOfType<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return false;
    }

    bool anyEventApplied = false;

    // Apply each event based on its type
    for ( RimWellEvent* event : eventsToApply )
    {
        // Find the well path by name
        RimWellPath* wellPath = wellPathCollection->wellPathByName( event->wellName() );
        if ( !wellPath )
        {
            continue;
        }

        bool success = false;
        switch ( event->eventType() )
        {
            case RimWellEvent::EventType::PERF:
            {
                auto* perfEvent = dynamic_cast<RimWellEventPerf*>( event );
                if ( perfEvent )
                {
                    success = applyPerfEvent( perfEvent, wellPath );
                }
                break;
            }
            case RimWellEvent::EventType::TUBING:
            {
                auto* tubingEvent = dynamic_cast<RimWellEventTubing*>( event );
                if ( tubingEvent )
                {
                    success = applyTubingEvent( tubingEvent, wellPath );
                }
                break;
            }
            case RimWellEvent::EventType::VALVE:
            {
                auto* valveEvent = dynamic_cast<RimWellEventValve*>( event );
                if ( valveEvent )
                {
                    success = applyValveEvent( valveEvent, wellPath );
                }
                break;
            }
            case RimWellEvent::EventType::WSTATE:
            case RimWellEvent::EventType::WCONTROL:
            case RimWellEvent::EventType::WTYPE:
                // Skip WSTATE and WCONTROL events per requirements
                break;
        }

        if ( success )
        {
            anyEventApplied = true;
        }
    }

    // Update views if any events were applied
    if ( anyEventApplied )
    {
        RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
    }

    return anyEventApplied;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventTimeline::applyTubingEvent( RimWellEventTubing* event, RimWellPath* wellPath )
{
    if ( !event || !wellPath )
    {
        return false;
    }

    auto* mswParams = wellPath->mswCompletionParameters();
    if ( !mswParams )
    {
        return false;
    }

    auto* intervalCollection = mswParams->diameterRoughnessIntervals();
    if ( !intervalCollection )
    {
        return false;
    }

    // Set the diameter roughness mode to intervals
    mswParams->setDiameterRoughnessMode( RimMswCompletionParameters::DiameterRoughnessMode::INTERVALS );

    // Create a new interval
    auto* interval = intervalCollection->createInterval( event->startMD(), event->endMD(), event->innerDiameter(), event->roughness() );

    if ( interval )
    {
        mswParams->updateConnectedEditors();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventTimeline::applyPerfEvent( RimWellEventPerf* event, RimWellPath* wellPath )
{
    if ( !event || !wellPath )
    {
        return false;
    }

    auto* perfCollection = wellPath->perforationIntervalCollection();
    if ( !perfCollection )
    {
        return false;
    }

    // Create a new perforation interval
    auto* perfInterval = new RimPerforationInterval;
    perfInterval->setStartAndEndMD( event->startMD(), event->endMD() );
    perfInterval->setDiameter( event->diameter() );
    perfInterval->setSkinFactor( event->skinFactor() );
    perfInterval->setUnitSystemSpecificDefaults();

    // Set the custom start date based on the event date
    perfInterval->enableCustomStartDate( true );
    perfInterval->setCustomStartDate( event->eventDate().date() );

    perfCollection->appendPerforation( perfInterval );
    perfCollection->updateConnectedEditors();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventTimeline::applyValveEvent( RimWellEventValve* event, RimWellPath* wellPath )
{
    if ( !event || !wellPath )
    {
        return false;
    }

    auto* perfCollection = wellPath->perforationIntervalCollection();
    if ( !perfCollection )
    {
        return false;
    }

    // Find a perforation interval that contains this valve's measured depth
    RimPerforationInterval*              targetPerf = nullptr;
    std::vector<RimPerforationInterval*> perfs      = perfCollection->perforationsNoConst();
    double                               valveMD    = event->measuredDepth();

    for ( auto* perf : perfs )
    {
        if ( valveMD >= perf->startMD() && valveMD <= perf->endMD() )
        {
            targetPerf = perf;
            break;
        }
    }

    if ( !targetPerf )
    {
        // No matching perforation found - valve cannot be placed
        return false;
    }

    // Create a new valve
    auto* valve = new RimWellPathValve;

    targetPerf->addValve( valve );

    valve->setMeasuredDepthAndCount( valveMD, 0, 1 );

    // Map the event valve type to the RiaDefines type and find a matching template
    RiaDefines::WellPathComponentType valveComponentType;
    switch ( event->valveType() )
    {
        case RimWellEventValve::ValveType::ICD:
            valveComponentType = RiaDefines::WellPathComponentType::ICD;
            break;
        case RimWellEventValve::ValveType::AICD:
            valveComponentType = RiaDefines::WellPathComponentType::AICD;
            break;
        case RimWellEventValve::ValveType::ICV:
        default:
            valveComponentType = RiaDefines::WellPathComponentType::ICV;
            break;
    }

    // Try to find a matching valve template
    RimProject* project = RimProject::current();
    if ( project )
    {
        std::vector<RimValveTemplate*> templates = project->allValveTemplates();
        for ( auto* tmpl : templates )
        {
            if ( tmpl->type() == valveComponentType )
            {
                valve->setValveTemplate( tmpl );
                break;
            }
        }
    }

    targetPerf->updateConnectedEditors();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTimeline::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "Event Timeline (%1 events)" ).arg( m_events.size() ) );

    // Sort events by date for display
    std::vector<RimWellEvent*> sortedEvents = events();
    std::sort( sortedEvents.begin(),
               sortedEvents.end(),
               []( const RimWellEvent* a, const RimWellEvent* b ) { return a->eventDate() < b->eventDate(); } );

    for ( auto* event : sortedEvents )
    {
        uiTreeOrdering.add( event );
    }

    uiTreeOrdering.skipRemainingChildren();
}
