/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimSurface.h"

#include "RiaDefines.h"
#include "cafPdmPtrField.h"

class RimCase;

class RimGridCaseSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCaseSurface();
    ~RimGridCaseSurface() override;

    void setCase( RimCase* sourceCase );

    bool loadData() override;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    bool updateSurfaceDataFromFile();
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmPtrField<RimCase*>                            m_case;
    caf::PdmField<caf::AppEnum<RiaDefines::GridCaseAxis>> m_sliceDirection;
};
