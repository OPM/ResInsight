/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include <map>
#include <string>

//--------------------------------------------------------------------------------------------------
/// Command ids that the execution sweep must not trigger via onActionTriggered().
///
/// The sweep protects against modal dialogs with a watchdog that closes any that appear, so the list
/// below focuses on features that block or cause side effects the watchdog cannot handle: process or
/// application termination, launching external processes, blocking network calls, and recursively
/// invoking the test/regression machinery. Modal file/preferences dialogs are included as well to
/// avoid the per-call watchdog delay. Every entry has a reason.
//--------------------------------------------------------------------------------------------------
inline const std::map<std::string, std::string>& featureSweepDenylist()
{
    static const std::map<std::string, std::string> deny = {
        // Application / process lifecycle
        { "RicExitApplicationFeature", "quits the application, terminating the test process" },
        { "RicCloseProjectFeature", "tears down the project mid-sweep; covered by curated tests" },
        { "RicOpenProjectFeature", "opens a QFileDialog to pick a project file" },
        { "RicOpenLastUsedFileFeature", "loads an arbitrary file from user settings" },
        { "RicSaveProjectFeature", "may open a save QFileDialog and writes to disk" },
        { "RicSaveProjectAsFeature", "opens a save QFileDialog" },
        { "RicSaveProjectNoGlobalPathsFeature", "opens a save QFileDialog" },

        // External processes
        { "RicExecuteScriptFeature", "launches an external Octave/Python process" },
        { "RicExecuteScriptForCasesFeature", "launches an external process" },
        { "RicNewOctaveScriptFeature", "opens an external script editor process" },
        { "RicNewPythonScriptFeature", "opens an external script editor process" },
        { "RicOpenInTextEditorFeature", "launches an external text editor process" },

        // Blocking network access
        { "RicHoloLensCreateSessionFeature", "opens a network session to the sharing server" },
        { "RicHoloLensCreateDummyFiledBackedSessionFeature", "starts a HoloLens session" },
        { "RicHoloLensExportToSharingServerFeature", "uploads over the network" },
        { "RicHoloLensAutoExportToSharingServerFeature", "uploads over the network" },
        { "RicHoloLensExportToFolderFeature", "opens a folder dialog and exports" },
        { "RicHoloLensTerminateSessionFeature", "acts on a live network session" },
        { "RicDeleteOsduTokenFeature", "modifies stored OSDU credentials" },
        { "RicDeleteSumoTokenFeature", "modifies stored Sumo credentials" },

        // Recursive test / regression machinery
        { "RicLaunchRegressionTestsFeature", "recursively runs the regression-test suite" },
        { "RicLaunchRegressionTestDialogFeature", "opens the regression-test dialog" },

        // Modal editors and preference dialogs
        { "RicEditPreferencesFeature", "opens the modal preferences dialog" },
        { "RicThemeColorEditorFeature", "opens a modal color-editor dialog" },
        { "RicShowMemoryCleanupDialogFeature", "opens a modal memory-cleanup dialog" },

        // Requires a rendering 3D viewer. The offscreen platform plugin creates the viewer widget but
        // no OpenGL context, so drawing dereferences a null context in caf::Viewer::paintGL(). These
        // work in the real application and belong in a software-OpenGL test tier.
        { "RicTogglePerspectiveViewFeature", "changes the projection of the active viewer, which needs an OpenGL context" },
        { "RicNewContourMapViewFeature", "creates and draws a new 3D view, which needs an OpenGL context" },
    };
    return deny;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
inline bool isFeatureDenylisted( const std::string& commandId )
{
    return featureSweepDenylist().find( commandId ) != featureSweepDenylist().end();
}
