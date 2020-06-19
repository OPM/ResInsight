//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafCadNavigation.h"
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
caf::CadNavigation::CadNavigation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::CadNavigation::~CadNavigation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::CadNavigation::handleInputEvent( QInputEvent* inputEvent )
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

            if ( me->button() == Qt::MidButton && me->modifiers() == Qt::NoModifier && isRotationEnabled() )
            {
                this->pickAndSetPointOfInterest( me->x(), me->y() );

                m_trackball->startNavigation( cvf::ManipulatorTrackball::ROTATE, translatedMousePosX, translatedMousePosY );
                m_roationSensitivityCalculator.init( me );

                m_isNavigating                  = true;
                m_hasMovedMouseDuringNavigation = false;
                isEventHandled                  = true;
            }
            else if ( me->button() == Qt::MidButton && ( me->modifiers() & Qt::ShiftModifier ) )
            {
                m_trackball->startNavigation( cvf::ManipulatorTrackball::PAN, translatedMousePosX, translatedMousePosY );
                m_isNavigating                  = true;
                m_hasMovedMouseDuringNavigation = false;
                isEventHandled                  = true;
            }
            forcePointOfInterestUpdateDuringNextWheelZoom();
        }
        break;
        case QEvent::MouseButtonRelease:
        {
            if ( m_isNavigating )
            {
                QMouseEvent* me = static_cast<QMouseEvent*>( inputEvent );
                if ( me->button() == Qt::MidButton )
                {
                    m_trackball->endNavigation();

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
                    double sensitivity = m_roationSensitivityCalculator.calculateSensitivity( me );

                    m_trackball->setRotationSensitivity( sensitivity );
                    bool needRedraw = m_trackball->updateNavigation( translatedMousePosX, translatedMousePosY );
                    if ( needRedraw )
                    {
                        m_viewer->navigationPolicyUpdate();
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
