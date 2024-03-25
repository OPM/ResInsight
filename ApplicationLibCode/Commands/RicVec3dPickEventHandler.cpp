/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RicVec3dPickEventHandler.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
/// zOffsetFactor will be multiplied by characteristic length to yield a z-offset
//--------------------------------------------------------------------------------------------------
RicVec3dPickEventHandler::RicVec3dPickEventHandler( caf::PdmField<cvf::Vec3d>* vectorField, double zOffsetFactor )
    : m_vectorField( vectorField )
    , m_zOffsetFactor( zOffsetFactor )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicVec3dPickEventHandler::handle3dPickEvent( const Ric3dPickEvent& eventObject )
{
    const Rim3dView* rimView = eventObject.m_view;

    cvf::Vec3d pickedPosition = eventObject.m_pickItemInfos.front().globalPickedPoint();

    if ( rimView->ownerCase() )
    {
        double zPickOffset = rimView->ownerCase()->characteristicCellSize() * m_zOffsetFactor;
        pickedPosition.z() += zPickOffset;
    }

    cvf::ref<caf::DisplayCoordTransform> transForm           = rimView->displayCoordTransform();
    cvf::Vec3d                           pickedPositionInUTM = transForm->transformToDomainCoord( pickedPosition );

    pickedPositionInUTM.z() *= -1.0;

    m_vectorField->setValueWithFieldChanged( pickedPositionInUTM );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicVec3dPickEventHandler::registerAsPickEventHandler()
{
    Ric3dViewPickEventHandler::registerAsPickEventHandler();
    RiuViewer::setHoverCursor( Qt::CrossCursor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicVec3dPickEventHandler::notifyUnregistered()
{
    RiuViewer::clearHoverCursor();
}
