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
#pragma once

#include "RicPickEventHandler.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"

//==================================================================================================
/// A temporary, dynamic pick handler that overrides the default ones
//==================================================================================================
class Ric3dViewPickEventHandler : public caf::PickEventHandler
{
public:
    // Override from caf
    void         registerAsPickEventHandler() override;
    void         unregisterAsPickEventHandler() override;
    bool         handlePickEvent( const caf::PickEvent& eventObject ) override;
    virtual bool handle3dPickEvent( const Ric3dPickEvent& eventObject ) = 0;
};
