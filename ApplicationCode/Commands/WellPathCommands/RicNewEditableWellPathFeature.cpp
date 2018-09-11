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
#include "Riu3DMainWindowTools.h"
#include "RimWellPathGeometryDef.h"

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
    RimProject* project = RiaApplication::instance()->project();
    if (project  && RiaApplication::instance()->project()->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = RiaApplication::instance()->project()->activeOilField()->wellPathCollection();

        if ( wellPathCollection )
        {
            std::vector<RimWellPath*> newWellPaths;
            auto newModeledWellPath = new RimModeledWellPath();
            newModeledWellPath->setUnitSystem(project->commonUnitSystemForAllCases());
            newWellPaths.push_back(newModeledWellPath);
            newWellPaths.back()->setName("UWell-" + QString::number(wellPathCollection->modelledWellPathCount()+1));
            wellPathCollection->addWellPaths(newWellPaths);
            wellPathCollection->uiCapability()->updateConnectedEditors();
            
            newModeledWellPath->geometryDefinition()->enableTargetPointPicking(true);

            RimProject* project;
            wellPathCollection->firstAncestorOrThisOfTypeAsserted(project);
            project->createDisplayModelAndRedrawAllViews();

            Riu3DMainWindowTools::selectAsCurrentItem(newModeledWellPath->geometryDefinition());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewEditableWellPathFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Create Well Path");
    actionToSetup->setIcon(QIcon(":/Well.png"));
}


