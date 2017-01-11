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

#include "RimView.h"
#include "RimFracture.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include "cafPdmProxyValueField.h"

class RimFractureDefinition;
class RimWellPath;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathFracture : public RimFracture
{
     CAF_PDM_HEADER_INIT;

public:
    RimWellPathFracture(void);
    virtual ~RimWellPathFracture(void);

    caf::PdmField<QString>                      name;
    caf::PdmPtrField<RimFractureDefinition* >   fractureDefinition;

    caf::PdmField<float>                        measuredDepth;
    caf::PdmField<cvf::Vec3d>                   positionAtWellpath;
    caf::PdmProxyValueField<cvf::Vec3d>         ui_positionAtWellpath;

    caf::PdmField<int>                          i;
    caf::PdmField<int>                          j;
    caf::PdmField<int>                          k;

    virtual QList<caf::PdmOptionItemInfo>       calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual caf::PdmFieldHandle*                userDescriptionField() override;
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    // Overrides from RimFracture
    virtual cvf::Vec3d                          centerPointForFracture() override;
    virtual RimFractureDefinition*              attachedFractureDefinition() override;

    virtual std::vector<size_t>                 getIJK() override;

protected:
    virtual void                                defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    cvf::Vec3d                                  fracturePositionForUi() const;

};
