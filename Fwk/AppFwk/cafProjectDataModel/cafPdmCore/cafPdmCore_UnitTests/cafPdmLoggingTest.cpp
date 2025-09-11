//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2025 Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "gtest/gtest.h"

#include "cafPdmLogging.h"

#include <vector>

//==================================================================================================
/// Test logger implementation that captures log messages for verification
//==================================================================================================
class TestLogger : public caf::PdmLogger
{
public:
    TestLogger( int logLevel = static_cast<int>( caf::PdmLogLevel::PDM_LL_DEBUG ) )
        : m_logLevel( logLevel )
    {
    }

    // PdmLogger interface
    int  level() const override { return m_logLevel; }
    void setLevel( int logLevel ) override { m_logLevel = logLevel; }

    void error( const QString& message ) override { m_errorMessages.push_back( message ); }
    void warning( const QString& message ) override { m_warningMessages.push_back( message ); }
    void info( const QString& message ) override { m_infoMessages.push_back( message ); }
    void debug( const QString& message ) override { m_debugMessages.push_back( message ); }

    // Test utilities
    void clearMessages()
    {
        m_errorMessages.clear();
        m_warningMessages.clear();
        m_infoMessages.clear();
        m_debugMessages.clear();
    }

    size_t errorCount() const { return m_errorMessages.size(); }
    size_t warningCount() const { return m_warningMessages.size(); }
    size_t infoCount() const { return m_infoMessages.size(); }
    size_t debugCount() const { return m_debugMessages.size(); }

    const std::vector<QString>& errorMessages() const { return m_errorMessages; }
    const std::vector<QString>& warningMessages() const { return m_warningMessages; }
    const std::vector<QString>& infoMessages() const { return m_infoMessages; }
    const std::vector<QString>& debugMessages() const { return m_debugMessages; }

private:
    int                  m_logLevel;
    std::vector<QString> m_errorMessages;
    std::vector<QString> m_warningMessages;
    std::vector<QString> m_infoMessages;
    std::vector<QString> m_debugMessages;
};

//==================================================================================================
/// Test fixture for PdmLogging tests
//==================================================================================================
class PdmLoggingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clear all loggers before each test
        caf::PdmLogging::clearAllLoggers();

        // Create test loggers
        m_testLogger1 = std::make_shared<TestLogger>( static_cast<int>( caf::PdmLogLevel::PDM_LL_DEBUG ) );
        m_testLogger2 = std::make_shared<TestLogger>( static_cast<int>( caf::PdmLogLevel::PDM_LL_INFO ) );
        m_testLogger3 = std::make_shared<TestLogger>( static_cast<int>( caf::PdmLogLevel::PDM_LL_ERROR ) );
    }

    void TearDown() override
    {
        // Clean up after each test
        caf::PdmLogging::clearAllLoggers();
    }

    std::shared_ptr<TestLogger> m_testLogger1; // Debug level
    std::shared_ptr<TestLogger> m_testLogger2; // Info level
    std::shared_ptr<TestLogger> m_testLogger3; // Error level only
};

//--------------------------------------------------------------------------------------------------
/// Test basic logger registration and unregistration
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, LoggerRegistration )
{
    // Initially no loggers
    EXPECT_FALSE( caf::PdmLogging::hasLoggers() );
    EXPECT_EQ( 0, caf::PdmLogging::effectiveLogLevel() );

    // Register first logger
    caf::PdmLogging::registerLogger( m_testLogger1 );
    EXPECT_TRUE( caf::PdmLogging::hasLoggers() );
    EXPECT_EQ( static_cast<int>( caf::PdmLogLevel::PDM_LL_DEBUG ), caf::PdmLogging::effectiveLogLevel() );

    // Register second logger
    caf::PdmLogging::registerLogger( m_testLogger2 );
    EXPECT_TRUE( caf::PdmLogging::hasLoggers() );
    EXPECT_EQ( static_cast<int>( caf::PdmLogLevel::PDM_LL_DEBUG ), caf::PdmLogging::effectiveLogLevel() ); // Higher level

    // Unregister first logger
    caf::PdmLogging::unregisterLogger( m_testLogger1 );
    EXPECT_TRUE( caf::PdmLogging::hasLoggers() );
    EXPECT_EQ( static_cast<int>( caf::PdmLogLevel::PDM_LL_INFO ), caf::PdmLogging::effectiveLogLevel() );

    // Unregister second logger
    caf::PdmLogging::unregisterLogger( m_testLogger2 );
    EXPECT_FALSE( caf::PdmLogging::hasLoggers() );
    EXPECT_EQ( 0, caf::PdmLogging::effectiveLogLevel() );
}

//--------------------------------------------------------------------------------------------------
/// Test duplicate logger registration handling
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, DuplicateRegistration )
{
    // Register same logger twice
    caf::PdmLogging::registerLogger( m_testLogger1 );
    caf::PdmLogging::registerLogger( m_testLogger1 );

    // Should still be registered only once
    caf::PdmLogging::error( "Test message" );
    EXPECT_EQ( 1, m_testLogger1->errorCount() );
}

//--------------------------------------------------------------------------------------------------
/// Test null logger handling
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, NullLoggerHandling )
{
    // Register null logger - should be ignored
    caf::PdmLogging::registerLogger( nullptr );
    EXPECT_FALSE( caf::PdmLogging::hasLoggers() );

    // Unregister null logger - should not crash
    caf::PdmLogging::unregisterLogger( nullptr );
    EXPECT_FALSE( caf::PdmLogging::hasLoggers() );
}

//--------------------------------------------------------------------------------------------------
/// Test basic message logging
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, BasicMessageLogging )
{
    caf::PdmLogging::registerLogger( m_testLogger1 );

    // Test all log levels
    caf::PdmLogging::error( "Error message" );
    caf::PdmLogging::warning( "Warning message" );
    caf::PdmLogging::info( "Info message" );
    caf::PdmLogging::debug( "Debug message" );

    // Verify messages were captured
    EXPECT_EQ( 1, m_testLogger1->errorCount() );
    EXPECT_EQ( 1, m_testLogger1->warningCount() );
    EXPECT_EQ( 1, m_testLogger1->infoCount() );
    EXPECT_EQ( 1, m_testLogger1->debugCount() );

    EXPECT_EQ( "Error message", m_testLogger1->errorMessages()[0] );
    EXPECT_EQ( "Warning message", m_testLogger1->warningMessages()[0] );
    EXPECT_EQ( "Info message", m_testLogger1->infoMessages()[0] );
    EXPECT_EQ( "Debug message", m_testLogger1->debugMessages()[0] );
}

//--------------------------------------------------------------------------------------------------
/// Test log level filtering
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, LogLevelFiltering )
{
    // Register logger with INFO level (should not receive DEBUG messages)
    caf::PdmLogging::registerLogger( m_testLogger2 );

    caf::PdmLogging::error( "Error message" );
    caf::PdmLogging::warning( "Warning message" );
    caf::PdmLogging::info( "Info message" );
    caf::PdmLogging::debug( "Debug message" );

    // Should receive error, warning, info but not debug
    EXPECT_EQ( 1, m_testLogger2->errorCount() );
    EXPECT_EQ( 1, m_testLogger2->warningCount() );
    EXPECT_EQ( 1, m_testLogger2->infoCount() );
    EXPECT_EQ( 0, m_testLogger2->debugCount() );
}

//--------------------------------------------------------------------------------------------------
/// Test multiple logger handling
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, MultipleLoggers )
{
    // Register multiple loggers with different levels
    caf::PdmLogging::registerLogger( m_testLogger1 ); // Debug level
    caf::PdmLogging::registerLogger( m_testLogger2 ); // Info level
    caf::PdmLogging::registerLogger( m_testLogger3 ); // Error level

    caf::PdmLogging::error( "Error message" );
    caf::PdmLogging::warning( "Warning message" );
    caf::PdmLogging::info( "Info message" );
    caf::PdmLogging::debug( "Debug message" );

    // Error message should reach all loggers
    EXPECT_EQ( 1, m_testLogger1->errorCount() );
    EXPECT_EQ( 1, m_testLogger2->errorCount() );
    EXPECT_EQ( 1, m_testLogger3->errorCount() );

    // Warning should reach debug and info loggers, but not error-only logger
    EXPECT_EQ( 1, m_testLogger1->warningCount() );
    EXPECT_EQ( 1, m_testLogger2->warningCount() );
    EXPECT_EQ( 0, m_testLogger3->warningCount() );

    // Debug should only reach debug logger
    EXPECT_EQ( 1, m_testLogger1->debugCount() );
    EXPECT_EQ( 0, m_testLogger2->debugCount() );
    EXPECT_EQ( 0, m_testLogger3->debugCount() );
}

//--------------------------------------------------------------------------------------------------
/// Test clearAllLoggers functionality
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, ClearAllLoggers )
{
    // Register multiple loggers
    caf::PdmLogging::registerLogger( m_testLogger1 );
    caf::PdmLogging::registerLogger( m_testLogger2 );
    EXPECT_TRUE( caf::PdmLogging::hasLoggers() );

    // Clear all loggers
    caf::PdmLogging::clearAllLoggers();
    EXPECT_FALSE( caf::PdmLogging::hasLoggers() );

    // Logging should not reach any logger now
    caf::PdmLogging::error( "Error message" );
    EXPECT_EQ( 0, m_testLogger1->errorCount() );
    EXPECT_EQ( 0, m_testLogger2->errorCount() );
}

//--------------------------------------------------------------------------------------------------
/// Test logging macros
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, LoggingMacros )
{
    caf::PdmLogging::registerLogger( m_testLogger1 );

    // Test standard macros
    CAF_PDM_LOG_ERROR( "Macro error" );
    CAF_PDM_LOG_WARNING( "Macro warning" );
    CAF_PDM_LOG_INFO( "Macro info" );
    CAF_PDM_LOG_DEBUG( "Macro debug" );

    EXPECT_EQ( 1, m_testLogger1->errorCount() );
    EXPECT_EQ( 1, m_testLogger1->warningCount() );
    EXPECT_EQ( 1, m_testLogger1->infoCount() );
    EXPECT_EQ( 1, m_testLogger1->debugCount() );

    EXPECT_EQ( "Macro error", m_testLogger1->errorMessages()[0] );
    EXPECT_EQ( "Macro warning", m_testLogger1->warningMessages()[0] );
    EXPECT_EQ( "Macro info", m_testLogger1->infoMessages()[0] );
    EXPECT_EQ( "Macro debug", m_testLogger1->debugMessages()[0] );
}

//--------------------------------------------------------------------------------------------------
/// Test conditional logging macros
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, ConditionalLoggingMacros )
{
    // Test conditional macros with no loggers registered
    CAF_PDM_LOG_ERROR_IF( "Conditional error" );
    CAF_PDM_LOG_WARNING_IF( "Conditional warning" );
    CAF_PDM_LOG_INFO_IF( "Conditional info" );
    CAF_PDM_LOG_DEBUG_IF( "Conditional debug" );

    // No messages should be logged since no loggers registered
    EXPECT_EQ( 0, m_testLogger1->errorCount() );

    // Register logger and test again
    caf::PdmLogging::registerLogger( m_testLogger1 );

    CAF_PDM_LOG_ERROR_IF( "Conditional error" );
    CAF_PDM_LOG_WARNING_IF( "Conditional warning" );
    CAF_PDM_LOG_INFO_IF( "Conditional info" );
    CAF_PDM_LOG_DEBUG_IF( "Conditional debug" );

    // Now messages should be logged
    EXPECT_EQ( 1, m_testLogger1->errorCount() );
    EXPECT_EQ( 1, m_testLogger1->warningCount() );
    EXPECT_EQ( 1, m_testLogger1->infoCount() );
    EXPECT_EQ( 1, m_testLogger1->debugCount() );
}

//--------------------------------------------------------------------------------------------------
/// Test thread safety (basic test)
//--------------------------------------------------------------------------------------------------
TEST_F( PdmLoggingTest, ThreadSafety )
{
    caf::PdmLogging::registerLogger( m_testLogger1 );

    // This is a basic test - in a real scenario, you'd use multiple threads
    // Here we just verify that multiple rapid operations don't cause issues
    for ( int i = 0; i < 100; ++i )
    {
        caf::PdmLogging::error( QString( "Error %1" ).arg( i ) );
        caf::PdmLogging::registerLogger( m_testLogger2 );
        caf::PdmLogging::unregisterLogger( m_testLogger2 );
    }

    EXPECT_EQ( 100, m_testLogger1->errorCount() );
    EXPECT_TRUE( caf::PdmLogging::hasLoggers() );
}
