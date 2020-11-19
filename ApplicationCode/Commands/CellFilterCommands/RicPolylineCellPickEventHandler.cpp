/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020     Equinor ASA
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

#include "RicPolylineCellPickEventHandler.h"

#include "RiaOffshoreSphericalCoords.h"

#include "Rim3dView.h"
#include "RimPolylineFilter.h"
#include "RimPolylineTarget.h"

#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolylineCellPickEventHandler::RicPolylineCellPickEventHandler( RimPolylineFilter* filterDef )
    : m_filterDef( filterDef )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolylineCellPickEventHandler::~RicPolylineCellPickEventHandler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineCellPickEventHandler::registerAsPickEventHandler()
{
    RiuViewer::setHoverCursor( Qt::CrossCursor );
    Ric3dViewPickEventHandler::registerAsPickEventHandler();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineCellPickEventHandler::notifyUnregistered()
{
    RiuViewer::clearHoverCursor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPolylineCellPickEventHandler::handle3dPickEvent( const Ric3dPickEvent& eventObject )
{
    if ( m_filterDef )
    {
        Rim3dView* rimView = eventObject.m_view;

        auto firstPickItem = eventObject.m_pickItemInfos.front();
        auto targetPointInDomain =
            rimView->displayCoordTransform()->transformToDomainCoord( firstPickItem.globalPickedPoint() );

        cvf::Vec3d( targetPointInDomain.x(), targetPointInDomain.y(), -targetPointInDomain.z() );

        auto* newTarget = new RimPolylineTarget();
        newTarget->setAsPointTargetXYD(
            cvf::Vec3d( targetPointInDomain.x(), targetPointInDomain.y(), -targetPointInDomain.z() ) );

        m_filterDef->insertTarget( nullptr, newTarget );
        m_filterDef->updateConnectedEditors();
        m_filterDef->updateVisualization();

        return true;
    }

    return false;
}
