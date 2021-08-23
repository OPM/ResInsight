/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "cafPdmObject.h"
#include "cafPdmObjectMethod.h"

//--------------------------------------------------------------------------------------------------
/// Usually, command router classes will require only the execute() method. Derive from RimCommandRouterMethod to get
/// default values for the other required virtual functions
//--------------------------------------------------------------------------------------------------
class RimCommandRouterMethod : public caf::PdmObjectMethod
{
public:
    RimCommandRouterMethod( PdmObjectHandle* self );

    bool                             isNullptrValidResult() const override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;
};

//==================================================================================================
/// The command router object is used as a hub for data processing commands independent to a ResInsight project
/// (RimProject). The intention for this object is to have a hub to connect to when using ResInsight as a data
/// processing server. Avoid dependency on a GUI, and make sure the execute() commands works in headless mode.
///
/// The router object is made available from Python using
///
///     resinsight = rips.Instance.find()
///     command_router = resinsight.command_router
///
/// Steps to add a new processing function
/// 1) Create a new class deriving from RimCommandRouterMethod
/// 2) Add Pdm fields to this class. These fields will be made available as parameters from Python
/// 2) Implement data processing in execute() method using the input values specified in the Pdm fields
///
/// Example : RimcCommandRouter_extractSurfaces
///
//==================================================================================================
class RimCommandRouter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCommandRouter();
};
