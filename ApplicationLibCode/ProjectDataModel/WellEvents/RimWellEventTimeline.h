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

#pragma once

#include "RimWellEvent.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <QDateTime>
#include <vector>

class RimWellEventPerf;
class RimWellEventValve;
class RimWellEventTubing;
class RimWellEventState;
class RimWellEventType;
class RimWellEventControl;
class RimWellEventKeyword;
class RimWellPath;

//==================================================================================================
///
/// Timeline container for well events
///
//==================================================================================================
class RimWellEventTimeline : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellEventTimeline();
    ~RimWellEventTimeline() override;

    // Access all events
    std::vector<RimWellEvent*> events() const;

    // Query methods
    std::vector<QDateTime>     getAllEventDates() const;
    std::vector<RimWellEvent*> getEventsAtDate( const QDateTime& date ) const;
    std::vector<RimWellEvent*> getEventsByType( RimWellEvent::EventType type ) const;
    std::vector<RimWellEvent*> getEventsUpToDate( const QDateTime& date ) const;
    std::vector<RimWellPath*>  getWellPathsWithEvents() const;

    // Add event methods (return the created event for further configuration)
    RimWellEventPerf*    addPerforationEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventValve*   addValveEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventTubing*  addTubingEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventState*   addStateEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventType*    addTypeEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventControl* addControlEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventKeyword* addWellKeywordEvent( RimWellPath* wellPath, const QDateTime& date, const QString& keywordName );

    // Generic add event method
    void addEvent( RimWellEvent* event );

    // Remove event
    void removeEvent( RimWellEvent* event );

    // Clear all events
    void clearAllEvents();

    // Get count
    size_t eventCount() const;

    // Apply events up to a given date (creates actual completions from event data)
    bool applyEventsUpToDate( const QDateTime& date );

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    bool applyTubingEvent( RimWellEventTubing* event, RimWellPath* wellPath );
    bool applyPerfEvent( RimWellEventPerf* event, RimWellPath* wellPath );
    bool applyValveEvent( RimWellEventValve* event, RimWellPath* wellPath );

    caf::PdmChildArrayField<RimWellEvent*> m_events;
};
