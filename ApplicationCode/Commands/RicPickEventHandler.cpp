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
#include "RicPickEventHandler.h"
#include "RiuViewerCommands.h"

#include <typeinfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPickEventHandler::registerAsPickEventHandler()
{
    RiuViewerCommands::setPickEventHandler(this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPickEventHandler::unregisterAsPickEventHandler()
{
    RiuViewerCommands::removePickEventHandlerIfActive(this);
}

//--------------------------------------------------------------------------------------------------
/// Override from caf::PickEventHandler. Translates to a 3d Pick event.
//--------------------------------------------------------------------------------------------------
bool RicPickEventHandler::handlePickEvent(const caf::PickEvent& eventObject)
{
    try
    {
        const Ric3dPickEvent& eventObject3d = dynamic_cast<const Ric3dPickEvent&>(eventObject);
        return handle3dPickEvent(eventObject3d);
    }
    catch (const std::bad_cast&)
    {
        return false;
    }
}
