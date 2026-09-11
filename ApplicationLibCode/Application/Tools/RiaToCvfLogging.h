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

// Forwards the cvf framework's own logging (cee.cvf, cee.cvf.qt, cee.cvf.OpenGL) to ResInsight's
// RiaLogging system, so messages end up in the log file and the message panel alongside the rest of
// the application's log output. Without this, cvf logs to a console destination nobody reads.
//
// How to use:
// 1. Enable the "CVF Logging" experimental feature (keyword "cvf-logging") in
//    Preferences -> System -> Experimental Features, or add "cvf-logging" to the legacy
//    "Keywords to enable experimental features" field, and restart ResInsight. This is required
//    because RiaCvfLoggingManager::initializeCvfLogging()/shutdownCvfLogging() are only called
//    when RiaPreferencesSystem::isFeatureEnabled("cvf-logging") returns true (see RiaApplication.cpp).
// 2. Start ResInsight with debug-level logging, e.g. `ResInsight --loglevel debug`, or enable it via
//    preferences, to actually see cvf's debug output; at the default INFO level only cvf
//    info/warning/error messages are forwarded.
// 3. Look for lines prefixed "cvf[<logger-name>]: " in the message panel/log file, e.g.
//    "cvf[cee.cvf.qt]: OpenGLWidget[0]::initializeGL()".
// Note: at debug level, cvf::Rendering/cvf::RenderEngine log one message per rendering pass and one
// more per part in that pass, every frame, so the log can grow very quickly. Disable the feature
// again (and restart) once done troubleshooting.

#include "cvfLogDestination.h"

//==================================================================================================
/// Bridge log destination that connects cvf's logging framework to ResInsight's RiaLogging system
/// This class implements the cvf::LogDestination interface and forwards all messages
/// to ResInsight's logging infrastructure
//==================================================================================================
class RiaToCvfLoggingBridge : public cvf::LogDestination
{
public:
    void log( const cvf::LogEvent& logEvent ) override;
};

//==================================================================================================
/// Utility class for managing cvf logging integration lifecycle
//==================================================================================================
class RiaCvfLoggingManager
{
public:
    /// Initialize cvf logging integration - should be called during application startup
    static void initializeCvfLogging();

    /// Cleanup cvf logging integration - should be called during application shutdown
    static void shutdownCvfLogging();

    /// Check if cvf logging is currently active
    static bool isCvfLoggingActive();

private:
    static bool s_isInitialized;
};
