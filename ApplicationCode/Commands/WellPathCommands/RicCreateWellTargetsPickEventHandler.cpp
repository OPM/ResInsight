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
#include "Rim3dView.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

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

        cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
        cvf::Vec3d domainCoord = transForm->transformToDomainCoord(eventObject.m_pickItemInfos.front().globalPickedPoint());
        if (!m_geometryToAddTargetsTo->firstActiveTarget())
        {
            m_geometryToAddTargetsTo->setReferencePoint(domainCoord);
        }
        cvf::Vec3d referencePoint = m_geometryToAddTargetsTo->referencePointXyz();
        cvf::Vec3d relativeTagetPoint = domainCoord - referencePoint;

        RimWellPathTarget* newTarget = new RimWellPathTarget;
        newTarget->setAsPointTargetXYD(cvf::Vec3d(relativeTagetPoint.x(), relativeTagetPoint.y(), -relativeTagetPoint.z()));

        m_geometryToAddTargetsTo->insertTarget(nullptr, newTarget);

        m_geometryToAddTargetsTo->updateConnectedEditors();
        m_geometryToAddTargetsTo->updateWellPathVisualization();

        return true; // Todo: See if we really should eat the event instead
    }

    return false;
}
