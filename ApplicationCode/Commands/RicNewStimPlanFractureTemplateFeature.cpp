/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicNewStimPlanFractureTemplateFeature.h"

#include "RiaApplication.h"

#include "RimOilField.h"
#include "RimFractureTemplateCollection.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileDialog>


CAF_CMD_SOURCE_INIT(RicNewStimPlanFractureTemplateFeature, "RicNewStimPlanFractureTemplateFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");
    QString fileName = QFileDialog::getOpenFileName(NULL, "Open StimPlan XML File", defaultDir, "StimPlan XML File (*.xml);;All files(*.*)");

    if (fileName.isEmpty()) return;

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimOilField* oilfield = project->activeOilField();
    if (oilfield == nullptr) return;

    RimFractureTemplateCollection* fracDefColl = oilfield->fractureDefinitionCollection();

    if (fracDefColl)
    {
        RimStimPlanFractureTemplate* fractureDef = new RimStimPlanFractureTemplate();
        fracDefColl->fractureDefinitions.push_back(fractureDef);
        fractureDef->name = "StimPlan Fracture Template";
        fractureDef->setFileName(fileName);
        fractureDef->loadDataAndUpdate();
        fractureDef->setDefaultsBasedOnXMLfile();
        fractureDef->setDefaultWellDiameterFromUnit();

        fracDefColl->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(fractureDef);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));
    actionToSetup->setText("New StimPlan Fracture Template");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewStimPlanFractureTemplateFeature::isCommandEnabled()
{
    return true;
}
