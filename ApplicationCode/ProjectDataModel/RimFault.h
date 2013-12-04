/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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
#include "cvfColor3.h"
#include "cafPdmFieldCvfColor.h"

class RigFault;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFault : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimFault();
    virtual ~RimFault();

    void                                setFaultGeometry(const RigFault* faultGeometry);
    const RigFault*                     faultGeometry() const;
    
    virtual caf::PdmFieldHandle*        userDescriptionField();
    virtual caf::PdmFieldHandle*        objectToggleField();

    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    caf::PdmField<bool>                 showFault;

    caf::PdmField<QString>              name;
    caf::PdmField<bool>                 showFaultLabel;

    caf::PdmField<bool>                 showFaultColor;
    caf::PdmField<cvf::Color3f>         faultColor;


private:
    const RigFault*                           m_rigFault;
};
