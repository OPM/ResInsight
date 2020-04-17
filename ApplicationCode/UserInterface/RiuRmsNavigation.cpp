/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuRmsNavigation.h"
#include "cafViewer.h"
#include "cvfCamera.h"
#include "cvfHitItemCollection.h"
#include "cvfManipulatorTrackball.h"
#include "cvfRay.h"
#include "cvfViewport.h"

#include <QInputEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRmsNavigation::RiuRmsNavigation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRmsNavigation::~RiuRmsNavigation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuRmsNavigation::handleInputEvent( QInputEvent* inputEvent )
{
    if ( !inputEvent ) return false;
    bool isEventHandled = false;
    switch ( inputEvent->type() )
    {
        case QEvent::MouseButtonPress:
        {
            QMouseEvent* me = static_cast<QMouseEvent*>( inputEvent );

            int translatedMousePosX, translatedMousePosY;
            cvfEventPos( me->x(), me->y(), &translatedMousePosX, &translatedMousePosY );

            if ( me->button() == Qt::MidButton && isRotationEnabled() )
            {
                this->pickAndSetPointOfInterest( me->x(), me->y() );

                m_trackball->startNavigation( cvf::ManipulatorTrackball::ROTATE, translatedMousePosX, translatedMousePosY );
                m_isNavigating                  = true;
                m_hasMovedMouseDuringNavigation = false;
                isEventHandled                  = true;
            }
            else if ( me->button() == Qt::RightButton )
            {
                if ( me->modifiers() == Qt::NoModifier )
                {
                    m_trackball->startNavigation( cvf::ManipulatorTrackball::PAN, translatedMousePosX, translatedMousePosY );
                    m_isNavigating                  = true;
                    m_hasMovedMouseDuringNavigation = false;
                    isEventHandled                  = true;
                }
            }
            else if ( me->button() == Qt::LeftButton )
            {
                if ( me->modifiers() == Qt::NoModifier )
                {
                    QMouseEvent* we = static_cast<QMouseEvent*>( inputEvent );
                    m_lastPosX      = we->x();
                    m_lastPosY      = we->y();

                    m_zoomRay = createZoomRay( translatedMousePosX, translatedMousePosY );

                    m_isNavigating                  = true;
                    m_hasMovedMouseDuringNavigation = false;
                    isEventHandled                  = true;
                    m_isZooming                     = true;
                }
            }
            forcePointOfInterestUpdateDuringNextWheelZoom();
        }
        break;
        case QEvent::MouseButtonRelease:
        {
            if ( m_isNavigating )
            {
                QMouseEvent* me = static_cast<QMouseEvent*>( inputEvent );
                if ( me->button() == Qt::RightButton || me->button() == Qt::MidButton )
                {
                    m_trackball->endNavigation();

                    m_isNavigating = false;
                    if ( m_hasMovedMouseDuringNavigation ) isEventHandled = true;
                    m_hasMovedMouseDuringNavigation = false;
                }
                else if ( me->button() == Qt::LeftButton )
                {
                    m_isZooming = false;

                    m_isNavigating = false;
                    if ( m_hasMovedMouseDuringNavigation ) isEventHandled = true;
                    m_hasMovedMouseDuringNavigation = false;
                }
            }
            forcePointOfInterestUpdateDuringNextWheelZoom();
        }
        break;
        case QEvent::MouseMove:
        {
            initializeRotationCenter();
            if ( m_isRotCenterInitialized )
            {
                QMouseEvent* me = static_cast<QMouseEvent*>( inputEvent );

                int translatedMousePosX, translatedMousePosY;
                cvfEventPos( me->x(), me->y(), &translatedMousePosX, &translatedMousePosY );

                if ( m_isNavigating )
                {
                    if ( m_isZooming )
                    {
                        int delta = -3 * ( m_lastPosY - me->y() );
                        this->zoomAlongRay( m_zoomRay.p(), delta );
                        m_lastPosX = me->x();
                        m_lastPosY = me->y();
                    }
                    else
                    {
                        bool needRedraw = m_trackball->updateNavigation( translatedMousePosX, translatedMousePosY );
                        if ( needRedraw )
                        {
                            m_viewer->navigationPolicyUpdate();
                        }
                    }
                    isEventHandled                  = true;
                    m_hasMovedMouseDuringNavigation = true;
                }
            }
        }
        break;
        case QEvent::Wheel:
        {
            if ( inputEvent->modifiers() == Qt::NoModifier )
            {
                QWheelEvent* we = static_cast<QWheelEvent*>( inputEvent );

                updatePointOfInterestDuringZoomIfNecessary( we->x(), we->y() );

                if ( m_isRotCenterInitialized )
                {
                    int translatedMousePosX, translatedMousePosY;
                    cvfEventPos( we->x(), we->y(), &translatedMousePosX, &translatedMousePosY );

                    cvf::ref<cvf::Ray> ray = createZoomRay( translatedMousePosX, translatedMousePosY );

                    zoomAlongRay( ray.p(), -we->delta() );
                }
                isEventHandled = true;
            }
        }
        break;
        default:
            break;
    }

    if ( isSupposedToConsumeEvents() )
        return isEventHandled;
    else
        return false;
}
