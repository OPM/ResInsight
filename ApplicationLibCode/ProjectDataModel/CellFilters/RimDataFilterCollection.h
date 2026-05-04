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

#include "RimCellFilter.h"

#include "cafPdmObjectCollection.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

class RimCase;
class RimCellRangeFilter;
class RimCombinedFilter;
class RimEclipsePropertyFilter;

//==================================================================================================
/// Case-level container of data filters. Filters configured here are shared by all views of the
/// owning case via per-view RimDataFilterInView wrappers, which expose an independent on/off
/// toggle without duplicating filter configuration. Mode-less: combine semantics live in
/// RimCombinedFilter children when needed.
//==================================================================================================
class RimDataFilterCollection : public caf::PdmObjectCollection<RimCellFilter>
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> filtersChanged; // add / remove / reorder / child changed

    RimDataFilterCollection();
    ~RimDataFilterCollection() override;

    void     setCase( RimCase* srcCase );
    RimCase* ownerCase() const;

    std::vector<RimCellFilter*> filters() const { return items(); }
    void                        addFilter( RimCellFilter* f ) { addItem( f ); }
    void                        removeFilter( RimCellFilter* f );

    RimEclipsePropertyFilter* addNewPropertyFilter();
    RimCellRangeFilter*       addNewRangeFilter();
    RimCombinedFilter*        addNewCombinedFilter();

    bool hasActiveFilters() const;

protected:
    void onItemsChanged() override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;
    void initAfterRead() override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    void connectChildSignal( RimCellFilter* child );
    void onChildFilterChanged( const caf::SignalEmitter* emitter );

    caf::PdmPtrField<RimCase*> m_srcCase;
};
