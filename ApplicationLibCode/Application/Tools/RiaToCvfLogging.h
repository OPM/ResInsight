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

// TEMPORARY investigation logging for the "black 3D view" issue (#14714). Forwards the cvf
// framework's own logging (cee.cvf.qt, cee.cvf.render, ...) to ResInsight's RiaLogging system, so
// messages end up in ~/.resinsight/logs and the message panel alongside the rest of the
// application's log output.
//
// Not intended to be merged - remove once the investigation concludes.

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
