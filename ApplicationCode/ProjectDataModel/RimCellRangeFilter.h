/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "RimCellFilter.h"

class RimReservoirView;

namespace cvf
{
    class CellRangeFilter;
}

class RimCellRangeFilterCollection;
class RigMainGrid;

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

    void setParentContainer(RimCellRangeFilterCollection* parentContainer);
    RimCellRangeFilterCollection* parentContainer();
    void setDefaultValues();

    caf::PdmField<int>      startIndexI;    // Eclipse indexing, first index is 1
    caf::PdmField<int>      startIndexJ;    // Eclipse indexing, first index is 1
    caf::PdmField<int>      startIndexK;    // Eclipse indexing, first index is 1
    caf::PdmField<int>      cellCountI;
    caf::PdmField<int>      cellCountJ;
    caf::PdmField<int>      cellCountK;

    void computeAndSetValidValues();

    virtual void            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

protected:
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

private:
    RimCellRangeFilterCollection* m_parentContainer;
};


