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

#include "RicCreateTextAnnotationFeature.h"

#include "RiaApplication.h"

#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimPolylinesAnnotation.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimProject.h"
#include "RimOilField.h"

#include "RiuMainWindow.h"

#include <cafSelectionManagerTools.h>

#include <QAction>


CAF_CMD_SOURCE_INIT(RicCreateTextAnnotationFeature, "RicCreateTextAnnotationFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateTextAnnotationFeature::isCommandEnabled()
{
    auto selObjsGlobal  = caf::selectedObjectsByTypeStrict<RimAnnotationCollection*>();
    auto selObjs2InView = caf::selectedObjectsByTypeStrict<RimAnnotationInViewCollection*>();

    return selObjsGlobal.size() == 1 || selObjs2InView.size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationFeature::onActionTriggered(bool isChecked)
{
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

    {
        auto coll = annotationInViewCollection();
        if (coll)
        {
            auto newAnnotation = new RimTextAnnotation();
            coll->addAnnotation(newAnnotation);
            coll->updateConnectedEditors();
            RiuMainWindow::instance()->selectAsCurrentItem(newAnnotation);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Plus.png"));
    actionToSetup->setText("Create Text Annotation");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
 RimAnnotationCollection* RicCreateTextAnnotationFeature::annotationCollection() const
{
     auto selObjs = caf::selectedObjectsByTypeStrict<RimAnnotationCollection*>();
     return selObjs.size() == 1 ? selObjs.front() : nullptr;
 }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection* RicCreateTextAnnotationFeature::annotationInViewCollection() const
{
    auto selObjs = caf::selectedObjectsByTypeStrict<RimAnnotationInViewCollection*>();
    return selObjs.size() == 1 ? selObjs.front() : nullptr;
}
