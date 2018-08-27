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

#include "Ric3dWellLogCurveViewerEventHandler.h"

#include "Rim3dWellLogCurve.h"
#include "Rim3dWellLogCurveCollection.h"
#include "RimWellPath.h"
#include "RiuMainWindow.h"
#include "RivObjectSourceInfo.h"

#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Ric3dWellLogCurveViewerEventHandler* Ric3dWellLogCurveViewerEventHandler::instance()
{
    static Ric3dWellLogCurveViewerEventHandler* singleton = new Ric3dWellLogCurveViewerEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Ric3dWellLogCurveViewerEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    if (eventObject.m_pickItemInfos.empty()) return false;

    cvf::uint triangleIndex = cvf::UNDEFINED_UINT;

    const auto&      firstPickedItem = eventObject.m_pickItemInfos.front();
    const cvf::Part* firstPickedPart            = firstPickedItem.pickedPart();

    const RivObjectSourceInfo* sourceInfo = dynamic_cast<const RivObjectSourceInfo*>(firstPickedPart->sourceInfo());
    if (sourceInfo)
    {
        Rim3dWellLogCurveCollection* curveCollection = dynamic_cast<Rim3dWellLogCurveCollection*>(sourceInfo->object());
        if (curveCollection)
        {
            RimWellPath* wellPath;
            curveCollection->firstAncestorOrThisOfTypeAsserted(wellPath);
            QString wellPathName = wellPath->name();
            cvf::Vec3d closestPoint;
            double measuredDepthAtPoint;
            double valueAtPoint;
            Rim3dWellLogCurve* curve = curveCollection->checkForCurveIntersection( firstPickedItem.globalPickedPoint(), 
                                                                                  &closestPoint, 
                                                                                  &measuredDepthAtPoint, 
                                                                                  &valueAtPoint);
            if (curve)
            {
                RiuMainWindow::instance()->selectAsCurrentItem(curve);

                QString curveText;
                curveText += QString("Curve name : %1\n").arg(curve->name());;
                curveText += QString("Well path name: %1\n").arg(wellPathName);
                curveText += QString("Measured depth: %1\n").arg(measuredDepthAtPoint);
                curveText += QString("%1 at depth: %2\n").arg(curve->resultPropertyString()).arg(valueAtPoint);

                RiuMainWindow::instance()->setResultInfo(curveText);
            }
            else
            {
                RiuMainWindow::instance()->selectAsCurrentItem(curveCollection);
            }
            return true;
        }
    }
    return false;
}
