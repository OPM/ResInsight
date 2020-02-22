/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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

#include "RicBoxManipulatorEventHandler.h"

#include "cafBoxManipulatorPartManager.h"
#include "cafEffectGenerator.h"
#include "cafViewer.h"

#include "cvfCamera.h"
#include "cvfDrawableGeo.h"
#include "cvfHitItemCollection.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRay.h"
#include "cvfRayIntersectSpec.h"

#include <QDebug>
#include <QMouseEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicBoxManipulatorEventHandler::RicBoxManipulatorEventHandler( caf::Viewer* viewer )
    : m_viewer( viewer )
{
    m_partManager = new caf::BoxManipulatorPartManager;

    m_viewer->installEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicBoxManipulatorEventHandler::~RicBoxManipulatorEventHandler()
{
    if ( m_viewer ) m_viewer->removeEventFilter( this );

    for ( auto viewer : m_otherViewers )
    {
        if ( viewer )
        {
            m_viewer->removeEventFilter( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::registerInAdditionalViewer( caf::Viewer* viewer )
{
    if ( viewer )
    {
        m_otherViewers.push_back( viewer );
        viewer->installEventFilter( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::setOrigin( const cvf::Vec3d& origin )
{
    m_partManager->setOrigin( origin );

    emit notifyRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::setSize( const cvf::Vec3d& size )
{
    m_partManager->setSize( size );

    emit notifyRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::appendPartsToModel( cvf::ModelBasicList* model )
{
    m_partManager->appendPartsToModel( model );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicBoxManipulatorEventHandler::eventFilter( QObject* obj, QEvent* inputEvent )
{
    caf::Viewer* viewer = dynamic_cast<caf::Viewer*>( obj );

    if ( !viewer ) return false;

    if ( inputEvent->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( inputEvent );

        if ( mouseEvent->button() == Qt::LeftButton )
        {
            cvf::HitItemCollection hitItems;
            if ( viewer->rayPick( mouseEvent->x(), mouseEvent->y(), &hitItems ) )
            {
                m_partManager->tryToActivateManipulator( hitItems.firstItem() );

                if ( m_partManager->isManipulatorActive() )
                {
                    emit notifyRedraw();

                    return true;
                }
            }
        }
    }
    else if ( inputEvent->type() == QEvent::MouseMove )
    {
        if ( m_partManager->isManipulatorActive() )
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( inputEvent );

            cvf::ref<cvf::RayIntersectSpec> rayIS =
                viewer->rayIntersectSpecFromWindowCoordinates( mouseEvent->pos().x(), mouseEvent->pos().y() );

            if ( rayIS.notNull() )
            {
                m_partManager->updateManipulatorFromRay( rayIS->ray() );

                cvf::Vec3d origin;
                cvf::Vec3d size;
                m_partManager->originAndSize( &origin, &size );

                emit notifyUpdate( origin, size );

                emit notifyRedraw();

                return true;
            }
        }
    }
    else if ( inputEvent->type() == QEvent::MouseButtonRelease )
    {
        if ( m_partManager->isManipulatorActive() )
        {
            m_partManager->endManipulator();

            cvf::Vec3d origin;
            cvf::Vec3d size;
            m_partManager->originAndSize( &origin, &size );

            emit notifyUpdate( origin, size );

            return true;
        }
    }

    return false;
}
