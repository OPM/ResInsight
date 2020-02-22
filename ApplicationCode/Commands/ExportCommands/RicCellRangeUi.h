/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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
#include "cafVecIjk.h"

class RimCase;
class RigActiveCellInfo;

//==================================================================================================
///
//==================================================================================================
class RicCellRangeUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicCellRangeUi();

    void setCase( RimCase* rimCase );

    caf::VecIjk start() const;
    caf::VecIjk count() const;
    QString     gridName() const;

private:
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void               clampValues();
    void               setDefaultValues();
    RigActiveCellInfo* activeCellInfo() const;
    void               updateLegendText();

private:
    caf::PdmPtrField<RimCase*> m_case;

    caf::PdmField<int> m_gridIndex;

    caf::PdmField<int> m_startIndexI; // Eclipse indexing, first index is 1
    caf::PdmField<int> m_startIndexJ; // Eclipse indexing, first index is 1
    caf::PdmField<int> m_startIndexK; // Eclipse indexing, first index is 1
    caf::PdmField<int> m_cellCountI;
    caf::PdmField<int> m_cellCountJ;
    caf::PdmField<int> m_cellCountK;
};
