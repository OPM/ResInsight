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

#include "RicNewWellPathFractureFeature.h"


#include "RiaApplication.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RiuMainWindow.h"
 
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewWellPathFractureFeature, "RicNewWellPathFractureFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureFeature::onActionTriggered(bool isChecked)
{
    RimWellPathFractureCollection* fractureColl = RicNewWellPathFractureFeature::selectedWellPathFractureCollection();
    if (!fractureColl) return;

    RimWellPath* wellPath = nullptr;
    fractureColl->firstAncestorOrThisOfTypeAsserted(wellPath);

    if (!RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem(wellPath)) return;

    RimWellPathFracture* fracture = new RimWellPathFracture();
    fractureColl->fractures.push_back(fracture);
        
    float md_default = 0.0f;
    fracture->setMeasuredDepth(md_default);
    fracture->fractureUnit = wellPath->unitSystem();
    
    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    cvf::Vec3d positionAtWellpath = wellPathGeometry->interpolatedPointAlongWellPath(md_default);
    fracture->setAnchorPosition(positionAtWellpath);

    RimOilField* oilfield = nullptr;
    fractureColl->firstAncestorOrThisOfType(oilfield);
    if (!oilfield) return;

    std::vector<RimFracture* > oldFractures;
    oilfield->descendantsIncludingThisOfType(oldFractures);
    QString fracNum = QString("%1").arg(oldFractures.size(), 2, 10, QChar('0'));

    fracture->setName(QString("Fracture_") + fracNum);

    if (oilfield->fractureDefinitionCollection->fractureDefinitions.size() > 0)
    {
        RimFractureTemplate* fracDef = oilfield->fractureDefinitionCollection->fractureDefinitions[0];
        fracture->setFractureTemplate(fracDef);
    }

    wellPath->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(fracture);

    RimWellPathCollection* wellPathColl = nullptr;
    fractureColl->firstAncestorOrThisOfType(wellPathColl);
    if (wellPathColl)
    {
        wellPathColl->scheduleGeometryRegenAndRedrawViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("New Fracture");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathFractureFeature::isCommandEnabled()
{
    if (selectedWellPathFractureCollection())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RicNewWellPathFractureFeature::selectedWellPathFractureCollection()
{
    RimWellPathFractureCollection* objToFind = nullptr;

    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}
