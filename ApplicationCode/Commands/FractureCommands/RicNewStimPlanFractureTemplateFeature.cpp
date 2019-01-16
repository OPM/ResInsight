/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-2018 Statoil ASA
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimEclipseView.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPathFracture.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileDialog>


CAF_CMD_SOURCE_INIT(RicNewStimPlanFractureTemplateFeature, "RicNewStimPlanFractureTemplateFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::createNewTemplateForFractureAndUpdate(RimFracture* fracture)
{
    std::vector<RimStimPlanFractureTemplate*> newTemplates = createNewTemplates();
    if (!newTemplates.empty())
    {
        RimStimPlanFractureTemplate* lastTemplateCreated = newTemplates.back();
        fracture->setFractureTemplate(lastTemplateCreated);
        
        selectFractureTemplateAndUpdate(lastTemplateCreated);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::selectFractureTemplateAndUpdate(RimFractureTemplate* fractureTemplate)
{
    fractureTemplate->loadDataAndUpdate();

    RimFractureTemplateCollection* templateCollection = nullptr;
    fractureTemplate->firstAncestorOrThisOfTypeAsserted(templateCollection);
    templateCollection->updateConnectedEditors();

    RimProject* project = RiaApplication::instance()->project();

    project->scheduleCreateDisplayModelAndRedrawAllViews();
    Riu3DMainWindowTools::selectAsCurrentItem(fractureTemplate);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanFractureTemplate*> RicNewStimPlanFractureTemplateFeature::createNewTemplates()
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");
    QStringList     fileNames =
        QFileDialog::getOpenFileNames(nullptr, "Open StimPlan XML File", defaultDir, "StimPlan XML File (*.xml);;All files(*.*)");

    if (fileNames.isEmpty()) return std::vector<RimStimPlanFractureTemplate*>();

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimOilField* oilfield = project->activeOilField();
    if (oilfield == nullptr) return std::vector<RimStimPlanFractureTemplate*>();

    RimFractureTemplateCollection* fracDefColl = oilfield->fractureDefinitionCollection();
    if (!fracDefColl) return std::vector<RimStimPlanFractureTemplate*>();

    std::vector<RimStimPlanFractureTemplate*> newFractures;
    for (auto fileName : fileNames)
    {
        if (fileName.isEmpty()) continue;

        RimStimPlanFractureTemplate* fractureDef = new RimStimPlanFractureTemplate();
        fracDefColl->addFractureTemplate(fractureDef);

        QFileInfo stimplanfileFileInfo(fileName);
        QString   name = stimplanfileFileInfo.baseName();
        if (name.isEmpty())
        {
            name = "StimPlan Fracture Template";
        }

        fractureDef->setName(name);

        fractureDef->setFileName(fileName);
        fractureDef->loadDataAndUpdate();
        fractureDef->setDefaultsBasedOnXMLfile();
        fractureDef->setDefaultWellDiameterFromUnit();
        fractureDef->updateFractureGrid();
        newFractures.push_back(fractureDef);
    }
    return newFractures;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimStimPlanFractureTemplate*> newFractures = createNewTemplates();
    selectFractureTemplateAndUpdate(newFractures.back());
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
