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

class RimCellFilterCollection;
class RimDataFilterInViewCollection;
class RimEclipsePropertyFilterCollection;

namespace caf
{
class CmdFeatureMenuBuilder;
class SignalEmitter;
} // namespace caf

//==================================================================================================
/// View-level facade that aggregates the three filter collections (cell filters, eclipse property
/// filters, per-view data-filter wrappers) under a single tree node. Owns no children itself —
/// only holds non-owning pointers to the source collections, which remain owned by the parent view.
/// The master toggle on this node cascades to the three sources.
//==================================================================================================
class RimFilterInViewCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFilterInViewCollection();
    ~RimFilterInViewCollection() override;

    void setSourceCollections( RimCellFilterCollection*            cellFilters,
                               RimEclipsePropertyFilterCollection* propertyFilters,
                               RimDataFilterInViewCollection*      dataFiltersInView );

    RimCellFilterCollection*            cellFilters() const;
    RimEclipsePropertyFilterCollection* propertyFilters() const;
    RimDataFilterInViewCollection*      dataFiltersInView() const;

    QString activeFiltersDisplayText() const;

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    void connectSourceSignals();
    void disconnectSourceSignals();
    void onSourceFiltersChanged( const caf::SignalEmitter* emitter );
    void cascadeMasterToggle();

    caf::PdmPtrField<RimCellFilterCollection*>            m_cellFilters;
    caf::PdmPtrField<RimEclipsePropertyFilterCollection*> m_propertyFilters;
    caf::PdmPtrField<RimDataFilterInViewCollection*>      m_dataFiltersInView;
};
