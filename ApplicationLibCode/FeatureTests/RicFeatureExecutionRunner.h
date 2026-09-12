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

#include <string>
#include <vector>

//==================================================================================================
/// Executes a single Ric*Feature against synthetic selections.
///
/// Shared by two callers:
/// - the in-process sweep (RicFeatureExecutionSweep-Test.cpp), which runs every feature in this
///   process and is therefore killed by the first feature that aborts,
/// - the subprocess sweep (RicFeatureSubprocessSweep-Test.cpp), which re-runs this executable once
///   per feature via the --feature-exec command line switch, so an abort or hang only takes down
///   that one child process.
//==================================================================================================
namespace RicFeatureExecutionRunner
{
// Command line switch used to run a single feature and exit; see main.cpp.
constexpr const char* featureExecSwitch = "--feature-exec=";

// Printed by the child once the feature has been executed under every applicable scenario, before
// any harness teardown. Lets the parent tell a crash inside the feature (marker absent) from a crash
// while closing the project afterwards (marker present).
constexpr const char* executionCompleteMarker = "[feature-execution-complete]";

// Execute the feature under every scenario where it reports itself enabled.
//
// Returns false only for conditions the caller can meaningfully report: an unknown command id, or a
// feature that left the project in an invalid state. A feature that aborts (CVF_ASSERT / CAF_ASSERT)
// or hangs never returns at all - that is precisely what subprocess isolation is for. Denylisted
// features return true without being executed.
bool executeFeature( const std::string& commandId );

// The command ids of all registered Ric*Feature commands. Shared by both sweeps: defining it in each
// of them breaks the unity build, where the two files end up in one translation unit.
std::vector<std::string> allRegisteredFeatureIds();

// Drop the shared model and clear the selection. Call when a suite of features has been executed.
void releaseModel();
} // namespace RicFeatureExecutionRunner
