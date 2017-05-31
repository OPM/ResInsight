/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafAppEnum.h"


//==================================================================================================
///  
///  
//==================================================================================================
class RimCellFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    enum FilterModeType
    {
        INCLUDE,
        EXCLUDE
    };

    RimCellFilter();
    virtual ~RimCellFilter();

    caf::PdmField<QString>  name;
    caf::PdmField<bool>     isActive;
    caf::PdmField< caf::AppEnum< FilterModeType > > filterMode;

    void updateIconState();

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField();
    virtual caf::PdmFieldHandle*            objectToggleField();
};
