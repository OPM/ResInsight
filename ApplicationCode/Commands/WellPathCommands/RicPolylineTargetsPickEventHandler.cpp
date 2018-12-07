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
        return false;
    }

    if (m_polylineDef)
    {
        Rim3dView* rimView             = eventObject.m_view;

        auto firstPickItem       = eventObject.m_pickItemInfos.front();
        auto targetPointInDomain = rimView->displayCoordTransform()->transformToDomainCoord(firstPickItem.globalPickedPoint());

        auto* newTarget = new RimPolylineTarget();
        newTarget->setAsPointTargetXYD(cvf::Vec3d(targetPointInDomain.x(), targetPointInDomain.y(), -targetPointInDomain.z()));

        m_polylineDef->insertTarget(nullptr, newTarget);
        m_polylineDef->updateConnectedEditors();
        m_polylineDef->updateVisualization();

        return true;
    }

    return false;
}

