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

#include "RicIntersectionPickEventHandler.h"
#include "RiaApplication.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicIntersectionPickEventHandler* RicIntersectionPickEventHandler::instance()
{
    static RicIntersectionPickEventHandler* singleton = new RicIntersectionPickEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicIntersectionPickEventHandler::handle3dPickEvent( const Ric3dPickEvent& eventObject )
{
    std::vector<RimExtrudedCurveIntersection*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( selection.size() == 1 )
    {
        {
            RimExtrudedCurveIntersection* intersection = selection[0];

            RimGridView* gridView = nullptr;
            intersection->firstAncestorOrThisOfTypeAsserted( gridView );

            if ( RiaApplication::instance()->activeMainOrComparisonGridView() != gridView )
            {
                return false;
            }

            cvf::ref<caf::DisplayCoordTransform> transForm = gridView->displayCoordTransform();

            cvf::Vec3d domainCoord =
                transForm->transformToDomainCoord( eventObject.m_pickItemInfos.front().globalPickedPoint() );

            if ( intersection->inputPolyLineFromViewerEnabled() )
            {
                intersection->appendPointToPolyLine( domainCoord );

                // Further Ui processing is stopped when true is returned
                return true;
            }
            else if ( intersection->inputExtrusionPointsFromViewerEnabled() )
            {
                intersection->appendPointToExtrusionDirection( domainCoord );

                // Further Ui processing is stopped when true is returned
                return true;
            }
            else if ( intersection->inputTwoAzimuthPointsFromViewerEnabled() )
            {
                intersection->appendPointToAzimuthLine( domainCoord );

                // Further Ui processing is stopped when true is returned
                return true;
            }
        }
    }

    return false;
}
