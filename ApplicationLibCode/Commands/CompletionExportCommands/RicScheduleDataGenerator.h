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

#include <QDateTime>
#include <QString>

#include <expected>
#include <map>
#include <optional>
#include <set>
#include <vector>

class RimEclipseCase;
class RimWellPath;
class RimWellEvent;
class RimWellEventTimeline;

namespace Opm
{
class DeckKeyword;
} // namespace Opm

//==================================================================================================
///
/// Generator for Eclipse schedule data based on well events
///
//==================================================================================================
class RicScheduleDataGenerator
{
public:
    // Generate schedule for multiple wells at specified dates.
    // mswWells lists the wells for which the multi-segment-well keywords (WELSEGS, COMPSEGS,
    // WSEGVALV, WSEGAICD) are exported. Wells not in the set get no MSW keywords; an empty set
    // suppresses MSW export for all wells. Well paths sharing an export name are one well in the deck:
    // selecting any of them exports the MSW keywords once for their shared well tree. MSW export returns
    // an error if a shared export name spans independent well trees, since COMPSEGS must map every connection.
    // When firstDateAsComment is true, the first (earliest) date is written as a comment line
    // instead of a DATES keyword, since some commercial simulators reject a DATES entry that
    // equals the simulation start date. Later dates are always emitted as DATES keywords.
    // When alignColumns is true, keywords are serialised with a "--"-prefixed column-header comment
    // and right-aligned, fixed-width columns instead of the compact default form.
    static std::expected<QString, QString> generateSchedule( const RimWellEventTimeline&         timeline,
                                                             RimEclipseCase&                     eclipseCase,
                                                             const std::vector<RimWellPath*>&    wellPaths,
                                                             const std::vector<QDateTime>&       dates,
                                                             const std::set<const RimWellPath*>& mswWells,
                                                             bool                                firstDateAsComment = true,
                                                             bool                                alignColumns       = false );

    // Collect all unique dates from all wells' timelines
    static std::vector<QDateTime> collectAllDates( const RimWellEventTimeline& timeline, const std::vector<RimWellPath*>& wellPaths );

    // Generate schedule for every event in the timeline. The dates and well paths are collected
    // from the timeline: when events have been applied up to a timestamp (see
    // RimWellEventTimeline::applyEventsUpToDate), later dates and wells are left out. Insert-date
    // events only force a DATES keyword and are therefore never filtered by that timestamp.
    // The remaining arguments are passed on to generateSchedule().
    static std::expected<QString, QString> generateScheduleForTimeline( const RimWellEventTimeline&         timeline,
                                                                        RimEclipseCase&                     eclipseCase,
                                                                        const std::set<const RimWellPath*>& mswWells,
                                                                        bool                                firstDateAsComment = true,
                                                                        bool                                alignColumns       = false );

private:
    // Generate schedule section for a single date. When dateAsComment is true, the date is
    // written as a comment line instead of a DATES keyword.
    static std::expected<QString, QString> generateDateSection( const RimWellEventTimeline&         timeline,
                                                                RimEclipseCase&                     eclipseCase,
                                                                const std::vector<RimWellPath*>&    wellPaths,
                                                                const QDateTime&                    date,
                                                                const std::set<const RimWellPath*>& mswWells,
                                                                std::map<QString, QString>&         emittedSegmentLayouts,
                                                                bool                                dateAsComment = false,
                                                                bool                                alignColumns  = false );

    static std::optional<Opm::DeckKeyword> generateWelspecsForWell( const RimWellEventTimeline&       timeline,
                                                                    RimEclipseCase&                   eclipseCase,
                                                                    RimWellPath&                      well,
                                                                    const QDateTime&                  date,
                                                                    const std::vector<RimWellEvent*>& eventsAtDate );

    // Generate COMPDAT (and COMPLUMP, when perforations carry a completion number) for a group of well paths sharing
    // one export name at a specific date based on events, merging both into the accumulator. Connections in cells
    // shared by several well paths in the group are combined into one record.
    static void generateCompletionsForWellGroup( RimEclipseCase&                      eclipseCase,
                                                 const std::vector<RimWellPath*>&     wellGroup,
                                                 const QDateTime&                     date,
                                                 const std::vector<RimWellEvent*>&    eventsAtDate,
                                                 std::map<QString, Opm::DeckKeyword>& keywordBlocks );

    // Generate WELSEGS / COMPSEGS / WSEGVALV / WSEGAICD for a well at a specific date.
    // WSEGVALV / WSEGAICD are merged into keywordBlocks; WELSEGS / COMPSEGS cannot be merged across
    // wells and are appended as separate per-well blocks in unmergedBlocks.
    // The caller decides which wells get MSW keywords (see mswWells in generateSchedule).
    // WELSEGS / COMPSEGS are written for segment/perforation events, and for valve events when the
    // segment layout differs from the one last written for the well (tracked in emittedSegmentLayouts).
    static void generateMswForWell( RimEclipseCase&                                   eclipseCase,
                                    RimWellPath&                                      well,
                                    const QDateTime&                                  date,
                                    const std::vector<RimWellEvent*>&                 eventsAtDate,
                                    std::map<QString, Opm::DeckKeyword>&              keywordBlocks,
                                    std::map<QString, std::vector<Opm::DeckKeyword>>& unmergedBlocks,
                                    std::map<QString, QString>&                       emittedSegmentLayouts );

    // Generate well control / well keyword event keywords for a well at a specific date, merging into the accumulator
    static void generateWellControlForWell( const RimWellPath&                   well,
                                            const std::vector<RimWellEvent*>&    eventsAtDate,
                                            std::map<QString, Opm::DeckKeyword>& keywordBlocks );

    // Append records of `kw` into the entry for `name`, creating it from `kw` if absent
    static void mergeKeyword( std::map<QString, Opm::DeckKeyword>& acc, const QString& name, Opm::DeckKeyword kw );
};
