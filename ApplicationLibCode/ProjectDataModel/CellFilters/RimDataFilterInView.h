/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimCheckableNamedObject.h"

#include "cafPdmPtrField.h"

#include "cvfArray.h"

class RigGridBase;
class RimCellFilter;

//==================================================================================================
/// Per-view delegate for a case-level RimCellFilter. Exposes only an on/off toggle (inherited
/// isChecked from RimCheckableNamedObject) and forwards applyToCellVisibility to the wrapped
/// source filter. Toggling the wrapper schedules regen on the owning view only.
//==================================================================================================
class RimDataFilterInView : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataFilterInView();
    ~RimDataFilterInView() override;

    RimCellFilter* sourceFilter() const;
    void           setSourceFilter( RimCellFilter* sourceFilter );

    bool isEvaluatable() const;

    void applyToCellVisibility( cvf::UByteArray* cellVisibility, const RigGridBase* grid, size_t timeStepIndex );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void initAfterRead() override;

private:
    void syncNameFromSource();

    caf::PdmPtrField<RimCellFilter*> m_sourceFilter;
};
