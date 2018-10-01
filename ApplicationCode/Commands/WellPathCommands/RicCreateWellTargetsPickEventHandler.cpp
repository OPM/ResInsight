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

#include "RicCreateWellTargetsPickEventHandler.h"

#include "RiaOffshoreSphericalCoords.h"

#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"
#include "RimWellPath.h"

#include "RivWellPathSourceInfo.h"

#include "cafSelectionManager.h"
#include "cafDisplayCoordTransform.h"

#include <vector>
#include "RiuViewerCommands.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicCreateWellTargetsPickEventHandler::RicCreateWellTargetsPickEventHandler(RimWellPathGeometryDef* wellGeometryDef)
    : m_geometryToAddTargetsTo(wellGeometryDef)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicCreateWellTargetsPickEventHandler::~RicCreateWellTargetsPickEventHandler()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetsPickEventHandler::notifyUnregistered()
{
    m_geometryToAddTargetsTo->enableTargetPointPicking(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCreateWellTargetsPickEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    if (!caf::SelectionManager::instance()->isSelected(m_geometryToAddTargetsTo.p(), 0))
    {
        m_geometryToAddTargetsTo->enableTargetPointPicking(false);

        return false;
    }

    if ( m_geometryToAddTargetsTo )
    {
        Rim3dView* rimView = eventObject.m_view;
        cvf::Vec3d targetPointInDomain = cvf::Vec3d::ZERO;

        // If clicked on an other well path, snap target point to well path center line
        auto firstPickItem = eventObject.m_pickItemInfos.front();
        auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(firstPickItem.sourceInfo());

        auto intersectionPointInDomain = rimView->displayCoordTransform()->transformToDomainCoord(firstPickItem.globalPickedPoint());
        bool doSetAzimuthAndInclination;
        double azimuth = 0.0, inclination = 0.0;
        if (wellPathSourceInfo)
        {
            targetPointInDomain = wellPathSourceInfo->closestPointOnCenterLine(firstPickItem.faceIdx(), intersectionPointInDomain);

            double md = wellPathSourceInfo->measuredDepth(firstPickItem.faceIdx(), intersectionPointInDomain);
            doSetAzimuthAndInclination = calculateAzimuthAndInclinationAtMd(md, wellPathSourceInfo->wellPath()->wellPathGeometry(), &azimuth, &inclination);
        }
        else
        {
            targetPointInDomain = intersectionPointInDomain;
            doSetAzimuthAndInclination = false;
        }

        if (!m_geometryToAddTargetsTo->firstActiveTarget())
        {
            m_geometryToAddTargetsTo->setReferencePointXyz(targetPointInDomain);
        }
        cvf::Vec3d referencePoint = m_geometryToAddTargetsTo->referencePointXyz();
        cvf::Vec3d relativeTagetPoint = targetPointInDomain - referencePoint;

        RimWellPathTarget* newTarget = new RimWellPathTarget;

        if (doSetAzimuthAndInclination)
        {
            newTarget->setAsPointXYZAndTangentTarget(cvf::Vec3d(relativeTagetPoint.x(), relativeTagetPoint.y(), relativeTagetPoint.z()), azimuth, inclination);
        }
        else
        {
            newTarget->setAsPointTargetXYD(cvf::Vec3d(relativeTagetPoint.x(), relativeTagetPoint.y(), -relativeTagetPoint.z()));
        }

        m_geometryToAddTargetsTo->insertTarget(nullptr, newTarget);

        m_geometryToAddTargetsTo->updateConnectedEditors();
        m_geometryToAddTargetsTo->updateWellPathVisualization();

        return true; // Todo: See if we really should eat the event instead
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCreateWellTargetsPickEventHandler::calculateAzimuthAndInclinationAtMd(double measuredDepth,
                                                                              const RigWellPath* wellPathGeometry,
                                                                              double *azimuth,
                                                                              double *inclination) const
{
    int mdIndex = -1;
    auto mdList = wellPathGeometry->measureDepths();

    for (int i = 0; i < mdList.size(); i++)
    {
        if (mdList[i] > measuredDepth)
        {
            mdIndex = i - 1;
            break;
        }
    }

    auto ptList = wellPathGeometry->wellPathPoints();
    if (mdIndex > 0 && mdIndex < ptList.size() - 2)
    {
        auto v1 = cvf::Vec3d(ptList[mdIndex - 1]);
        auto v2 = cvf::Vec3d(ptList[mdIndex]);
        auto v3 = cvf::Vec3d(ptList[mdIndex + 1]);
        auto v4 = cvf::Vec3d(ptList[mdIndex + 2]);

        auto v21 = v2 - v1;
        auto v32 = v3 - v2;
        auto v43 = v4 - v3;

        v21.normalize();
        v32.normalize();
        v43.normalize();

        auto v13mean = (v21 + v32) / 2;
        auto v24mean = (v32 + v43) / 2;

        double weight = (measuredDepth - mdList[mdIndex]) / (mdList[mdIndex + 1] - mdList[mdIndex]);
        auto vTan = v13mean * weight + v24mean * (1 - weight);

        RiaOffshoreSphericalCoords coords(vTan);
        *azimuth = coords.azi();
        *inclination = coords.inc();
        return true;
    }

    *azimuth = 0.0;
    *inclination = 0.0;
    return false;
}
