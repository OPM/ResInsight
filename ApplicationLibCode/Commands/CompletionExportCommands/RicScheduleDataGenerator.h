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

//==================================================================================================
///
/// Generator for Eclipse schedule data based on well events
///
//==================================================================================================
class RicScheduleDataGenerator
{
public:
    struct Options
    {
        bool includeMsw         = true;
        bool includeCompdat     = true;
        bool includeWellControl = true;
        bool includeComments    = true;
    };

    // Generate schedule for multiple wells at specified dates
    static QString generateSchedule( RimEclipseCase*                    eclipseCase,
                                     const std::vector<RimWellPath*>&   wellPaths,
                                     const std::vector<QDateTime>&      dates,
                                     const Options&                     options );

    // Collect all unique dates from all wells' timelines
    static std::vector<QDateTime> collectAllDates( const std::vector<RimWellPath*>& wellPaths );

private:
    // Generate schedule section for a single date
    static QString generateDateSection( RimEclipseCase*                    eclipseCase,
                                        const std::vector<RimWellPath*>&   wellPaths,
                                        const QDateTime&                   date,
                                        const Options&                     options );

    // Generate DATES keyword
    static QString generateDatesKeyword( const QDateTime& date );

    // Generate COMPDAT for a well at a specific date based on events
    static QString generateCompdatForWell( RimEclipseCase* eclipseCase, RimWellPath* well, const QDateTime& date );

    // Generate WELSEGS and COMPSEGS for a well at a specific date
    static QString generateMswForWell( RimEclipseCase* eclipseCase, RimWellPath* well, const QDateTime& date );

    // Generate well control keywords (WCONPROD, WCONINJE, WELOPEN) for a well at a specific date
    static QString generateWellControlForWell( RimWellPath* well, const QDateTime& date );

    // Get events at or before a specific date for a well
    static std::vector<RimWellEvent*> getActiveEventsAtDate( RimWellPath* well, const QDateTime& date );
};
