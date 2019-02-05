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

#include "cafDisplayCoordTransform.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicVec3dPickEventHandler::RicVec3dPickEventHandler(caf::PdmField<cvf::Vec3d>* vectorField)
    : m_vectorField(vectorField)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicVec3dPickEventHandler::handlePickEvent(const Ric3DPickEvent& eventObject)
{
    const Rim3dView* rimView = eventObject.m_view;

    cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
    cvf::Vec3d pickedPositionInUTM = transForm->transformToDomainCoord(eventObject.m_pickItemInfos.front().globalPickedPoint());
    
    pickedPositionInUTM.z() *= -1.0;
    m_vectorField->setValueWithFieldChanged(pickedPositionInUTM);
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicVec3dPickEventHandler::notifyUnregistered()
{
}
