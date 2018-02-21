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

#include "RicNewEllipseFractureTemplateFeature.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewEllipseFractureTemplateFeature, "RicNewEllipseFractureTemplateFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEllipseFractureTemplateFeature::selectFractureTemplateAndUpdate(RimFractureTemplateCollection* templateCollection,
                                                                           RimFractureTemplate*           fractureTemplate)
{
    fractureTemplate->loadDataAndUpdate();

    templateCollection->updateConnectedEditors();

    RimProject* project = RiaApplication::instance()->project();

    std::vector<Rim3dView*> views;
    project->allVisibleViews(views);

    for (Rim3dView* view : views)
    {
        if (dynamic_cast<RimEclipseView*>(view))
        {
            view->updateConnectedEditors();
        }
    }

    RiuMainWindow::instance()->selectAsCurrentItem(fractureTemplate);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEllipseFractureTemplateFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimOilField* oilfield = project->activeOilField();
    if (oilfield == nullptr) return;

    RimFractureTemplateCollection* fracDefColl = oilfield->fractureDefinitionCollection();

    if (fracDefColl)
    {
        RimEllipseFractureTemplate* ellipseFractureTemplate = new RimEllipseFractureTemplate();

        fracDefColl->fractureDefinitions.push_back(ellipseFractureTemplate);
        ellipseFractureTemplate->setName("Ellipse Fracture Template");
        ellipseFractureTemplate->setFractureTemplateUnit(fracDefColl->defaultUnitsForFracTemplates());
        ellipseFractureTemplate->setDefaultValuesFromUnit();

        selectFractureTemplateAndUpdate(fracDefColl, ellipseFractureTemplate);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEllipseFractureTemplateFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));
    actionToSetup->setText("New Ellipse Fracture Template");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEllipseFractureTemplateFeature::isCommandEnabled()
{
    return true;
}
