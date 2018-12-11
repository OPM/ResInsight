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

#include "RicContourMapPickEventHandler.h"
#include "RimContourMapProjection.h"
#include "RimContourMapView.h"
#include "RimEclipseCellColors.h"
#include "Rim3dView.h"

#include "RiuMainWindow.h"
#include "RivObjectSourceInfo.h"

#include "cafDisplayCoordTransform.h"

#include "cvfPart.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicContourMapPickEventHandler* RicContourMapPickEventHandler::instance()
{
    static RicContourMapPickEventHandler* singleton = new RicContourMapPickEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicContourMapPickEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    if (eventObject.m_pickItemInfos.empty()) return false;

    const RiuPickItemInfo&      firstPickedItem = eventObject.m_pickItemInfos.front();
    const cvf::Part* firstPickedPart = firstPickedItem.pickedPart();

    const RivObjectSourceInfo* sourceInfo = dynamic_cast<const RivObjectSourceInfo*>(firstPickedPart->sourceInfo());
    if (sourceInfo)
    {
        RimContourMapProjection* contourMap = dynamic_cast<RimContourMapProjection*>(sourceInfo->object());
        if (contourMap)
        {
            RiuMainWindow::instance()->selectAsCurrentItem(contourMap);

            RimContourMapView* view = nullptr;
            contourMap->firstAncestorOrThisOfTypeAsserted(view);

            cvf::Vec2d pickedPoint;
            cvf::Vec2ui pickedCell;
            double valueAtPoint = 0.0;
            if (contourMap->checkForMapIntersection(firstPickedItem.globalPickedPoint(), &pickedPoint, &pickedCell, &valueAtPoint))
            {
                QString curveText;
                curveText += QString("%1\n").arg(view->createAutoName());
                curveText += QString("Picked Point X, Y: %1, %2\n").arg(pickedPoint.x(), 5, 'f', 0).arg(pickedPoint.y(), 5, 'f', 0);
                curveText += QString("Result Type: %1\n").arg(contourMap->resultDescriptionText());
                curveText += QString("Aggregated Value: %1\n").arg(valueAtPoint);

                RiuMainWindow::instance()->setResultInfo(curveText);

                contourMap->setPickPoint(pickedPoint);
                view->updateCurrentTimeStepAndRedraw();
                return true;
            }
            contourMap->setPickPoint(cvf::Vec2d::UNDEFINED);
            view->updateCurrentTimeStepAndRedraw();
            return true;
        }
    }
    return false;
}

