/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "cafPdmLogging.h"

#include <memory>

//==================================================================================================
/// Bridge logger that connects CAF PDM logging to ResInsight's RiaLogging system
/// This class implements the caf::PdmLogger interface and forwards all messages
/// to ResInsight's logging infrastructure
//==================================================================================================
class RiaToCafLoggingBridge : public caf::PdmLogger
{
public:
    RiaToCafLoggingBridge();
    ~RiaToCafLoggingBridge() override = default;

    // PdmLogger interface implementation
    int  level() const override;
    void setLevel( int logLevel ) override;

    void error( const QString& message ) override;
    void warning( const QString& message ) override;
    void info( const QString& message ) override;
    void debug( const QString& message ) override;

    // Static utility methods for easy integration
    static void                                   registerWithCafLogging();
    static void                                   unregisterFromCafLogging();
    static std::shared_ptr<RiaToCafLoggingBridge> instance();

private:
    int                                           m_logLevel;
    static std::shared_ptr<RiaToCafLoggingBridge> s_instance;
};

//==================================================================================================
/// Utility class for managing CAF logging integration lifecycle
//==================================================================================================
class RiaCafLoggingManager
{
public:
    /// Initialize CAF logging integration - should be called during application startup
    static void initializeCafLogging();

    /// Cleanup CAF logging integration - should be called during application shutdown
    static void shutdownCafLogging();

    /// Check if CAF logging is currently active
    static bool isCafLoggingActive();

private:
    static bool s_isInitialized;
};