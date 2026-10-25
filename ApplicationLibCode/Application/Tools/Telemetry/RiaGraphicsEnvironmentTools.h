/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

//==================================================================================================
/// Collects graphics environment information for telemetry, used to correlate rendering crashes
/// (e.g. aborts inside the GL driver) with driver, display server and remote desktop setup.
//==================================================================================================
namespace RiaGraphicsEnvironmentTools
{
// Display server and environment information. Requires a QGuiApplication, but no OpenGL context.
std::map<std::string, std::string> displayEnvironment();

// Vendor, renderer and version strings of the current OpenGL context. Returns an empty map if no context is current.
std::map<std::string, std::string> currentOpenGLContextInfo();
} // namespace RiaGraphicsEnvironmentTools
