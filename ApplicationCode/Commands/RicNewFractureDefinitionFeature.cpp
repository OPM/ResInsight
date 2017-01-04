/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewFractureDefinitionFeature.h"

#include "RiaApplication.h"

#include "RimOilField.h"
#include "RimFractureDefinition.h"
#include "RimFractureDefinitionCollection.h"
#include "RimProject.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewFractureDefinitionFeature, "RicNewFractureDefinitionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFractureDefinitionFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimOilField* oilfield = project->activeOilField();
    if (oilfield == nullptr) return;

    RimFractureDefinitionCollection* fracDefColl = oilfield->fractureDefinitionCollection();

    if (fracDefColl)
    {
        RimFractureDefinition* fractureDef = new RimFractureDefinition();
        fracDefColl->fractureDefinitions.push_back(fractureDef);
        fractureDef->name = "Fracture Template";
        
        fracDefColl->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(fractureDef);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFractureDefinitionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Fracture16x16.png"));
    actionToSetup->setText("New Fracture Template");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewFractureDefinitionFeature::isCommandEnabled()
{
    return true;
}
