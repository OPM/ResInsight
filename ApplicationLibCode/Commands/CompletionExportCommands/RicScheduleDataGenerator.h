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
    // suppresses MSW export for all wells.
    static QString generateSchedule( const RimWellEventTimeline&         timeline,
                                     RimEclipseCase&                     eclipseCase,
                                     const std::vector<RimWellPath*>&    wellPaths,
                                     const std::vector<QDateTime>&       dates,
                                     const std::set<const RimWellPath*>& mswWells );

    // Collect all unique dates from all wells' timelines
    static std::vector<QDateTime> collectAllDates( const RimWellEventTimeline& timeline, const std::vector<RimWellPath*>& wellPaths );

private:
    // Generate schedule section for a single date
    static QString generateDateSection( const RimWellEventTimeline&         timeline,
                                        RimEclipseCase&                     eclipseCase,
                                        const std::vector<RimWellPath*>&    wellPaths,
                                        const QDateTime&                    date,
                                        const std::set<const RimWellPath*>& mswWells );

    static std::optional<Opm::DeckKeyword>
        generateWelspecsForWell( const RimWellEventTimeline& timeline, RimEclipseCase& eclipseCase, RimWellPath& well, const QDateTime& date );

    // Generate COMPDAT (and COMPLUMP, when perforations carry a completion number) for a well at a
    // specific date based on events, merging both into the accumulator.
    static void generateCompletionsForWell( const RimWellEventTimeline&          timeline,
                                            RimEclipseCase&                      eclipseCase,
                                            RimWellPath&                         well,
                                            const QDateTime&                     date,
                                            std::map<QString, Opm::DeckKeyword>& keywordBlocks );

    // Generate WELSEGS / COMPSEGS / WSEGVALV / WSEGAICD for a well at a specific date.
    // WSEGVALV / WSEGAICD are merged into keywordBlocks; WELSEGS / COMPSEGS cannot be merged across
    // wells and are appended as separate per-well blocks in unmergedBlocks.
    // All four keywords are emitted only when the well is present in mswWells; otherwise none are.
    static void generateMswForWell( const RimWellEventTimeline&                       timeline,
                                    RimEclipseCase&                                   eclipseCase,
                                    RimWellPath&                                      well,
                                    const QDateTime&                                  date,
                                    std::map<QString, Opm::DeckKeyword>&              keywordBlocks,
                                    std::map<QString, std::vector<Opm::DeckKeyword>>& unmergedBlocks,
                                    const std::set<const RimWellPath*>&               mswWells );

    // Generate well control / well keyword event keywords for a well at a specific date, merging into the accumulator
    static void generateWellControlForWell( const RimWellEventTimeline&          timeline,
                                            const RimWellPath&                   well,
                                            const QDateTime&                     date,
                                            std::map<QString, Opm::DeckKeyword>& keywordBlocks );

    // Append records of `kw` into the entry for `name`, creating it from `kw` if absent
    static void mergeKeyword( std::map<QString, Opm::DeckKeyword>& acc, const QString& name, Opm::DeckKeyword kw );
};
