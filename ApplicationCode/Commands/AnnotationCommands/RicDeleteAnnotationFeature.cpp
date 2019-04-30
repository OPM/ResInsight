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

#include "RicDeleteAnnotationFeature.h"

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


CAF_CMD_SOURCE_INIT(RicDeleteAnnotationFeature, "RicDeleteAnnotationFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteAnnotationFeature::isCommandEnabled()
{
    auto textAnnots  = caf::selectedObjectsByTypeStrict<RimTextAnnotation*>();
    auto lineBasedAnnots = caf::selectedObjectsByTypeStrict<RimLineBasedAnnotation*>();

    return !textAnnots.empty() || !lineBasedAnnots.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteAnnotationFeature::onActionTriggered(bool isChecked)
{
    {
        auto annotations = caf::selectedObjectsByTypeStrict<RimTextAnnotation*>();
        while(!annotations.empty())
        {
            auto annotation = annotations.front();

            RimAnnotationCollection* coll;
            annotation->firstAncestorOrThisOfType(coll);
            if (coll)
            {
                coll->addAnnotation()
            }

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
void RicDeleteAnnotationFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/minus-sign-red.png"));
    actionToSetup->setText("Delete Annotation");
}
