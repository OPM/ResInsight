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

#include "RicIntersectionViewerEventHandler.h"
#include "RimIntersection.h"
#include "Rim3dView.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicIntersectionViewerEventHandler* RicIntersectionViewerEventHandler::instance()
{
    static RicIntersectionViewerEventHandler* singleton = new RicIntersectionViewerEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicIntersectionViewerEventHandler::handleEvent(cvf::Object* eventObject)
{
    std::vector<RimIntersection*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() == 1)
    {
        RicViewerEventObject* polylineUiEvent = dynamic_cast<RicViewerEventObject*>(eventObject);
        if (polylineUiEvent)
        {
            RimIntersection* intersection = selection[0];

            Rim3dView* rimView = nullptr;
            intersection->firstAncestorOrThisOfType(rimView);
            CVF_ASSERT(rimView);

            cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
            cvf::Vec3d domainCoord = transForm->transformToDomainCoord(polylineUiEvent->globalIntersectionPoint);

            if (intersection->inputPolyLineFromViewerEnabled())
            {
                intersection->appendPointToPolyLine(domainCoord);

                // Further Ui processing is stopped when true is returned
                return true;
            }
            else if (intersection->inputExtrusionPointsFromViewerEnabled())
            {
                intersection->appendPointToExtrusionDirection(domainCoord);

                // Further Ui processing is stopped when true is returned
                return true;
            }
            else if (intersection->inputTwoAzimuthPointsFromViewerEnabled())
            {
                intersection->appendPointToAzimuthLine(domainCoord);

                // Further Ui processing is stopped when true is returned
                return true;
            }
        }
    }

    return false;
}

