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

#include "RimFracture.h"
#include "RimView.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfVector3.h"


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

    caf::PdmField<QString>                      name;
    caf::PdmPtrField<RimFractureDefinition*>    fractureDefinition;
    size_t                                      gridindex;

    virtual QList<caf::PdmOptionItemInfo>       calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual caf::PdmFieldHandle*                userDescriptionField() override;
    void                                        setIJK(size_t i, size_t j, size_t k);
    std::vector<size_t>                         getIJK() override;
    void                                        setCellCenterPosition();
    // Overrides from RimFracture
    virtual cvf::Vec3d                          centerPointForFracture() override;
    virtual RimFractureDefinition*              attachedFractureDefinition() override;

    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;


    virtual std::vector<std::pair<size_t, size_t>>  getFracturedCells() override;

protected:
    virtual void                                defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    cvf::Vec3d                                  fracturePositionForUi() const;

private:
    caf::PdmField<int>                          m_i;  //Eclipse indexing, lowest value is 1
    caf::PdmField<int>                          m_j;
    caf::PdmField<int>                          m_k;
    caf::PdmField<cvf::Vec3d>                   cellCenterPosition;
    caf::PdmProxyValueField<cvf::Vec3d>         ui_cellCenterPosition;
};
