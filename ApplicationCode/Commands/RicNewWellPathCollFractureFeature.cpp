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

#include "RicNewWellPathCollFractureFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimFracture.h"
#include "RimFractureCollection.h"
#include "RimProject.h"
 
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "QAction.h"
#include "RimEclipseWell.h"
#include "RimWellPathCollection.h"


CAF_CMD_SOURCE_INIT(RicNewWellPathCollFractureFeature, "RicNewWellPathCollFractureFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCollFractureFeature::onActionTriggered(bool isChecked)
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimWellPathCollection* wellPathColl = nullptr;
    objHandle->firstAncestorOrThisOfType(wellPathColl);

//     RimEclipseWell* eclipseWell = nullptr;
//     objHandle->firstAncestorOrThisOfType(eclipseWell);

    RimFractureCollection* fractureCollection = nullptr;
    objHandle->firstAncestorOrThisOfType(fractureCollection);
    CVF_ASSERT(fractureCollection);

    RimFracture* fracture = new RimFracture();
    fractureCollection->fractures.push_back(fracture);
        
    fracture->name = "New Well Path Fracture";
    fracture->welltype = RimFracture::FRACTURE_WELL_PATH;
    //TODO set all relevant defaults...

    fractureCollection->updateConnectedEditors();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCollFractureFeature::setupActionLook(QAction* actionToSetup)
{
//    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Fracture");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathCollFractureFeature::isCommandEnabled()
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return false;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return false;

    RimWellPathCollection* wellPathColl = nullptr;
    objHandle->firstAncestorOrThisOfType(wellPathColl);

    if (wellPathColl)
    {
        return true;
    }

    return false;
}
