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

#include "RiaExperimentalFeatures.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RiaExperimentalFeatures::Feature>& RiaExperimentalFeatures::availableFeatures()
{
    // Register experimental features here. Each entry automatically becomes a checkbox in
    // Preferences -> System -> Experimental Features. Gate the related code path behind
    // RiaPreferencesSystem::isFeatureEnabled( "<keyword>" ).
    //
    // Example:
    //   { "my-feature", "My Feature", "Short description of what it does." },
    static const std::vector<Feature> features = {
        { "osdu-well-logs", "OSDU Well Logs", "Enable import of well logs from OSDU." },
        { "undo-redo-view", "Undo/Redo View", "Show the command undo/redo history view." },
        { "export-dock-layout", "Export Dock Layout", "Add a window menu action to export the dock layout to the clipboard." },
        { "oil-volume-result", "Oil Volume Result", "Compute the derived oil volume cell result." },
    };

    return features;
}
