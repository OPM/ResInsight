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

#include "RicPointTangentManipulator.h"

#include "RicPointTangentManipulatorPartMgr.h"
#include "RivPartPriority.h"

#include "cafPdmUiCommandSystemProxy.h"
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
RicPointTangentManipulator::RicPointTangentManipulator( caf::Viewer* viewer )
    : m_viewer( viewer )
{
    m_partManager = new RicPointTangentManipulatorPartMgr;

    m_viewer->installEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulator::~RicPointTangentManipulator()
{
    if ( m_viewer ) m_viewer->removeEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::setOrigin( const cvf::Vec3d& origin )
{
    m_partManager->setOrigin( origin );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::setTangent( const cvf::Vec3d& tangent )
{
    m_partManager->setTangent( tangent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::setHandleSize( double handleSize )
{
    m_partManager->setHandleSize( handleSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::setPolyline( const std::vector<cvf::Vec3d>& polyline )
{
    m_partManager->setPolyline( polyline );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::appendPartsToModel( cvf::ModelBasicList* model )
{
    m_partManager->appendPartsToModel( model );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPointTangentManipulator::eventFilter( QObject* obj, QEvent* inputEvent )
{
    if ( inputEvent->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( inputEvent );

        if ( mouseEvent->button() == Qt::LeftButton )
        {
            cvf::HitItemCollection hitItems;
            if ( m_viewer->rayPick( mouseEvent->x(), mouseEvent->y(), &hitItems ) )
            {
                m_partManager->tryToActivateManipulator( hitItems.firstItem() );

                if ( m_partManager->isManipulatorActive() )
                {
                    m_isDraggingInComparisonView =
                        m_viewer->isMousePosWithinComparisonView( mouseEvent->x(), mouseEvent->y() );

                    emit notifySelected();

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

            cvf::ref<cvf::RayIntersectSpec> rayIs =
                m_viewer->rayIntersectSpecFromWindowCoordinates( mouseEvent->pos().x(),
                                                                 mouseEvent->pos().y(),
                                                                 m_isDraggingInComparisonView );

            if ( !rayIs.isNull() && rayIs->ray() )
            {
                m_partManager->updateManipulatorFromRay( rayIs->ray() );

                cvf::Vec3d origin;
                cvf::Vec3d tangent;
                m_partManager->originAndTangent( &origin, &tangent );

                emit notifyUpdate( origin, tangent );

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
            cvf::Vec3d tangent;
            m_partManager->originAndTangent( &origin, &tangent );

            emit notifyUpdate( origin, tangent );
            emit notifyDragFinished();

            return true;
        }
    }

    return false;
}
