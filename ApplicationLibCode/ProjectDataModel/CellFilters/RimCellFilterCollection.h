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
#include "cafSignal.h"

#include "cvfArray.h"

class RimCellFilter;
class RimCellIndexFilter;
class RimCellRangeFilter;
class RimPolygonFilter;
class RimUserDefinedFilter;
class RimUserDefinedIndexFilter;
class RimCase;
class RimPolygonInView;
class RimPolygon;

namespace cvf
{
class CellRangeFilter;
};

//==================================================================================================
///
///
//==================================================================================================
class RimCellFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum CombineFilterModeType
    {
        OR,
        AND
    };

    RimCellFilterCollection();
    ~RimCellFilterCollection() override;

    caf::Signal<> filtersChanged;

    RimPolygonFilter*          addNewPolygonFilter( RimCase* srcCase, RimPolygon* polygon );
    RimCellRangeFilter*        addNewCellRangeFilter( RimCase* srcCase, int gridIndex, int sliceDirection = -1, int defaultSlice = -1 );
    RimCellIndexFilter*        addNewCellIndexFilter( RimCase* srcCase );
    RimUserDefinedFilter*      addNewUserDefinedFilter( RimCase* srcCase );
    RimUserDefinedIndexFilter* addNewUserDefinedIndexFilter( RimCase* srcCase, const std::vector<size_t>& defCellIndexes = {} );

    void addFilterAndNotifyChanges( RimCellFilter* pFilter, RimCase* srcCase );
    void removeFilter( RimCellFilter* filter );
    void notifyGridReload();

    bool isEmpty() const;
    bool isActive() const;
    void setActive( bool bActive );

    bool useAndOperation() const;

    void compoundCellRangeFilter( cvf::CellRangeFilter* cellRangeFilter, size_t gridIndex ) const;
    void updateCellVisibilityByIndex( cvf::UByteArray* cellsIncluded, cvf::UByteArray* cellsExcluded, size_t gridIndex ) const;

    std::vector<RimPolygonInView*> enabledCellFilterPolygons() const;
    std::vector<RimCellFilter*>    filters() const;

    bool hasActiveFilters() const;
    bool hasActiveIncludeIndexFilters() const;
    bool hasActiveIncludeRangeFilters() const;

    void updateIconState();
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void connectToFilterUpdates( RimCellFilter* filter );

    void setCase( RimCase* theCase );

    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* objectToggleField() override;
    void                 initAfterRead() override;

    void onFilterUpdated( const SignalEmitter* emitter );

private:
    void setAutoName( RimCellFilter* pFilter );
    void addFilter( RimCellFilter* pFilter );

    caf::PdmChildArrayField<RimCellFilter*>            m_cellFilters;
    caf::PdmField<bool>                                m_isActive;
    caf::PdmField<QString>                             m_combineModeLabel;
    caf::PdmField<caf::AppEnum<CombineFilterModeType>> m_combineFilterMode;

    caf::PdmChildArrayField<RimCellRangeFilter*> m_rangeFilters_OBSOLETE;
};
