/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiaDefines.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimViewLinker;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewLinkerCollection : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewLinkerCollection(void);
    virtual ~RimViewLinkerCollection(void);

    caf::PdmField<bool>                 isActive;
    caf::PdmChildField<RimViewLinker*>  viewLinker;

protected:
    virtual caf::PdmFieldHandle*    objectToggleField()     { return &isActive; }
    virtual void                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                    initAfterRead();
};
