/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cafCmdFeature.h"

#include <QString>

#include <expected>

class RimEclipseCase;
class RimWellEventTimeline;

//==================================================================================================
/// Export an Eclipse schedule generated from the well event timeline.
///
/// The file name is the only thing asked for; everything else uses fixed defaults (see
/// exportScheduleToFile).
//==================================================================================================
class RicExportScheduleFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    // Generate the schedule for the timeline and write it to fileName. MSW keywords are exported
    // for every well with events, the first (simulation start) date is written as a comment
    // instead of a DATES keyword, and keywords are emitted with aligned columns.
    // Returns the written file name, or an error message.
    static std::expected<QString, QString>
        exportScheduleToFile( const RimWellEventTimeline& timeline, RimEclipseCase& eclipseCase, const QString& fileName );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    // The timeline to export: the selected timeline, the one of a selected well path collection,
    // or - when the selection has neither, e.g. when the feature is invoked from the File menu -
    // the timeline of the project's well path collection. Returns nullptr when no timeline has
    // events.
    static RimWellEventTimeline* timelineToExport();

    // The Eclipse case of the active view, or the first Eclipse case of the project.
    static RimEclipseCase* defaultEclipseCase();
};
