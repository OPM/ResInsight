/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"

#include <QString>
#include <QDateTime>

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryTimeAxisProperties : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryTimeAxisProperties();

    caf::PdmField<int>          fontSize;
    caf::PdmField<QString>      title;
    caf::PdmField<bool>         showTitle;

    double visibleRangeMin() const;
    double visibleRangeMax() const;

    void setVisibleRangeMin(double value);
    void setVisibleRangeMax(double value);

    bool isActive() const;

protected:
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual caf::PdmFieldHandle*            objectToggleField() override;

private:
    caf::PdmField<bool>         m_isActive;
    caf::PdmField<QDateTime>    m_visibleRangeMin;
    caf::PdmField<QDateTime>    m_visibleRangeMax;
};
