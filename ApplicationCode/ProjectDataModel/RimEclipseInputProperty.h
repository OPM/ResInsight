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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafAppEnum.h"

class QString;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimEclipseInputProperty : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseInputProperty();
    ~RimEclipseInputProperty() override;

    enum ResolveState
    {
        UNKNOWN,
        FILE_MISSING,
        KEYWORD_NOT_IN_FILE,
        RESOLVED_NOT_SAVED,
        RESOLVED
    };
    typedef  caf::AppEnum<ResolveState> ResolveStateEnum;

    // Fields:                        
    caf::PdmField<QString>          resultName;
    caf::PdmField<QString>          eclipseKeyword;
    caf::PdmField<QString>          fileName; // ReadOnly Serves as key to syncronize read eclipse prop data and this inputProp object.
    caf::PdmField<ResolveStateEnum> resolvedState; // ReadOnly and not writable

    // PdmObject Overrides
    caf::PdmFieldHandle*    userDescriptionField() override  { return &resultName;}
    void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
};
