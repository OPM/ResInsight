/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RicCreateUserDefinedPolylinesAnnotationFeature.h"

#include "RiaApplication.h"

#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimUserDefinedPolylinesAnnotation.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimProject.h"
#include "RimOilField.h"

#include "RiuMainWindow.h"

#include <cafSelectionManagerTools.h>

#include <QAction>


CAF_CMD_SOURCE_INIT(RicCreateUserDefinedPolylinesAnnotationFeature, "RicCreateUserDefinedPolylinesAnnotationFeature");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateUserDefinedPolylinesAnnotationFeature::isCommandEnabled()
{
    auto selObjs = caf::selectedObjectsByTypeStrict<RimAnnotationCollection*>();
    return selObjs.size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateUserDefinedPolylinesAnnotationFeature::onActionTriggered(bool isChecked)
{
    auto coll = annotationCollection();
    if (coll)
    {
        auto newAnnotation = new RimUserDefinedPolylinesAnnotation();
        coll->addAnnotation(newAnnotation);
        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(newAnnotation);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateUserDefinedPolylinesAnnotationFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Plus.png"));
    actionToSetup->setText("Create User Defined Polyline Annotation");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
 RimAnnotationCollection* RicCreateUserDefinedPolylinesAnnotationFeature::annotationCollection() const
{
    auto project = RiaApplication::instance()->project();
    auto oilField = project->activeOilField();
    return oilField ? oilField->annotationCollection() : nullptr;
}
