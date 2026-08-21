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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

#include <QDateTime>
#include <vector>

class RimWellEventPerf;
class RimWellEventValve;
class RimWellEventTubing;
class RimWellEventState;
class RimWellEventType;
class RimWellEventWellSpec;
class RimWellEventControl;
class RimWellEventKeyword;
class RimKeywordEvent;
class RimWellEventRawText;
class RimWellPath;
class RimWellPathCollection;

//==================================================================================================
///
/// Timeline container for well events
///
//==================================================================================================
class RimWellEventTimeline : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SortMode
    {
        DATE,
        WELL,
        TYPE
    };

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
    std::vector<RimWellPath*>  getWellPathsWithEventsUpToDate( const QDateTime& date ) const;

    // Add event methods (return the created event for further configuration)
    RimWellEventPerf*     addPerforationEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventValve*    addValveEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventTubing*   addTubingEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventState*    addStateEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventType*     addTypeEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventWellSpec* addWellSpecEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventControl*  addControlEvent( RimWellPath* wellPath, const QDateTime& date );
    RimWellEventKeyword*  addWellKeywordEvent( RimWellPath* wellPath, const QDateTime& date, const QString& keywordName );
    RimKeywordEvent*      addKeywordEvent( const QDateTime& date, const QString& keywordName );
    RimWellEventRawText*  addRawTextEvent( const QDateTime& date );

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

    // Get the last applied timestamp (returns invalid QDateTime if no timestamp was set)
    QDateTime lastAppliedTimestamp() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    bool applyEvent( RimWellPathCollection* wellPathCollection, RimWellEvent* event );

    bool applyTubingEvent( const RimWellEventTubing& event, RimWellPath& wellPath );
    bool applyPerfEvent( const RimWellEventPerf& event, RimWellPath& wellPath );
    bool applyValveEvent( RimWellEventValve& event, RimWellPath& wellPath );
    bool applyWellSpecEvent( const RimWellEventWellSpec& event, RimWellPath& wellPath );

    std::vector<RimWellEvent*> filteredAndSortedEventsForUi() const;
    void                       updateEditorsAfterEventChange();

    caf::PdmChildArrayField<RimWellEvent*> m_events;
    QDateTime                              m_lastAppliedTimestamp;

    caf::PdmField<caf::AppEnum<SortMode>>                             m_sortBy;
    caf::PdmField<bool>                                               m_sortDescending;
    caf::PdmPtrArrayField<RimWellPath*>                               m_wellPathFilter;
    caf::PdmField<std::vector<caf::AppEnum<RimWellEvent::EventType>>> m_eventTypeFilter;
    caf::PdmField<bool>                                               m_filterByStartDate;
    caf::PdmField<QDateTime>                                          m_startDateFilter;
    caf::PdmField<bool>                                               m_filterByEndDate;
    caf::PdmField<QDateTime>                                          m_endDateFilter;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventTimeline::SortMode>::setUp();
} // namespace caf
