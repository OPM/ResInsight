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

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiFieldEditorHandle.h"

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaMainTools::initializeSingletons()
{
    caf::CmdFeatureManager::createSingleton();
    RiaRegressionTestRunner::createSingleton();
    caf::PdmDefaultObjectFactory::createSingleton();
}

//--------------------------------------------------------------------------------------------------
/// This method is used to release memory allocated by static functions. This enables use of memory allocation tools
/// after the application has closed down.
//--------------------------------------------------------------------------------------------------
void RiaMainTools::releaseSingletonAndFactoryObjects()
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
