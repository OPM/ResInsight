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

//==================================================================================================
///
///
//==================================================================================================
class RimMockModelSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimMockModelSettings();
    ~RimMockModelSettings() override;

    caf::PdmField<quint64> cellCountX;
    caf::PdmField<quint64> cellCountY;
    caf::PdmField<quint64> cellCountZ;

    caf::PdmField<quint64> totalCellCount;

    caf::PdmField<quint64> resultCount;
    caf::PdmField<quint64> timeStepCount;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
};
