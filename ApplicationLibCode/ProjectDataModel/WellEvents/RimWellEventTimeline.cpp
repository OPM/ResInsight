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

#include "RimKeywordEvent.h"
#include "RimWellEvent.h"
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
#include "RimValveTemplateCollection.h"
#include "RimWellPath.h"
#include "RimWellPathAicdParameters.h"
#include "RimWellPathCollection.h"
#include "RimWellPathValve.h"

#include "Riu3DMainWindowTools.h"

#include "cvfMath.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

#include <algorithm>
#include <cmath>
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
std::vector<RimWellPath*> RimWellEventTimeline::getWellPathsWithEvents() const
{
    std::set<RimWellPath*> uniqueWellPaths;

    for ( auto& event : m_events )
    {
        if ( event && event->wellPath() )
        {
            uniqueWellPaths.insert( event->wellPath() );
        }
    }

    return std::vector<RimWellPath*>( uniqueWellPaths.begin(), uniqueWellPaths.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellEventTimeline::getWellPathsWithEventsUpToDate( const QDateTime& date ) const
{
    std::set<RimWellPath*> uniqueWellPaths;

    for ( auto& event : m_events )
    {
        if ( event && event->wellPath() && event->eventDate().isValid() && event->eventDate() <= date )
        {
            uniqueWellPaths.insert( event->wellPath() );
        }
    }

    return std::vector<RimWellPath*>( uniqueWellPaths.begin(), uniqueWellPaths.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellEventTimeline::lastAppliedTimestamp() const
{
    return m_lastAppliedTimestamp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventPerf* RimWellEventTimeline::addPerforationEvent( RimWellPath* wellPath, const QDateTime& date )
{
    auto* event = new RimWellEventPerf();
    event->setWellPath( wellPath );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventValve* RimWellEventTimeline::addValveEvent( RimWellPath* wellPath, const QDateTime& date )
{
    auto* event = new RimWellEventValve();
    event->setWellPath( wellPath );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventTubing* RimWellEventTimeline::addTubingEvent( RimWellPath* wellPath, const QDateTime& date )
{
    auto* event = new RimWellEventTubing();
    event->setWellPath( wellPath );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventState* RimWellEventTimeline::addStateEvent( RimWellPath* wellPath, const QDateTime& date )
{
    auto* event = new RimWellEventState();
    event->setWellPath( wellPath );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventType* RimWellEventTimeline::addTypeEvent( RimWellPath* wellPath, const QDateTime& date )
{
    auto* event = new RimWellEventType();
    event->setWellPath( wellPath );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventControl* RimWellEventTimeline::addControlEvent( RimWellPath* wellPath, const QDateTime& date )
{
    auto* event = new RimWellEventControl();
    event->setWellPath( wellPath );
    event->setEventDate( date );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeyword* RimWellEventTimeline::addWellKeywordEvent( RimWellPath* wellPath, const QDateTime& date, const QString& keywordName )
{
    auto* event = new RimWellEventKeyword();
    event->setWellPath( wellPath );
    event->setEventDate( date );
    event->setKeywordName( keywordName );
    m_events.push_back( event );
    updateConnectedEditors();
    return event;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordEvent* RimWellEventTimeline::addKeywordEvent( const QDateTime& date, const QString& keywordName )
{
    auto* event = new RimKeywordEvent();
    // Note: wellPath is NOT set for schedule-level keyword events
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
    // Store the timestamp for later use in schedule generation
    m_lastAppliedTimestamp = date;

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
        bool success = applyEvent( wellPathCollection, event );
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
bool RimWellEventTimeline::applyEvent( RimWellPathCollection* wellPathCollection, RimWellEvent* event )
{
    // Find the well path by name
    if ( RimWellPath* wellPath = wellPathCollection->wellPathByName( event->wellName() ) )
    {
        switch ( event->eventType() )
        {
            case RimWellEvent::EventType::PERF:
            {
                if ( auto* perfEvent = dynamic_cast<RimWellEventPerf*>( event ) )
                {
                    return applyPerfEvent( *perfEvent, *wellPath );
                }
                break;
            }
            case RimWellEvent::EventType::TUBING:
            {
                if ( auto* tubingEvent = dynamic_cast<RimWellEventTubing*>( event ) )
                {
                    return applyTubingEvent( *tubingEvent, *wellPath );
                }
                break;
            }
            case RimWellEvent::EventType::VALVE:
            {
                if ( auto* valveEvent = dynamic_cast<RimWellEventValve*>( event ) )
                {
                    return applyValveEvent( *valveEvent, *wellPath );
                }
                break;
            }
            case RimWellEvent::EventType::WSTATE:
            case RimWellEvent::EventType::WCONTROL:
            case RimWellEvent::EventType::WTYPE:
                // Skip WSTATE and WCONTROL events per requirements
                break;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventTimeline::applyTubingEvent( const RimWellEventTubing& event, RimWellPath& wellPath )
{
    auto* mswParams = wellPath.mswCompletionParameters();
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
    auto* interval = intervalCollection->createInterval( event.startMD(), event.endMD(), event.innerDiameter(), event.roughness() );

    if ( interval )
    {
        // Set the custom start date based on the event date
        interval->enableCustomStartDate( true );
        interval->setCustomStartDate( event.eventDate().date() );

        mswParams->updateConnectedEditors();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventTimeline::applyPerfEvent( const RimWellEventPerf& event, RimWellPath& wellPath )
{
    auto* perfCollection = wellPath.perforationIntervalCollection();
    if ( !perfCollection )
    {
        return false;
    }

    // Create a new perforation interval
    auto* perfInterval = new RimPerforationInterval;
    perfInterval->setStartAndEndMD( event.startMD(), event.endMD() );
    perfInterval->setDiameter( event.diameter() );
    perfInterval->setSkinFactor( event.skinFactor() );
    perfInterval->setUnitSystemSpecificDefaults();

    // Set the custom start date based on the event date
    perfInterval->enableCustomStartDate( true );
    perfInterval->setCustomStartDate( event.eventDate().date() );

    perfCollection->appendPerforation( perfInterval );
    perfCollection->updateConnectedEditors();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellEventTimeline::applyValveEvent( const RimWellEventValve& event, RimWellPath& wellPath )
{
    auto* perfCollection = wellPath.perforationIntervalCollection();
    if ( !perfCollection )
    {
        return false;
    }

    auto findByMeasuredDepth = []( const std::vector<RimPerforationInterval*>& perfs, double valveMD ) -> RimPerforationInterval*
    {
        for ( auto* perf : perfs )
        {
            if ( valveMD >= perf->startMD() && valveMD <= perf->endMD() ) return perf;
        }

        return nullptr;
    };

    // Find a perforation interval that contains this valve's measured depth
    std::vector<RimPerforationInterval*> perfs      = perfCollection->perforationsNoConst();
    double                               valveMD    = event.measuredDepth();
    RimPerforationInterval*              targetPerf = findByMeasuredDepth( perfs, valveMD );

    if ( !targetPerf )
    {
        // No matching perforation found - valve cannot be placed
        return false;
    }

    // Create a new valve
    auto* valve = new RimWellPathValve;

    targetPerf->addValve( valve );

    // For ICD/AICD valves, the valve range must cover the perforation interval so that
    // valveLocations() returns a non-empty list and the accumulator can validate the valve.
    // For ICV, a single point at the valve MD is sufficient.
    if ( event.valveType() == RimWellEventValve::ValveType::ICV )
    {
        valve->setMeasuredDepthAndCount( valveMD, 0, 1 );
    }
    else
    {
        double perfRange = targetPerf->endMD() - targetPerf->startMD();
        valve->setMeasuredDepthAndCount( targetPerf->startMD(), perfRange, 1 );
    }

    // Set the custom start date based on the event date
    valve->enableCustomStartDate( true );
    valve->setCustomStartDate( event.eventDate().date() );

    auto convertToValveType = []( RimWellEventValve::ValveType valveType )
    {
        switch ( valveType )
        {
            case RimWellEventValve::ValveType::ICD:
                return RiaDefines::WellPathComponentType::ICD;
            case RimWellEventValve::ValveType::AICD:
                return RiaDefines::WellPathComponentType::AICD;
            case RimWellEventValve::ValveType::ICV:
            default:
                return RiaDefines::WellPathComponentType::ICV;
        }
    };

    // Map the event valve type to the RiaDefines type and find a matching template
    RiaDefines::WellPathComponentType valveComponentType = convertToValveType( event.valveType() );

    // Create a new valve template with the event's specific parameters
    if ( RimProject* project = RimProject::current() )
    {
        auto collections = project->allValveTemplateCollections();
        if ( !collections.empty() )
        {
            auto* tmpl = new RimValveTemplate;
            tmpl->setType( valveComponentType );

            tmpl->setFlowCoefficient( event.flowCoefficient() );
            double orifice_mm = 2000.0 * std::sqrt( event.area() / cvf::PI_D );
            tmpl->setOrificeDiameter( orifice_mm );

            if ( valveComponentType == RiaDefines::WellPathComponentType::AICD )
            {
                tmpl->setAicdParameter( AICD_STRENGTH, event.aicdStrength() );
                tmpl->setAicdParameter( AICD_DENSITY_CALIB_FLUID, event.aicdDensityCalibFluid() );
                tmpl->setAicdParameter( AICD_VISCOSITY_CALIB_FLUID, event.aicdViscosityCalibFluid() );
                tmpl->setAicdParameter( AICD_VOL_FLOW_EXP, event.aicdVolFlowExp() );
                tmpl->setAicdParameter( AICD_VISOSITY_FUNC_EXP, event.aicdViscFuncExp() );
            }

            tmpl->setUserLabel( QString( "Event Valve %1" ).arg( event.measuredDepth() ) );
            collections.front()->addItem( tmpl );
            valve->setValveTemplate( tmpl );
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
