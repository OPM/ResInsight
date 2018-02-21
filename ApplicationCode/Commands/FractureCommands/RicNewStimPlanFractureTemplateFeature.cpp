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

#include "RimEclipseView.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
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
    QString fileName = QFileDialog::getOpenFileName(nullptr, "Open StimPlan XML File", defaultDir, "StimPlan XML File (*.xml);;All files(*.*)");

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

        QFileInfo stimplanfileFileInfo(fileName);
        QString name = stimplanfileFileInfo.baseName();
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

        fracDefColl->updateConnectedEditors();

        std::vector<Rim3dView*> views;
        project->allVisibleViews(views);

        for (Rim3dView* view : views)
        {
            if (dynamic_cast<RimEclipseView*>(view))
            {
                view->updateConnectedEditors();
            }
        }

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
