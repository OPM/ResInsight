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

#include "cafPdmChildArrayField.h"
#include "cafPdmPtrField.h"

class RimCellFilter;
class RimDataFilterCollection;
class RimDataFilterInView;

//==================================================================================================
/// View-level collection of RimDataFilterInView wrappers, one per case-level filter. Auto-syncs
/// with the case-level RimDataFilterCollection (mirroring RimSurfaceInViewCollection's pattern):
/// new wrappers default to checked, removed source filters drop their wrappers, and the wrapper
/// list follows the case order.
//==================================================================================================
class RimDataFilterInViewCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataFilterInViewCollection();
    ~RimDataFilterInViewCollection() override;

    void                     setSourceCollection( RimDataFilterCollection* sourceCollection );
    RimDataFilterCollection* sourceCollection() const;

    void syncWithSource();

    std::vector<RimDataFilterInView*> wrappers() const;
    std::vector<RimDataFilterInView*> activeWrappers() const;

    bool hasActiveFilters() const;

protected:
    void initAfterRead() override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    RimDataFilterInView* findWrapperFor( RimCellFilter* sourceFilter ) const;
    void                 connectSourceSignal();
    void                 onSourceFiltersChanged( const caf::SignalEmitter* emitter );
    void                 scheduleViewRegen();

    caf::PdmChildArrayField<RimDataFilterInView*> m_wrappers;
    caf::PdmPtrField<RimDataFilterCollection*>    m_sourceCollection;
};
