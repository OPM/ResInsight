/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicNewFishbonesSubsFeature.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RiaApplication.h"

#include "RigWellPath.h"

#include "RimProject.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimView.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

#include <cmath>


CAF_CMD_SOURCE_INIT(RicNewFishbonesSubsFeature, "RicNewFishbonesSubsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double getWellPathTipMd(RimWellPath* wellPath);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onActionTriggered(bool isChecked)
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT(fishbonesCollection);

    RimWellPath* wellPath;
    fishbonesCollection->firstAncestorOrThisOfTypeAsserted(wellPath);
    if (!RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem(wellPath)) return;

    RimFishbonesMultipleSubs* obj = new RimFishbonesMultipleSubs;
    fishbonesCollection->appendFishbonesSubs(obj);

    double wellPathTipMd = getWellPathTipMd(wellPath);
    if (wellPathTipMd != HUGE_VAL)
    {
        double startMd = wellPathTipMd - 150 - 100;
        if (startMd < 100) startMd = 100;

        obj->setMeasuredDepthAndCount(startMd, 12.5, 13);
    }

    RicNewFishbonesSubsFeature::askUserToSetUsefulScaling(fishbonesCollection);


    RimWellPathCollection* wellPathCollection = nullptr;
    fishbonesCollection->firstAncestorOrThisOfType(wellPathCollection);
    if (wellPathCollection)
    {
        wellPathCollection->uiCapability()->updateConnectedEditors();
    }

    RiuMainWindow::instance()->selectAsCurrentItem(obj);

    RimProject* proj;
    fishbonesCollection->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicNewFishbonesSubsFeature::selectedFishbonesCollection()
{
    RimFishbonesCollection* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    if (objToFind == nullptr)
    {
        std::vector<RimWellPath*> wellPaths;
        caf::SelectionManager::instance()->objectsByType(&wellPaths);
        if (!wellPaths.empty())
        {
            return wellPaths[0]->fishbonesCollection();
        }
    }

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FishBoneGroup16x16.png"));
    actionToSetup->setText("New Fishbones Subs Definition");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewFishbonesSubsFeature::isCommandEnabled()
{
    if (selectedFishbonesCollection())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::askUserToSetUsefulScaling(RimFishbonesCollection* fishboneCollection)
{
    // Always reset well path collection scale factor
    CVF_ASSERT(fishboneCollection);
    RimWellPathCollection* wellPathColl = nullptr;
    fishboneCollection->firstAncestorOrThisOfTypeAsserted(wellPathColl);
    wellPathColl->wellPathRadiusScaleFactor = 0.01;

    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RiaApplication* app = RiaApplication::instance();
    QString sessionKey = "AutoAdjustSettingsForFishbones";

    bool autoAdjustSettings = false;
    QVariant v = app->cacheDataObject(sessionKey);
    if (!v.isValid())
    {
        double currentScaleFactor = activeView->scaleZ();
        if (fabs(currentScaleFactor - 1.0) < 0.1) return;

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);

        QString questionText;
        questionText = QString("When displaying Fishbones structures, the view scaling should be set to 1.\n\nDo you want ResInsight to automatically set view scaling to 1?");

        msgBox.setText(questionText);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
        {
            autoAdjustSettings = true;
        }

        app->setCacheDataObject(sessionKey, autoAdjustSettings);
    }
    else
    {
        autoAdjustSettings = v.toBool();
    }

    if (autoAdjustSettings)
    {
        activeView->setScaleZAndUpdate(1.0);
        activeView->scheduleCreateDisplayModelAndRedraw();

        RiuMainWindow::instance()->updateScaleValue();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double getWellPathTipMd(RimWellPath* wellPath)
{
    RigWellPath* geometry = wellPath ? wellPath->wellPathGeometry() : nullptr;

    if (geometry && !geometry->m_measuredDepths.empty())
    {
        return geometry->m_measuredDepths.back();
    }
    return HUGE_VAL;
}
