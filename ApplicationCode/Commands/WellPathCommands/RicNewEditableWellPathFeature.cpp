/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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
#include "RicNewEditableWellPathFeature.h"

CAF_CMD_SOURCE_INIT(RicNewEditableWellPathFeature, "RicNewEditableWellPathFeature");

#include "cafSelectionManager.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RiaApplication.h"
#include "RimProject.h"
#include "RimModeledWellPath.h"
#include <QAction>
#include "RimOilField.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewEditableWellPathFeature::isCommandEnabled()
{
    {
        std::vector<RimWellPath*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if ( objects.size() > 0 )
        {
            return true;
        }
    }
    {
        std::vector<RimWellPathCollection*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if ( objects.size() > 0 )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewEditableWellPathFeature::onActionTriggered(bool isChecked)
{
    if ( RiaApplication::instance()->project() && RiaApplication::instance()->project()->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = RiaApplication::instance()->project()->activeOilField()->wellPathCollection();

        if ( wellPathCollection )
        {
            std::vector<RimWellPath*> newWellPaths;
            newWellPaths.push_back(new RimModeledWellPath());
            wellPathCollection->addWellPaths(newWellPaths);
            wellPathCollection->uiCapability()->updateConnectedEditors();
            wellPathCollection->scheduleRedrawAffectedViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewEditableWellPathFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Create Editable Well Path");
    actionToSetup->setIcon(QIcon(":/Well.png"));
}


