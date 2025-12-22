/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RiaMainTools.h"
#include "RiaBaseDefs.h"
#include "RiaFileLogger.h"
#include "RiaLogging.h"
#include "RiaOpenTelemetryManager.h"
#include "RiaRegressionTestRunner.h"
#include "RiaSocketCommand.h"
#include "RiaVersionInfo.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <QDir>

#include <stacktrace>

namespace internal
{
// Custom formatter for stacktrace
std::string formatStacktrace( const std::stacktrace& st )
{
    std::stringstream ss;
    int               frame = 0;
    for ( const auto& entry : st )
    {
        ss << "  [" << frame++ << "] " << entry.description() << " at " << entry.source_file() << ":"
           << entry.source_line() << "\n";
    }
    return ss.str();
}
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void manageSegFailure( int signalCode )
{
    // Executing function here is not safe, but works as expected on Windows. Behavior on Linux is undefined, but will
    // work in some cases.
    // https://github.com/gabime/spdlog/issues/1607

    auto loggers = RiaLogging::loggerInstances();

    for ( auto logger : loggers )
    {
        if ( auto fileLogger = dynamic_cast<RiaFileLogger*>( logger ) )
        {
            auto versionText = QString( STRPRODUCTVER );
            auto str =
                QString( "Segmentation fault (signal code: %1) - ResInsight version %2" ).arg( signalCode ).arg( versionText );

            fileLogger->error( str.toStdString().data() );

            auto        st      = std::stacktrace::current();
            std::string message = "Stack trace:\n" + internal::formatStacktrace( st );
            logger->error( message.data() );

            fileLogger->flush();
        }
    }

    // Report crash to OpenTelemetry if enabled and initialized
    auto& otelManager = RiaOpenTelemetryManager::instance();
    if ( otelManager.isEnabled() )
    {
        auto st = std::stacktrace::current();
        otelManager.reportCrash( signalCode, st );
    }

    exit( 1 );
}

namespace RiaMainTools
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void initializeSingletons()
{
    caf::CmdFeatureManager::createSingleton();
    RiaRegressionTestRunner::createSingleton();
    caf::PdmDefaultObjectFactory::createSingleton();
}

//--------------------------------------------------------------------------------------------------
/// This method is used to release memory allocated by static functions. This enables use of memory allocation tools
/// after the application has closed down.
//--------------------------------------------------------------------------------------------------
void releaseSingletonAndFactoryObjects()
{
    caf::CmdFeatureManager::deleteSingleton();
    RiaRegressionTestRunner::deleteSingleton();
    caf::PdmDefaultObjectFactory::deleteSingleton();

    {
        auto factory = caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance();
        factory->deleteCreatorObjects();
    }

    {
        auto factory = caf::Factory<caf::CmdFeature, std::string>::instance();
        factory->deleteCreatorObjects();
    }
    {
        auto factory = caf::Factory<RiaSocketCommand, QString>::instance();
        factory->deleteCreatorObjects();
    }
}

//--------------------------------------------------------------------------------------------------
/// On Linux, application settings are stored in a text file in the users home folder. On Windows, these settings are
/// stored in Registry. Users have reported stale lock files the configuration directory. In some cases, these lock
/// files can prevent the application from starting. It appears that the application start, but no GUI is displayed.
///
/// This method deletes stale lock files.
///
/// https://github.com/OPM/ResInsight/issues/12205
//--------------------------------------------------------------------------------------------------
void deleteStaleSettingsLockFiles()
{
    auto organizationName = QString( RI_COMPANY_NAME );
    auto applicationName  = QString( RI_APPLICATION_NAME );

    auto lockFilePath = QDir::homePath() + "/.config/" + organizationName + "/" + applicationName + ".conf.lock";

    auto isLockStale = []( const QString& lockFilePath ) -> bool
    {
        QFileInfo lockFileInfo( lockFilePath );

        if ( !lockFileInfo.exists() ) return false;

        QDateTime currentTime      = QDateTime::currentDateTime();
        int       thresholdSeconds = 2 * 60; // 2 minutes

        return lockFileInfo.lastModified().secsTo( currentTime ) > thresholdSeconds;
    };

    if ( isLockStale( lockFilePath ) )
    {
        QFile lockFile( lockFilePath );

        QString logMessage;
        if ( lockFile.remove() )
        {
            logMessage = QString( "Deleted stale lock file: %1" ).arg( lockFilePath );
        }
        else
        {
            logMessage = QString( "Tried, but failed to delete stale lock file: %1" ).arg( lockFilePath );
        }

        qDebug() << logMessage;
    }
}

} // namespace RiaMainTools
