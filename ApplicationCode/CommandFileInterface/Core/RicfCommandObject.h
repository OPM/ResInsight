/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cafCmdFeature.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmScriptResponse.h"

#define RICF_HEADER_INIT \
    CAF_CMD_HEADER_INIT; \
    CAF_PDM_HEADER_INIT

// RICF_SOURCE_INIT calls CAF_FACTORY_REGISTER2 to avoid name conflicts with CAF_PDM_SOURCE_INIT
#define RICF_SOURCE_INIT( ClassName, CommandIdName, CommandKeyword )                             \
    const std::string& ClassName::idNameStatic()                                                 \
    {                                                                                            \
        static std::string id = CommandIdName;                                                   \
        return id;                                                                               \
    }                                                                                            \
    CAF_FACTORY_REGISTER2( caf::CmdFeature, ClassName, std::string, ClassName::idNameStatic() ); \
    CAF_PDM_SOURCE_INIT( ClassName, CommandKeyword )

//==================================================================================================
//
//
//
//==================================================================================================
class RicfCommandObject : public caf::PdmObject, public caf::PdmObjectScriptingCapability
{
public:
    RicfCommandObject();
    ~RicfCommandObject() override;

    virtual caf::PdmScriptResponse execute() = 0;
};
