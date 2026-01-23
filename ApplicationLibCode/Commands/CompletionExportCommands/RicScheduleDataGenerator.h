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

#include <vector>

class RimEclipseCase;
class RimWellPath;
class RimWellEvent;
class RimWellEventTimeline;

//==================================================================================================
///
/// Generator for Eclipse schedule data based on well events
///
//==================================================================================================
class RicScheduleDataGenerator
{
public:
    // Generate schedule for multiple wells at specified dates
    static QString generateSchedule( const RimWellEventTimeline&      timeline,
                                     RimEclipseCase&                  eclipseCase,
                                     const std::vector<RimWellPath*>& wellPaths,
                                     const std::vector<QDateTime>&    dates );

    // Collect all unique dates from all wells' timelines
    static std::vector<QDateTime> collectAllDates( const RimWellEventTimeline& timeline, const std::vector<RimWellPath*>& wellPaths );

private:
    // Generate schedule section for a single date
    static QString generateDateSection( const RimWellEventTimeline&      timeline,
                                        RimEclipseCase&                  eclipseCase,
                                        const std::vector<RimWellPath*>& wellPaths,
                                        const QDateTime&                 date );

    // Generate COMPDAT for a well at a specific date based on events
    static QString
        generateCompdatForWell( const RimWellEventTimeline& timeline, RimEclipseCase& eclipseCase, RimWellPath& well, const QDateTime& date );

    // Generate WELSEGS and COMPSEGS for a well at a specific date
    static QString
        generateMswForWell( const RimWellEventTimeline& timeline, RimEclipseCase& eclipseCase, RimWellPath& well, const QDateTime& date );

    // Generate well control keywords (WCONPROD, WCONINJE, WELOPEN) for a well at a specific date
    static QString generateWellControlForWell( const RimWellEventTimeline& timeline, const RimWellPath& well, const QDateTime& date );
};
