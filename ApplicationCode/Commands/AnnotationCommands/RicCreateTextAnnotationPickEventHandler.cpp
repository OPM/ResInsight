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

#include "RicCreateTextAnnotationPickEventHandler.h"

#include "RiaOffshoreSphericalCoords.h"

#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimModeledWellPath.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "RiuViewerCommands.h"

#include "RivWellPathSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateTextAnnotationPickEventHandler::RicCreateTextAnnotationPickEventHandler(RimTextAnnotation* textAnnotation)
    : m_annotationToEdit(textAnnotation)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateTextAnnotationPickEventHandler::~RicCreateTextAnnotationPickEventHandler() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationPickEventHandler::notifyUnregistered()
{
    m_annotationToEdit->enableTargetPointPicking(false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateTextAnnotationPickEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    if (!caf::SelectionManager::instance()->isSelected(m_annotationToEdit.p(), 0))
    {
        m_annotationToEdit->enableTargetPointPicking(false);

        return false;
    }

    if (m_annotationToEdit)
    {
        Rim3dView* rimView             = eventObject.m_view;
        cvf::Vec3d targetPointInDomain = cvf::Vec3d::ZERO;

        // If clicked on an other well path, snap target point to well path center line
        auto firstPickItem      = eventObject.m_pickItemInfos.front();
        auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(firstPickItem.sourceInfo());

        auto intersectionPointInDomain =
            rimView->displayCoordTransform()->transformToDomainCoord(firstPickItem.globalPickedPoint());
        bool   doSetAzimuthAndInclination = false;
        double azimuth                    = 0.0;
        double inclination                = 0.0;

        if (wellPathSourceInfo)
        {
            targetPointInDomain =
                wellPathSourceInfo->closestPointOnCenterLine(firstPickItem.faceIdx(), intersectionPointInDomain);

            double md                  = wellPathSourceInfo->measuredDepth(firstPickItem.faceIdx(), intersectionPointInDomain);
            doSetAzimuthAndInclination = calculateAzimuthAndInclinationAtMd(
                md, wellPathSourceInfo->wellPath()->wellPathGeometry(), &azimuth, &inclination);
        }
        else
        {
            targetPointInDomain        = intersectionPointInDomain;
            doSetAzimuthAndInclination = false;
        }

        if (!m_annotationToEdit->firstActiveTarget())
        {
            m_annotationToEdit->setReferencePointXyz(targetPointInDomain);

            if (wellPathSourceInfo)
            {
                double mdrkbAtFirstTarget = wellPathSourceInfo->measuredDepth(firstPickItem.faceIdx(), intersectionPointInDomain);

                RimModeledWellPath* modeledWellPath = dynamic_cast<RimModeledWellPath*>(wellPathSourceInfo->wellPath());
                if (modeledWellPath)
                {
                    mdrkbAtFirstTarget += modeledWellPath->geometryDefinition()->mdrkbAtFirstTarget();
                }

                m_annotationToEdit->setMdrkbAtFirstTarget(mdrkbAtFirstTarget);
            }
        }

        cvf::Vec3d referencePoint     = m_annotationToEdit->referencePointXyz();
        cvf::Vec3d relativeTagetPoint = targetPointInDomain - referencePoint;

        RimWellPathTarget* newTarget = new RimWellPathTarget;

        if (doSetAzimuthAndInclination)
        {
            newTarget->setAsPointXYZAndTangentTarget(
                cvf::Vec3d(relativeTagetPoint.x(), relativeTagetPoint.y(), relativeTagetPoint.z()), azimuth, inclination);
        }
        else
        {
            newTarget->setAsPointTargetXYD(cvf::Vec3d(relativeTagetPoint.x(), relativeTagetPoint.y(), -relativeTagetPoint.z()));
        }

        m_annotationToEdit->insertTarget(nullptr, newTarget);

        m_annotationToEdit->updateConnectedEditors();
        m_annotationToEdit->updateWellPathVisualization();

        return true; // Todo: See if we really should eat the event instead
    }

    return false;
}

