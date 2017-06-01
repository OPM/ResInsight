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

#include "RiaApplication.h"

#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimView.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>


CAF_CMD_SOURCE_INIT(RicNewFishbonesSubsFeature, "RicNewFishbonesSubsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onActionTriggered(bool isChecked)
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT(fishbonesCollection);

    RimFishbonesMultipleSubs* obj = new RimFishbonesMultipleSubs;
    obj->setName(QString("Fishbones Subs (%1)").arg(fishbonesCollection->fishbonesSubs.size()));
    fishbonesCollection->appendFishbonesSubs(obj);

    RicNewFishbonesSubsFeature::askUserToSetUsefulScaling(fishbonesCollection);

    fishbonesCollection->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(obj);
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

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
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

        wellPathColl->scheduleGeometryRegenAndRedrawViews();

        RiuMainWindow::instance()->updateScaleValue();
    }
}
