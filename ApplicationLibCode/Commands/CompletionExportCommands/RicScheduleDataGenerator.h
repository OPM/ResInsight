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
    // includeWelsegs / includeCompsegs gate the corresponding multi-segment-well keywords;
    // WSEGVALV and WSEGAICD remain unaffected.
    static QString generateSchedule( const RimWellEventTimeline&      timeline,
                                     RimEclipseCase&                  eclipseCase,
                                     const std::vector<RimWellPath*>& wellPaths,
                                     const std::vector<QDateTime>&    dates,
                                     bool                             includeWelsegs  = true,
                                     bool                             includeCompsegs = true );

    // Collect all unique dates from all wells' timelines
    static std::vector<QDateTime> collectAllDates( const RimWellEventTimeline& timeline, const std::vector<RimWellPath*>& wellPaths );

private:
    // Generate schedule section for a single date
    static QString generateDateSection( const RimWellEventTimeline&      timeline,
                                        RimEclipseCase&                  eclipseCase,
                                        const std::vector<RimWellPath*>& wellPaths,
                                        const QDateTime&                 date,
                                        bool                             includeWelsegs,
                                        bool                             includeCompsegs );

    static std::optional<Opm::DeckKeyword>
        generateWelspecsForWell( const RimWellEventTimeline& timeline, RimEclipseCase& eclipseCase, RimWellPath& well, const QDateTime& date );

    // Generate COMPDAT for a well at a specific date based on events
    static std::optional<Opm::DeckKeyword>
        generateCompdatForWell( const RimWellEventTimeline& timeline, RimEclipseCase& eclipseCase, RimWellPath& well, const QDateTime& date );

    // Generate WELSEGS / COMPSEGS / WSEGVALV / WSEGAICD for a well at a specific date, merging into the accumulator.
    // includeWelsegs/includeCompsegs suppress only those two keywords; WSEGVALV/WSEGAICD always emit.
    static void generateMswForWell( const RimWellEventTimeline&          timeline,
                                    RimEclipseCase&                      eclipseCase,
                                    RimWellPath&                         well,
                                    const QDateTime&                     date,
                                    std::map<QString, Opm::DeckKeyword>& keywordBlocks,
                                    bool                                 includeWelsegs,
                                    bool                                 includeCompsegs );

    // Generate well control / well keyword event keywords for a well at a specific date, merging into the accumulator
    static void generateWellControlForWell( const RimWellEventTimeline&          timeline,
                                            const RimWellPath&                   well,
                                            const QDateTime&                     date,
                                            std::map<QString, Opm::DeckKeyword>& keywordBlocks );

    // Append records of `kw` into the entry for `name`, creating it from `kw` if absent
    static void mergeKeyword( std::map<QString, Opm::DeckKeyword>& acc, const QString& name, Opm::DeckKeyword kw );
};
