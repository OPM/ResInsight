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

#include "RimCommandRouter.h"

#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimCommandRouter, "RimCommandRouter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCommandRouter::RimCommandRouter()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "CommandRouter",
                                                    "",
                                                    "",
                                                    "",
                                                    "CommandRouter",
                                                    "The CommandRouter is used to call code independent to the "
                                                    "project" );
}

//--------------------------------------------------------------------------------------------------
///
/// RimCommandRouterMethod
///
///
///
///
//--------------------------------------------------------------------------------------------------
RimCommandRouterMethod::RimCommandRouterMethod( PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCommandRouterMethod::isNullptrValidResult() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCommandRouterMethod::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimCommandRouterMethod::defaultResult() const
{
    return nullptr;
}
