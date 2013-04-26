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

#include "RimCellRangeFilter.h"

class RigActiveCellInfo;
class RigGridBase;

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellRangeFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCellRangeFilterCollection();
    virtual ~RimCellRangeFilterCollection();

    // Fields
    caf::PdmField<bool> active;
    caf::PdmField< std::list< caf::PdmPointer< RimCellRangeFilter > > > rangeFilters;

    // Methods
    RimCellRangeFilter* createAndAppendRangeFilter();
    void                remove(RimCellRangeFilter* rangeFilter);

    void                compoundCellRangeFilter(cvf::CellRangeFilter* cellRangeFilter, const RigGridBase* grid) const;
    bool                hasActiveFilters() const;

    void                setReservoirView(RimReservoirView* reservoirView);
    RimReservoirView*   reservoirView();
    RigMainGrid*        mainGrid() const;
    RigActiveCellInfo*  activeCellInfo() const;

    // Overridden methods
    virtual void                    fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );
    virtual caf::PdmFieldHandle*    objectToggleField();

protected:
    // Overridden methods
    virtual void initAfterRead();


private:
    RimReservoirView* m_reservoirView;
};
