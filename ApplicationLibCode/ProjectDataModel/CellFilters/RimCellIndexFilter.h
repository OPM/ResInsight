/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCellFilter.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimCase;
class RimGeoMechCase;

class RimCellIndexFilter : public RimCellFilter
{
    CAF_PDM_HEADER_INIT;

public:
    RimCellIndexFilter();
    ~RimCellIndexFilter() override;

    void setCase( RimCase* srcCase );

    void updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex ) override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    QString fullName() const override;

    RimGeoMechCase* geoMechCase() const;

    void updateCells();

private:
    std::vector<size_t> m_cells;

    caf::PdmPtrField<RimCase*> m_case;
    caf::PdmField<int>         m_partId;
    caf::PdmField<int>         m_setId;
};
