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

#include "ContourMap/RigContourMapProjection.h"

#include "ContourMap/RimContourMapInView.h"
#include "ContourMap/RimContourMapInViewCollection.h"
#include "ContourMap/RimContourMapProjection.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"
#include "RimGeoMechContourMapView.h"
#include "RimGridView.h"

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
            // The RimContourMapProjection displayed inside a regular 3d view is owned by an internal, hidden
            // RimEclipseContourMapView/RimGeoMechContourMapView, so climbing its own ancestor chain would find that
            // hidden view rather than the visible one. Use the view the pick actually happened in instead.
            RimGridView* view = dynamic_cast<RimGridView*>( eventObject.m_view );
            if ( !view ) return false;

            // When the contour map is shown inside a regular 3d view, keep the project tree selection on the
            // RimContourMapInView object rather than jumping to the internal RimContourMapProjection object.
            RimContourMapInViewCollection* contourMapCollection = view->contourMapInViewCollection();
            RimContourMapInView*           contourMapInView     = nullptr;
            if ( contourMapCollection )
            {
                for ( RimContourMapInView* candidate : contourMapCollection->allContourMapsInView() )
                {
                    RimEclipseContourMapView* candidateSourceView = candidate->sourceItem();
                    if ( candidateSourceView && candidateSourceView->contourMapProjection() == contourMap )
                    {
                        contourMapInView = candidate;
                        break;
                    }
                }
            }

            if ( contourMapInView )
            {
                RiuMainWindow::instance()->selectAsCurrentItem( contourMapInView );
            }
            else
            {
                RiuMainWindow::instance()->selectAsCurrentItem( contourMap );
            }

            const auto& firstPickItem       = eventObject.m_pickItemInfos.front();
            auto        targetPointInDomain = view->displayCoordTransform()->transformToDomainCoord( firstPickItem.globalPickedPoint() );

            QString curveText = QString( "%1\n" ).arg( view->createAutoName() );

            cvf::Vec2d pickedPoint( cvf::Vec2d::UNDEFINED );
            double     valueAtPoint = 0.0;

            if ( contourMap->mapProjection() &&
                 contourMap->mapProjection()->checkForMapIntersection( targetPointInDomain, &pickedPoint, &valueAtPoint ) )
            {
                curveText += QString( "Picked Point X, Y: %1, %2\n" ).arg( pickedPoint.x(), 5, 'f', 0 ).arg( pickedPoint.y(), 5, 'f', 0 );
                curveText += QString( "Result Type: %1\n" ).arg( contourMap->resultDescriptionText() );
                curveText += QString( "Aggregated Value: %1\n" ).arg( valueAtPoint );
            }

            contourMap->setPickPoint( pickedPoint );

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
