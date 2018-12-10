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

#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

class Rim3dView;
class RimCellRangeFilter;

namespace cvf {
    class CellRangeFilter;
};

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellRangeFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCellRangeFilterCollection();
    ~RimCellRangeFilterCollection() override;

    // Fields
    caf::PdmField<bool> isActive;
    caf::PdmChildArrayField<RimCellRangeFilter*> rangeFilters;

    // Methods
    void                            compoundCellRangeFilter(cvf::CellRangeFilter* cellRangeFilter, size_t gridIndex) const;
    bool                            hasActiveFilters() const;
    bool                            hasActiveIncludeFilters() const;

    void                            updateDisplayModeNotifyManagedViews(RimCellRangeFilter* changedRangeFilter);
    void                            updateIconState();

protected:
    void                    fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;
    caf::PdmFieldHandle*    objectToggleField() override;

private:
    Rim3dView*                        baseView() const;
};
