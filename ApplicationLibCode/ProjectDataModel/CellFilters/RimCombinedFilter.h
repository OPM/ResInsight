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
#include "RimCellFilterCollection.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

class RimCellIndexFilter;
class RimCellRangeFilter;
class RimPolygon;
class RimPolygonFilter;
class RimUserDefinedFilter;
class RimUserDefinedIndexFilter;

//==================================================================================================
/// A filter whose result is a boolean combination (AND / OR) of its child filters' results.
/// Child filters are evaluated to per-cell masks (each respecting its own INCLUDE/EXCLUDE mode),
/// then combined. The combined filter's own INCLUDE/EXCLUDE mode is applied to the output.
/// Nesting is supported — a combined filter may contain other combined filters. Lives only inside
/// RimEclipsePropertyFilterCollection; that's the one collection whose evaluation pipeline supplies
/// a time step, which is required for property-filter children.
//==================================================================================================
class RimCombinedFilter : public RimCellFilter
{
    CAF_PDM_HEADER_INIT;

public:
    using CombineMode = RimCellFilterCollection::CombineFilterModeType;

    RimCombinedFilter();
    ~RimCombinedFilter() override;

    void setCase( RimCase* srcCase ) override;
    bool isFilterEnabled() const override;
    void onGridChanged() override;

    void applyToCellVisibility( cvf::UByteArray* cellVisibility, const RigGridBase* grid, size_t timeStepIndex ) override;

    // Bridge from legacy collection dispatch: combined filter is declared INDEX-type, so the cell
    // filter collection's evaluation path (if ever called) would route through updateCellIndexFilter.
    void updateCompundFilter( cvf::CellRangeFilter*, int ) override {}
    void updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex ) override;

    QString fullName() const override;

    void                        addFilter( RimCellFilter* child );
    void                        removeFilter( RimCellFilter* child );
    std::vector<RimCellFilter*> filters() const;

    // Recursive introspection helpers — used by the property-filter collection so that a combined
    // filter sitting in that collection can report whether it has evaluation-worthy descendants.
    bool hasActiveEvaluatableDescendant() const;
    bool hasActiveDynamicPropertyDescendant() const;
    bool hasActiveFormationNamesPropertyDescendant() const;

    // Typed factory methods — mirror the ones on RimCellFilterCollection so features can add
    // children of the correct type directly.
    RimCellRangeFilter*        addNewCellRangeFilter( RimCase* srcCase, int gridIndex, int sliceDirection = -1, int defaultSlice = -1 );
    RimPolygonFilter*          addNewPolygonFilter( RimCase* srcCase, RimPolygon* polygon );
    RimCellIndexFilter*        addNewCellIndexFilter( RimCase* srcCase );
    RimUserDefinedFilter*      addNewUserDefinedFilter( RimCase* srcCase );
    RimUserDefinedIndexFilter* addNewUserDefinedIndexFilter( RimCase* srcCase, const std::vector<size_t>& defCellIndexes = {} );
    RimCombinedFilter*         addNewCombinedFilter( RimCase* srcCase );

    void        setCombineMode( CombineMode mode );
    CombineMode combineMode() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void onChildAdded( caf::PdmFieldHandle* containerForNewObject ) override;

private:
    bool wouldCreateCycle( RimCellFilter* candidate ) const;
    void onChildFilterChanged( const caf::SignalEmitter* emitter );

    // Combined filters only sit inside RimEclipsePropertyFilterCollection, which doesn't use
    // filterChanged signals — it requires an explicit updateDisplayModelNotifyManagedViews() call
    // to trigger a view regen. This helper performs that call.
    void notifyHostCollection();

    caf::PdmChildArrayField<RimCellFilter*>  m_filters;
    caf::PdmField<caf::AppEnum<CombineMode>> m_combineMode;
};
