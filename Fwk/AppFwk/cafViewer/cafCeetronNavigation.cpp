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

#include "cafCeetronNavigation.h"
#include "cafViewer.h"
#include "cvfCamera.h"
#include "cvfHitItemCollection.h"
#include "cvfModel.h"
#include "cvfRay.h"
#include "cvfScene.h"
#include "cvfViewport.h"

#include <QInputEvent>

using cvf::ManipulatorTrackball;

//==================================================================================================
///
/// \class CeetronNavigationPolicy
/// \ingroup Caf
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::CeetronNavigation::CeetronNavigation()
    : m_isRotCenterInitialized( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::CeetronNavigation::~CeetronNavigation()
{
}

//--------------------------------------------------------------------------------------------------
/// Repositions and orients the camera to view the rotation point along the
/// direction "alongDirection". The distance to the rotation point is maintained.
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection )
{
    m_trackball->setView( alongDirection, upDirection );
    /*
    if (m_camera.isNull()) return;

    Vec3d dir = alongDirection;
    if (!dir.normalize()) return;
    Vec3d up = upDirection;
    if(!up.normalize()) up = Vec3d::Z_AXIS;

    if((up * dir) < 1e-2) up = dir.perpendicularVector();

    Vec3d cToE = m_camera->position() - m_rotationPoint;
    Vec3d newEye = m_rotationPoint - cToE.length() * dir;

    m_camera->setFromLookAt(newEye, m_rotationPoint, upDirection);
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::init()
{
    m_trackball = new cvf::ManipulatorTrackball;
    m_trackball->setCamera( m_viewer->mainCamera() );
    m_isRotCenterInitialized = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::CeetronNavigation::handleInputEvent( QInputEvent* inputEvent )
{
    if ( !inputEvent ) return false;

    switch ( inputEvent->type() )
    {
        case QEvent::MouseButtonPress:
            mouseMoveEvent( static_cast<QMouseEvent*>( inputEvent ) );
            break;
        case QEvent::MouseButtonRelease:
            mouseReleaseEvent( static_cast<QMouseEvent*>( inputEvent ) );
            break;
        case QEvent::MouseMove:
            mouseMoveEvent( static_cast<QMouseEvent*>( inputEvent ) );
            break;
        case QEvent::Wheel:
            wheelEvent( static_cast<QWheelEvent*>( inputEvent ) );
            break;
        default:
            break;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::mouseMoveEvent( QMouseEvent* event )
{
    if ( !m_viewer->canRender() ) return;
    initializeRotationCenter();
    int posX = event->x();
    int posY = m_viewer->height() - event->y();

    ManipulatorTrackball::NavigationType navType = getNavigationTypeFromMouseButtons( event->buttons() );
    if ( navType != m_trackball->activeNavigation() )
    {
        m_trackball->startNavigation( navType, posX, posY );
    }

    bool needRedraw = m_trackball->updateNavigation( posX, posY );
    if ( needRedraw )
    {
        m_viewer->navigationPolicyUpdate();
    }

    setCursorFromCurrentState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::mousePressEvent( QMouseEvent* event )
{
    if ( !m_viewer->canRender() ) return;
    initializeRotationCenter();
    int posX = event->x();
    int posY = m_viewer->height() - event->y();

    ManipulatorTrackball::NavigationType navType = getNavigationTypeFromMouseButtons( event->buttons() );
    m_trackball->startNavigation( navType, posX, posY );

    setCursorFromCurrentState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::mouseReleaseEvent( QMouseEvent* event )
{
    if ( !m_viewer->canRender() ) return;
    initializeRotationCenter();
    ManipulatorTrackball::NavigationType navType = getNavigationTypeFromMouseButtons( event->buttons() );
    m_trackball->startNavigation( navType, event->x(), event->y() );

    setCursorFromCurrentState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::wheelEvent( QWheelEvent* event )
{
    if ( !m_viewer->canRender() ) return;
    if ( !m_viewer->mainCamera() ) return;
    initializeRotationCenter();
    int vpHeight = m_viewer->mainCamera()->viewport()->height();
    if ( vpHeight <= 0 ) return;

    int navDelta = vpHeight / 5;
    if ( event->delta() < 0 ) navDelta *= -1;

    int posY = m_viewer->height() - event->y();

    m_trackball->startNavigation( ManipulatorTrackball::WALK, event->x(), posY );

    m_trackball->updateNavigation( event->x(), posY + navDelta );
    m_trackball->endNavigation();

    m_viewer->updateParallelProjectionHeightFromMoveZoom( m_pointOfInterest );

    m_viewer->navigationPolicyUpdate();

    event->accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ManipulatorTrackball::NavigationType caf::CeetronNavigation::getNavigationTypeFromMouseButtons( Qt::MouseButtons mouseButtons )
{
    if ( mouseButtons == Qt::LeftButton )
    {
        return ManipulatorTrackball::PAN;
    }
    else if ( mouseButtons == Qt::RightButton )
    {
        return ManipulatorTrackball::ROTATE;
    }
    else if ( mouseButtons == Qt::MidButton || mouseButtons == ( Qt::LeftButton | Qt::RightButton ) )
    {
        return ManipulatorTrackball::WALK;
    }
    else
    {
        return ManipulatorTrackball::NONE;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::setCursorFromCurrentState()
{
    ManipulatorTrackball::NavigationType navType = m_trackball->activeNavigation();
    switch ( navType )
    {
        case ManipulatorTrackball::PAN:
            // m_viewer->setCursor(RiuCursors::get(RiuCursors::PAN));
            return;
        case ManipulatorTrackball::WALK:
            // m_viewer->setCursor(RiuCursors::get(RiuCursors::WALK));
            return;
        case ManipulatorTrackball::ROTATE:
            // m_viewer->setCursor(RiuCursors::get(RiuCursors::ROTATE));
            return;
        default:
            break;
    }

    // m_viewer->setCursor(RiuCursors::get(RiuCursors::PICK));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::initializeRotationCenter()
{
    if ( m_isRotCenterInitialized || m_trackball.isNull() || !m_viewer->currentScene()->boundingBox().isValid() )
    {
        return;
    }

    cvf::Vec3d pointOfInterest = m_viewer->currentScene()->boundingBox().center();

    this->setPointOfInterest( pointOfInterest );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::CeetronNavigation::pointOfInterest()
{
    initializeRotationCenter();
    return m_pointOfInterest;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::CeetronNavigation::setPointOfInterest( cvf::Vec3d poi )
{
    m_pointOfInterest = poi;
    m_trackball->setRotationPoint( poi );
    m_isRotCenterInitialized = true;
    m_viewer->updateParallelProjectionCameraPosFromPointOfInterestMove( m_pointOfInterest );
}
