/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicPolylineTargetsPickEventHandler.h"

#include "RiaOffshoreSphericalCoords.h"

#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimModeledWellPath.h"
#include "RimPolylineTarget.h"
#include "RimUserDefinedPolylinesAnnotation.h"

#include "RiuViewerCommands.h"

#include "RivPolylinesAnnotationSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolylineTargetsPickEventHandler::RicPolylineTargetsPickEventHandler(RimUserDefinedPolylinesAnnotation* polylineDef)
    : m_polylineDef(polylineDef)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolylineTargetsPickEventHandler::~RicPolylineTargetsPickEventHandler() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineTargetsPickEventHandler::notifyUnregistered()
{
    //m_polylineDef->enableTargetPointPicking(false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPolylineTargetsPickEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    if (!caf::SelectionManager::instance()->isSelected(m_polylineDef.p(), 0))
    {
        //m_geometryToAddTargetsTo->enableTargetPointPicking(false);

        return false;
    }

    if (m_polylineDef)
    {
        Rim3dView* rimView             = eventObject.m_view;
        cvf::Vec3d targetPointInDomain = cvf::Vec3d::ZERO;

        // If clicked on an other well path, snap target point to well path center line
        auto firstPickItem      = eventObject.m_pickItemInfos.front();
        auto wellPathSourceInfo = dynamic_cast<const RivPolylinesAnnotationSourceInfo*>(firstPickItem.sourceInfo());

        auto intersectionPointInDomain =
            rimView->displayCoordTransform()->transformToDomainCoord(firstPickItem.globalPickedPoint());
        bool   doSetAzimuthAndInclination = false;
        double azimuth                    = 0.0;
        double inclination                = 0.0;

        if (wellPathSourceInfo)
        {
            //targetPointInDomain =
            //    wellPathSourceInfo->closestPointOnCenterLine(firstPickItem.faceIdx(), intersectionPointInDomain);

            //double md                  = wellPathSourceInfo->measuredDepth(firstPickItem.faceIdx(), intersectionPointInDomain);
            //doSetAzimuthAndInclination = calculateAzimuthAndInclinationAtMd(
            //    md, wellPathSourceInfo->wellPath()->wellPathGeometry(), &azimuth, &inclination);
        }
        else
        {
            targetPointInDomain        = intersectionPointInDomain;
            doSetAzimuthAndInclination = false;
        }

        //if (!m_polylineDef->firstActiveTarget())
        //{
        //    m_geometryToAddTargetsTo->setReferencePointXyz(targetPointInDomain);

        //    if (wellPathSourceInfo)
        //    {
        //        double mdrkbAtFirstTarget = wellPathSourceInfo->measuredDepth(firstPickItem.faceIdx(), intersectionPointInDomain);

        //        RimModeledWellPath* modeledWellPath = dynamic_cast<RimModeledWellPath*>(wellPathSourceInfo->wellPath());
        //        if (modeledWellPath)
        //        {
        //            mdrkbAtFirstTarget += modeledWellPath->geometryDefinition()->mdrkbAtFirstTarget();
        //        }

        //        m_geometryToAddTargetsTo->setMdrkbAtFirstTarget(mdrkbAtFirstTarget);
        //    }
        //}

        //cvf::Vec3d referencePoint     = m_polylineDef->referencePointXyz();
        //cvf::Vec3d relativeTagetPoint = targetPointInDomain - referencePoint;

        auto* newTarget = new RimPolylineTarget();

        newTarget->setAsPointTargetXYD(cvf::Vec3d(intersectionPointInDomain.x(), intersectionPointInDomain.y(), -intersectionPointInDomain.z()));

        m_polylineDef->insertTarget(nullptr, newTarget);

        m_polylineDef->updateConnectedEditors();
        m_polylineDef->updateVisualization();

        return true; // Todo: See if we really should eat the event instead
    }

    return false;
}

