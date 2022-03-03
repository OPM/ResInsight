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
#include "RiaRegressionTestRunner.h"
#include "RiaSocketCommand.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiFieldEditorHandle.h"

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
