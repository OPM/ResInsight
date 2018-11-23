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

#include "RicNewAnnotationFeature.h"

#include "RiaApplication.h"

#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimPolylineAnnotation.h"
#include "RimAnnotationCollection.h"
#include "RimProject.h"
#include "RimOilField.h"

#include "RiuMainWindow.h"

#include <cafSelectionManagerTools.h>

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewAnnotationFeature, "RicNewAnnotationFeature");
CAF_CMD_SOURCE_INIT(RicNewTextAnnotationFeature, "RicNewTextAnnotationFeature");
CAF_CMD_SOURCE_INIT(RicNewPolylineAnnotationFeature, "RicNewPolygonAnnotationFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewAnnotationFeature::isCommandEnabled()
{
    auto selObjs = caf::selectedObjectsByTypeStrict<RimAnnotationCollection*>();
    return selObjs.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewAnnotationFeature::onActionTriggered(bool isChecked)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewAnnotationFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Plus.png"));
    actionToSetup->setText("(Not valid)");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection* RicNewAnnotationFeature::annotationCollection() const
{
    auto project = RiaApplication::instance()->project();
    auto oilField = project->activeOilField();
    return oilField ? oilField->annotationCollection() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewTextAnnotationFeature::onActionTriggered(bool isChecked)
{
    auto coll = annotationCollection();
    if (coll)
    {
        auto newAnnotation = new RimTextAnnotation();
        coll->addAnnotation(newAnnotation);
        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(newAnnotation);
    }

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewTextAnnotationFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Plus.png"));
    actionToSetup->setText("New Text Annotation");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolylineAnnotationFeature::onActionTriggered(bool isChecked)
{
    auto coll = annotationCollection();
    if (coll)
    {
        auto newAnnotation = new RimPolylineAnnotation();
        coll->addAnnotation(newAnnotation);
        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(newAnnotation);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolylineAnnotationFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Plus.png"));
    actionToSetup->setText("New Polyline Annotation");
}
