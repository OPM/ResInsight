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

#include "RicNewWellPathCollFractureAtPosFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimFracture.h"
#include "RimFractureCollection.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"
 
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "QAction.h"
#include "RimWellPath.h"
#include "RimView.h"
#include "cvfVector3.h"
#include "cvfRenderState_FF.h"
#include "RiuViewer.h"


CAF_CMD_SOURCE_INIT(RicNewWellPathCollFractureAtPosFeature, "RicNewWellPathCollFractureAtPosFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCollFractureAtPosFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    std::vector<RimWellPath*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    if (collection.size() != 1) return;

    RimWellPath* wellPath = collection[0];

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(wellPath);
    if (!objHandle) return;

    RimWellPathCollection* wellPathColl = nullptr;
    objHandle->firstAncestorOrThisOfType(wellPathColl);
    CVF_ASSERT(wellPathColl);

    RimFractureCollection* fractureCollection = wellPathColl->fractureCollection();
     
    RimFracture* fracture = new RimFracture();
    fractureCollection->fractures.push_back(fracture);
        
    fracture->name = "New Well Path Fracture";
    fracture->welltype = RimFracture::FRACTURE_WELL_PATH;
    fracture->wellpath = wellPath;
    //TODO set all relevant defaults...


    cvf::Vec3d domainCoord = activeView->viewer()->lastPickPositionInDomainCoords();
    fracture->positionAtWellpath = domainCoord;



    fractureCollection->updateConnectedEditors();

}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCollFractureAtPosFeature::setupActionLook(QAction* actionToSetup)
{
//   actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Fracture");
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathCollFractureAtPosFeature::isCommandEnabled()
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
