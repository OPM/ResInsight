/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "Ric3dWellLogCurveDeleteFeature.h"

#include "RiaApplication.h"

#include "Rim3dWellLogCurve.h"
#include "Rim3dWellLogCurveCollection.h"
#include "RimProject.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(Ric3dWellLogCurveDeleteFeature, "Ric3dWellLogCurveDeleteFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Ric3dWellLogCurveDeleteFeature::isCommandEnabled()
{
    std::vector<Rim3dWellLogCurve*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() > 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Ric3dWellLogCurveDeleteFeature::onActionTriggered(bool isChecked)
{
    std::vector<Rim3dWellLogCurve*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() == 0) return;

    Rim3dWellLogCurve* firstCurve = objects[0];

    Rim3dWellLogCurveCollection* curveCollection = nullptr;
    firstCurve->firstAncestorOrThisOfType(curveCollection);
    if (!curveCollection) return;

    for (Rim3dWellLogCurve* curve : objects)
    {
        curveCollection->remove3dWellLogCurve(curve);
        delete curve;
    }
    curveCollection->redrawAffectedViewsAndEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Ric3dWellLogCurveDeleteFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete 3D Well Log Curve(s)");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}
