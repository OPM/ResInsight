/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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
#include "cafPdmField.h"
#include "cafAppEnum.h"

class RigMainGrid;

class RimFractureContainment : public caf::PdmObject 
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureContainment();
    ~RimFractureContainment();

    enum FaultTruncType
    {
        DISABLED,
        TRUNCATE_AT_FAULT,
        CONTINUE_IN_CONTAINMENT_ZONE
    };

    bool isEnabled() const { return m_isUsingFractureContainment();}
    bool isEclipseCellWithinContainment(const RigMainGrid* mainGrid, size_t anchorEclipseCell, size_t globalCellIndex) const;

    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

protected:

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

private:
    friend caf::AppEnum< FaultTruncType >;
    caf::PdmField< caf::AppEnum< FaultTruncType > >  m_faultTruncation;

    caf::PdmField<bool> m_isUsingFractureContainment;
    caf::PdmField<int>  m_topKLayer;
    caf::PdmField<int>  m_baseKLayer;

};



