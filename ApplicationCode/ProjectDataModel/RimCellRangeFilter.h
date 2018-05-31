/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cafPdmFieldCvfVec3d.h"

class RigGridBase;
class RigMainGrid;
class RimCellRangeFilterCollection;
class RimEclipseView;

namespace cvf
{
    class CellRangeFilter;
    class StructGridInterface;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellRangeFilter : public RimCellFilter
{
    CAF_PDM_HEADER_INIT;
public:
    RimCellRangeFilter();
    virtual ~RimCellRangeFilter();

    caf::PdmField<int>      gridIndex;      // The index of the grid that this filter applies to
    caf::PdmField<bool>     propagateToSubGrids; // Do propagate the effects to the sub-grids

    caf::PdmField<int>      startIndexI;    // Eclipse indexing, first index is 1
    caf::PdmField<int>      startIndexJ;    // Eclipse indexing, first index is 1
    caf::PdmField<int>      startIndexK;    // Eclipse indexing, first index is 1
    caf::PdmField<int>      cellCountI;
    caf::PdmField<int>      cellCountJ;
    caf::PdmField<int>      cellCountK;

    void                    setDefaultValues();
    void                    updateActiveState();

    bool                           useIndividualCellIndices() const;
    const std::vector<cvf::Vec3d>& individualCellIndices() const;

protected:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) ;
    virtual void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName);

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly );

private:
    RimCellRangeFilterCollection*   parentContainer();
    bool                            isRangeFilterControlled() const;
    void                            computeAndSetValidValues();
    const cvf::StructGridInterface* selectedGrid();

    caf::PdmField<bool>                    m_useIndividualCellIndices;
    caf::PdmField<std::vector<cvf::Vec3d>> m_individualCellIndices;
};

