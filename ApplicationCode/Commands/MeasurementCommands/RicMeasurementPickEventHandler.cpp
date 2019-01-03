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

#include "RicMeasurementPickEventHandler.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimIntersection.h"
#include "RimMeasurement.h"
#include "RimProject.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include "RivPartPriority.h"

#include "cvfPart.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMeasurementPickEventHandler* RicMeasurementPickEventHandler::instance()
{
    static RicMeasurementPickEventHandler* singleton = new RicMeasurementPickEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMeasurementPickEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    auto measurement = RiaApplication::instance()->project()->measurement();

    if (measurement && measurement->isInMeasurementMode())
    {
        const RiuPickItemInfo* firstGeometryPickInfo = nullptr;
        for (const auto& info : eventObject.m_pickItemInfos)
        {
            auto partCandidate = info.pickedPart();
            if (!firstGeometryPickInfo && partCandidate->priority() != RivPartPriority::PartType::Text)
            {
                firstGeometryPickInfo = &info;
            }
        }

        Rim3dView* rimView = RiaApplication::instance()->activeReservoirView();

        if (firstGeometryPickInfo && rimView)
        {
            cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();

            cvf::Vec3d domainCoord = transForm->transformToDomainCoord(firstGeometryPickInfo->globalPickedPoint());

            bool isControlButtonDown = QApplication::keyboardModifiers() & Qt::ControlModifier;

            if (!isControlButtonDown)
            {
                if (measurement->pointsInDomainCoords().size() > 1)
                {
                    measurement->removeAllPoints();
                }
            }

            measurement->addPointInDomainCoords(domainCoord);
        }

        // Further Ui processing is stopped when true is returned
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMeasurementPickEventHandler::notifyUnregistered() {}
