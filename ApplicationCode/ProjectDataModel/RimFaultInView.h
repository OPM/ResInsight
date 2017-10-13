/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
#include "cafPdmPointer.h"

#include "cvfBase.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    

class RigFault;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFaultInView : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimFaultInView();
    virtual ~RimFaultInView();

    void                                setFaultGeometry(const RigFault* faultGeometry);
    const RigFault*                     faultGeometry() const;
    
    virtual caf::PdmFieldHandle*        userDescriptionField();
    virtual caf::PdmFieldHandle*        objectToggleField();

    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    caf::PdmField<bool>                 showFault;

    caf::PdmField<QString>              name;

    caf::PdmField<cvf::Color3f>         faultColor;

private:
    const RigFault*                           m_rigFault;
};
