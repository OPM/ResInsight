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

#include "RicContourMapPickEventHandler.h"
#include "Rim3dView.h"
#include "RimContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimGeoMechContourMapView.h"

#include "RiuMainWindow.h"
#include "RivObjectSourceInfo.h"

#include "cafDisplayCoordTransform.h"

#include "cvfPart.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicContourMapPickEventHandler* RicContourMapPickEventHandler::instance()
{
    static RicContourMapPickEventHandler* singleton = new RicContourMapPickEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicContourMapPickEventHandler::handle3dPickEvent( const Ric3dPickEvent& eventObject )
{
    if ( eventObject.m_pickItemInfos.empty() ) return false;

    const RiuPickItemInfo& firstPickedItem = eventObject.m_pickItemInfos.front();
    const cvf::Part*       firstPickedPart = firstPickedItem.pickedPart();

    const RivObjectSourceInfo* sourceInfo = dynamic_cast<const RivObjectSourceInfo*>( firstPickedPart->sourceInfo() );
    if ( sourceInfo )
    {
        RimContourMapProjection* contourMap = dynamic_cast<RimContourMapProjection*>( sourceInfo->object() );
        if ( contourMap )
        {
            QString curveText;

            RiuMainWindow::instance()->selectAsCurrentItem( contourMap );

            RimGridView* view = contourMap->firstAncestorOrThisOfTypeAsserted<RimGridView>();
            if ( !view ) return false;

            const auto& firstPoint = firstPickedItem.globalPickedPoint();

            curveText += QString( "Debug point X, Y: %1, %2\n" ).arg( firstPoint.x(), 5, 'f', 0 ).arg( firstPoint.y(), 5, 'f', 0 );

            cvf::Vec2d pickedPoint;
            double     valueAtPoint = 0.0;
            if ( contourMap->checkForMapIntersection( firstPoint, &pickedPoint, &valueAtPoint ) )
            {
                curveText += QString( "%1\n" ).arg( view->createAutoName() );
                curveText += QString( "Picked Point X, Y: %1, %2\n" ).arg( pickedPoint.x(), 5, 'f', 0 ).arg( pickedPoint.y(), 5, 'f', 0 );
                curveText += QString( "Result Type: %1\n" ).arg( contourMap->resultDescriptionText() );
                curveText += QString( "Aggregated Value: %1\n" ).arg( valueAtPoint );

                contourMap->setPickPoint( pickedPoint );
            }
            else
            {
                curveText += QString( "%1\n" ).arg( view->createAutoName() );

                contourMap->setPickPoint( cvf::Vec2d::UNDEFINED );
                // view->updateDisplayModelForCurrentTimeStepAndRedraw();
            }

            RimGeoMechContourMapView* geoMechContourView = dynamic_cast<RimGeoMechContourMapView*>( view );
            RimEclipseContourMapView* eclipseContourView = dynamic_cast<RimEclipseContourMapView*>( view );
            if ( geoMechContourView )
            {
                geoMechContourView->updatePickPointAndRedraw();
            }
            else if ( eclipseContourView )
            {
                eclipseContourView->updatePickPointAndRedraw();
            }

            RiuMainWindow::instance()->setResultInfo( curveText );

            return true;
        }
    }
    return false;
}
