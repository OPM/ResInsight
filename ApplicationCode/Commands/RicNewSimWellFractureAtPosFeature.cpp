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

#include "RicNewSimWellFractureAtPosFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseWell.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimProject.h"
 
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include "RiuSelectionManager.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RiuMainWindow.h"


CAF_CMD_SOURCE_INIT(RicNewSimWellFractureAtPosFeature, "RicNewSimWellFractureAtPosFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureAtPosFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(RiuSelectionManager::RUI_TEMPORARY);

    RiuSimWellSelectionItem* simWellItem = nullptr;
    if (selItem->type() == RiuSelectionItem::SIMWELL_SELECTION_OBJECT)
    {
        simWellItem = static_cast<RiuSimWellSelectionItem*>(selItem);
        if (!simWellItem) return;
    }
    
    RimEclipseWell* simWell = simWellItem->m_simWell;
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(simWell);
    if (!objHandle) return;

    RimEclipseWell* simWellObject = nullptr;
    objHandle->firstAncestorOrThisOfType(simWellObject);
    if (!simWellObject) return;

    RimSimWellFractureCollection* fractureCollection = simWellObject->simwellFractureCollection();
    if (!fractureCollection) return;

    RimSimWellFracture* fracture = new RimSimWellFracture();
    fractureCollection->simwellFractures.push_back(fracture);

    fracture->name = "Simulation Well Fracture";
    fracture->setijk(simWellItem->i, simWellItem->j, simWellItem->k);

    fractureCollection->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(fracture);


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureAtPosFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Fracture");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellFractureAtPosFeature::isCommandEnabled()
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
