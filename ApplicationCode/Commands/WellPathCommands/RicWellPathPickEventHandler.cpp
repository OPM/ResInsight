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

#include "RicWellPathPickEventHandler.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "Rim2dIntersectionView.h"

#include "RiuMainWindow.h"

#include "RivObjectSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "cafDisplayCoordTransform.h"

#include "cvfPart.h"
#include "cvfVector3.h"
#include "RivIntersectionPartMgr.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicWellPathPickEventHandler* RicWellPathPickEventHandler::instance()
{
    static RicWellPathPickEventHandler* singleton = new RicWellPathPickEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathPickEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    if (eventObject.m_pickItemInfos.empty()) return false;

    const caf::PdmObject* objectToSelect = nullptr;

    cvf::uint wellPathTriangleIndex = cvf::UNDEFINED_UINT;
    const RivWellPathSourceInfo* wellPathSourceInfo = nullptr;

    if(!eventObject.m_pickItemInfos.empty())
    {
        const auto & firstPickedItem = eventObject.m_pickItemInfos.front();
        const cvf::Part* firstPickedPart = firstPickedItem.pickedPart();

        if (firstPickedPart)
        {
            const RivObjectSourceInfo* sourceInfo = dynamic_cast<const RivObjectSourceInfo*>(firstPickedPart->sourceInfo());
            if (sourceInfo)
            {
                if (dynamic_cast<RimPerforationInterval*>(sourceInfo->object()))
                {
                    objectToSelect = sourceInfo->object();

                    if (eventObject.m_pickItemInfos.size() > 1)
                    {
                        const auto&      secondPickedItem = eventObject.m_pickItemInfos[1];
                        const cvf::Part* secondPickedPart = secondPickedItem.pickedPart();
                        if (secondPickedPart)
                        {
                            auto wellPathSourceCandidate =
                                dynamic_cast<const RivWellPathSourceInfo*>(secondPickedPart->sourceInfo());
                            if (wellPathSourceCandidate)
                            {
                                RimWellPath* perforationWellPath = nullptr;
                                objectToSelect->firstAncestorOrThisOfType(perforationWellPath);
                                if (perforationWellPath == wellPathSourceCandidate->wellPath())
                                {
                                    wellPathSourceInfo =
                                        dynamic_cast<const RivWellPathSourceInfo*>(secondPickedPart->sourceInfo());
                                    wellPathTriangleIndex = secondPickedItem.faceIdx();
                                }
                            }
                        }
                    }
                }
            }

            if (dynamic_cast<const RivWellPathSourceInfo*>(firstPickedPart->sourceInfo()))
            {
                wellPathSourceInfo    = dynamic_cast<const RivWellPathSourceInfo*>(firstPickedPart->sourceInfo());
                wellPathTriangleIndex = firstPickedItem.faceIdx();
            }
        }
    }

    if (wellPathSourceInfo)
    {
        Rim3dView* rimView = RiaApplication::instance()->activeReservoirView();
        if (!rimView) return false;

        cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
        cvf::Vec3d pickedPositionInUTM = transForm->transformToDomainCoord(eventObject.m_pickItemInfos.front().globalPickedPoint());

        if (auto intersectionView = dynamic_cast<Rim2dIntersectionView*>(rimView))
        {
            pickedPositionInUTM = intersectionView->transformToUtm(pickedPositionInUTM);
        }

        double measuredDepth = wellPathSourceInfo->measuredDepth(wellPathTriangleIndex, pickedPositionInUTM);

        // NOTE: This computation was used to find the location for a fracture when clicking on a well path
        // It turned out that the computation was a bit inaccurate
        // Consider to use code in RigSimulationWellCoordsAndMD instead
        cvf::Vec3d trueVerticalDepth = wellPathSourceInfo->closestPointOnCenterLine(wellPathTriangleIndex, pickedPositionInUTM);

        QString wellPathText;
        wellPathText += QString("Well path name : %1\n").arg(wellPathSourceInfo->wellPath()->name());
        wellPathText += QString("Measured depth : %1\n").arg(measuredDepth);

        QString formattedText;
        formattedText.sprintf("Intersection point : [E: %.2f, N: %.2f, Depth: %.2f]", trueVerticalDepth.x(), trueVerticalDepth.y(), -trueVerticalDepth.z());
        wellPathText += formattedText;

        RiuMainWindow::instance()->setResultInfo(wellPathText);

        if (objectToSelect)
        {
            RiuMainWindow::instance()->selectAsCurrentItem(objectToSelect);
        }
        else
        {
            RiuMainWindow::instance()->selectAsCurrentItem(wellPathSourceInfo->wellPath());
        }

        return true;
    }

    return false;
}

