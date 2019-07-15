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
#include "cafPdmObject.h"
#include "RicfObjectCapability.h"
#include "RicfFieldCapability.h"
#include "RicfCommandResponse.h"

#define RICF_InitField(field, keyword, default, uiName, iconResourceName, toolTip, whatsThis) \
CAF_PDM_InitField(field, keyword, default, uiName, iconResourceName, toolTip, whatsThis); \
AddRicfCapabilityToField(field)

#define RICF_InitFieldNoDefault(field, keyword, uiName, iconResourceName, toolTip, whatsThis) \
CAF_PDM_InitFieldNoDefault(field, keyword, uiName, iconResourceName, toolTip, whatsThis); \
AddRicfCapabilityToField(field)

//==================================================================================================
//
// 
//
//==================================================================================================
class RicfCommandObject : public caf::PdmObject, public RicfObjectCapability
{
public:
    RicfCommandObject();
    ~RicfCommandObject() override;

    virtual RicfCommandResponse execute() = 0;
};


