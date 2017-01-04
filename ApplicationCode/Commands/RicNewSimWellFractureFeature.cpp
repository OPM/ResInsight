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

#include "RicNewSimWellFractureFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseWell.h"
#include "RimFractureDefinition.h"
#include "RimFractureDefinitionCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"

#include "RiuMainWindow.h"
 
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewSimWellFractureFeature, "RicNewSimWellFractureFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureFeature::onActionTriggered(bool isChecked)
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimEclipseWell* eclipseWell = nullptr;
    objHandle->firstAncestorOrThisOfType(eclipseWell);

    RimSimWellFractureCollection* fractureCollection = nullptr;
    objHandle->firstAncestorOrThisOfType(fractureCollection);
    CVF_ASSERT(fractureCollection);

    RimSimWellFracture* fracture = new RimSimWellFracture();
    fractureCollection->simwellFractures.push_back(fracture);
        
    fracture->name = "Simulation Well Fracture";
    RimOilField* oilfield = nullptr;
    objHandle->firstAncestorOrThisOfType(oilfield);
    if (!oilfield) return;

    if (oilfield->fractureDefinitionCollection->fractureDefinitions.size() > 0)
    {
        RimFractureDefinition* fracDef = oilfield->fractureDefinitionCollection->fractureDefinitions[0];
        fracture->fractureDefinition = fracDef;
    }


    fractureCollection->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(fracture);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Fracture");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellFractureFeature::isCommandEnabled()
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return false;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return false;

    RimEclipseWell* eclipseWell = nullptr;
    objHandle->firstAncestorOrThisOfType(eclipseWell);

    if (eclipseWell)
    {
        return true;
    }

    return false;
}
