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

#include "RicWellPathViewerEventHandler.h"

#include "RiaApplication.h"

#include "RimView.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"

#include "RivWellPathSourceInfo.h"

#include "cafDisplayCoordTransform.h"

#include "cvfPart.h"
#include "cvfVector3.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicWellPathViewerEventHandler* RicWellPathViewerEventHandler::instance()
{
    static RicWellPathViewerEventHandler* singleton = new RicWellPathViewerEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathViewerEventHandler::handleEvent(cvf::Object* eventObject)
{
    RicViewerEventObject* uiEvent = dynamic_cast<RicViewerEventObject*>(eventObject);
    if (!uiEvent) return false;

    if (uiEvent->firstHitPart && uiEvent->firstHitPart->sourceInfo())
    {
        const RivWellPathSourceInfo* wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(uiEvent->firstHitPart->sourceInfo());
        if (wellPathSourceInfo)
        {
            RimView* rimView = RiaApplication::instance()->activeReservoirView();
            if (!rimView) return false;

            cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
            cvf::Vec3d domainCoord = transForm->transformToDomainCoord(uiEvent->globalIntersectionPoint);

            double measuredDepth = wellPathSourceInfo->measuredDepth(uiEvent->firstPartTriangleIndex, domainCoord);
            cvf::Vec3d trueVerticalDepth = wellPathSourceInfo->trueVerticalDepth(uiEvent->firstPartTriangleIndex, domainCoord);

            QString wellPathText;
            wellPathText += QString("Well path name : %1\n").arg(wellPathSourceInfo->wellPath()->name());
            wellPathText += QString("Measured depth : %1\n").arg(measuredDepth);

            QString formattedText;
            formattedText.sprintf("Intersection point : [E: %.2f, N: %.2f, Depth: %.2f]", trueVerticalDepth.x(), trueVerticalDepth.y(), -trueVerticalDepth.z());
            wellPathText += formattedText;

            RiuMainWindow::instance()->setResultInfo(wellPathText);

            RiuMainWindow::instance()->selectAsCurrentItem(wellPathSourceInfo->wellPath());

            return true;
        }
    }

    return false;
}

