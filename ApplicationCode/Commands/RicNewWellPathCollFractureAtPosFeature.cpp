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

#include "RimWellPath.h"
#include "RimView.h"
#include "cvfVector3.h"
#include "cvfRenderState_FF.h"
#include "RiuViewer.h"
#include "RiuSelectionManager.h"
#include <QAction>



CAF_CMD_SOURCE_INIT(RicNewWellPathCollFractureAtPosFeature, "RicNewWellPathCollFractureAtPosFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCollFractureAtPosFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    RiuSelectionItem* selItem = riuSelManager->selectedItem(RiuSelectionManager::RUI_TEMPORARY);

    RiuWellPathSelectionItem* wellPathItem = nullptr;
    if (selItem->type() == RiuSelectionItem::WELLPATH_SELECTION_OBJECT)
    {
        wellPathItem = static_cast<RiuWellPathSelectionItem*>(selItem);
        if (!wellPathItem) return;
    }

    const RivWellPathSourceInfo* wellpathSourceInfo = wellPathItem->m_wellpathSourceInfo;
    RimWellPath* wellPath = wellpathSourceInfo->wellPath();
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(wellPath);
    if (!objHandle) return;

    RimWellPath* wellPathObj = nullptr;
    objHandle->firstAncestorOrThisOfType(wellPathObj);
    if (!wellPathObj) return;

    RimFractureCollection* fractureCollection = wellPathObj->fractureCollection();

    RimFracture* fracture = new RimFracture();
    fractureCollection->fractures.push_back(fracture);
        
    fracture->name = "New Well Path Fracture";
    fracture->welltype = RimFracture::FRACTURE_WELL_PATH;
    fracture->wellpath = wellPath;
    fracture->positionAtWellpath = wellPathItem->m_currentPickPositionInDomainCoords;

    double measuredDepth = wellpathSourceInfo->measuredDepth(wellPathItem->m_firstPartTriangleIndex, wellPathItem->m_currentPickPositionInDomainCoords);
    fracture->measuredDepth = measuredDepth;
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
