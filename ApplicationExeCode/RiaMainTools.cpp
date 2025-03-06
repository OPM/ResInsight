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
#include "RiaFileLogger.h"
#include "RiaLogging.h"
#include "RiaRegressionTestRunner.h"
#include "RiaSocketCommand.h"
#include "RiaBaseDefs.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void manageSegFailure( int signalCode )
{
    // Executing function here is not safe, but works as expected on Windows. Behavior on Linux is undefined, but will
    // work in some cases.
    // https://github.com/gabime/spdlog/issues/1607

    auto loggers = RiaLogging::loggerInstances();

    QString str = QString( "Segmentation fault. Signal code: %1" ).arg( signalCode );

    for ( auto logger : loggers )
    {
        if ( auto fileLogger = dynamic_cast<RiaFileLogger*>( logger ) )
        {
            fileLogger->error( str.toStdString().data() );

            fileLogger->flush();
        }
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
///
//--------------------------------------------------------------------------------------------------
void removeSettingsLockFiles()
{
      // Get the organization and application name
        auto organizationName = QString(RI_COMPANY_NAME);
        auto applicationName = QString(RI_APPLICATION_NAME);

          auto lockFilePath = QDir::homePath() + "/.config/" + organizationName + "/" + applicationName + ".conf.lock";
      
    //  lockFilePath = "/home/builder/.config/Ceetron/ResInsight.conf.lock";
    
    auto isLockStale = []( const QString& lockFilePath ) -> bool
    {
        QFileInfo lockFileInfo( lockFilePath );

        // If the lock file doesn't exist, it's not stale
        if ( !lockFileInfo.exists() ) return false;

        int thresholdSeconds = 1;//10 * 60; // 10 minutes

        QDateTime currentTime = QDateTime::currentDateTime();

        auto logMessage = QString( "Seconds since lock file last modified: %1" ).arg(lockFileInfo.lastModified().secsTo( currentTime ) );
        qDebug()<< logMessage;
        RiaLogging::warning( logMessage );

        return lockFileInfo.lastModified().secsTo( currentTime ) > thresholdSeconds;
    };

//    QSettings mySettings;
//    QString   settingsPath = mySettings.fileName();
//    QString   lockFilePath = settingsPath + ".lock";



    if ( isLockStale( lockFilePath ) )
    {
        QFile lockFile( lockFilePath );
        lockFile.remove();

        QString logMessage = QString( "Removed stale lock file: %1" ).arg( lockFilePath );
        qDebug()<< logMessage;
        RiaLogging::warning( logMessage );
    }
    else
    {
        QString logMessage = QString( "No lock files present" );
        qDebug()<< logMessage;
        RiaLogging::info( logMessage);
    }
}

} // namespace RiaMainTools
