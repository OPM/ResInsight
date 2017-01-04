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

#include "RicNewWellPathFractureAtPosFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimFractureDefinition.h"
#include "RimFractureDefinitionCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "RivWellPathSourceInfo.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"
#include "cvfVector3.h"
#include "cvfRenderState_FF.h"

#include <QAction>



CAF_CMD_SOURCE_INIT(RicNewWellPathFractureAtPosFeature, "RicNewWellPathFractureAtPosFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureAtPosFeature::onActionTriggered(bool isChecked)
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

    RimWellPath* wellPath = wellPathItem->m_wellpath;
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(wellPath);
    if (!objHandle) return;

    RimWellPath* wellPathObj = nullptr;
    objHandle->firstAncestorOrThisOfType(wellPathObj);
    if (!wellPathObj) return;

    RimWellPathFractureCollection* fractureCollection = wellPathObj->fractureCollection();

    RimWellPathFracture* fracture = new RimWellPathFracture();
    fractureCollection->fractures.push_back(fracture);
        
    fracture->name = "Well Path Fracture";
    fracture->positionAtWellpath = wellPathItem->m_pipeCenterlineIntersectionInDomainCoords;
    fracture->measuredDepth = wellPathItem->m_measuredDepth;

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
void RicNewWellPathFractureAtPosFeature::setupActionLook(QAction* actionToSetup)
{
//   actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Fracture");
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathFractureAtPosFeature::isCommandEnabled()
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
