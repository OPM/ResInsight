/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "RimView.h"
#include "cvfVector3.h"

#include "RimFracture.h"

class RimFractureDefinition;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSimWellFracture : public RimFracture
{
     CAF_PDM_HEADER_INIT;

public:
    RimSimWellFracture(void);
    virtual ~RimSimWellFracture(void);

    caf::PdmField<QString>                              name;
    caf::PdmPtrField<RimFractureDefinition* >           fractureDefinition;

    size_t                          gridindex;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    void                                  setijk(size_t i, size_t j, size_t k);


    // Overrides from RimFracture
    virtual cvf::Vec3d              centerPointForFracture() override;
    virtual RimFractureDefinition*  attachedFractureDefinition() override;

protected:
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

private:
    caf::PdmField<int>              m_i;  //Eclipse indexing, lowest value is 1
    caf::PdmField<int>              m_j;
    caf::PdmField<int>              m_k;



};
