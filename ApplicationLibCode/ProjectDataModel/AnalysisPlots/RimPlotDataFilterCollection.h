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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimPlotDataFilterItem;

//==================================================================================================
///
//==================================================================================================
class RimPlotDataFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> filtersChanged;

public:
    RimPlotDataFilterCollection();

    RimPlotDataFilterItem*              addFilter();
    void                                removeFilter( RimPlotDataFilterItem* filter );
    std::vector<RimPlotDataFilterItem*> filters() const;
    bool                                isActive() const;

private:
    caf::PdmFieldHandle* objectToggleField() override;

    void onFilterChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<bool>                             m_isActive;
    caf::PdmChildArrayField<RimPlotDataFilterItem*> m_filters;
};
