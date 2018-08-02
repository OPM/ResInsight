/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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
#include "cafPdmPtrField.h"

class RimFractureTemplate;

//==================================================================================================
///
//==================================================================================================
class RicCreateMultipleFracturesOptionItemUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicCreateMultipleFracturesOptionItemUi();

    void setValues(int topKOneBased, int baseKOneBased, RimFractureTemplate* fractureTemplate, double minimumSpacing);

    RimFractureTemplate* fractureTemplate() const;
    double               minimumSpacing() const;

    bool                isKLayerContained(int k) const;

private:
    virtual void
        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;

private:
    caf::PdmField<int>                     m_topKOneBased; // Eclipse uses 1-based indexing
    caf::PdmField<int>                     m_baseKOneBased; // Eclipse uses 1-based indexing
    caf::PdmPtrField<RimFractureTemplate*> m_fractureTemplate;
    caf::PdmField<double>                  m_minSpacing;
};
