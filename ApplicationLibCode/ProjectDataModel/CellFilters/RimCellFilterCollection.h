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

class RimCellFilter;
class RimCellRangeFilter;
class RimPolygonFilter;
class RimUserDefinedFilter;
class RimCase;

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
    RimCellFilterCollection();
    ~RimCellFilterCollection() override;

    RimPolygonFilter*     addNewPolygonFilter( RimCase* srcCase );
    RimUserDefinedFilter* addNewUserDefinedFilter( RimCase* srcCase );
    RimCellRangeFilter*   addNewCellRangeFilter( RimCase* srcCase, int sliceDirection = -1, int defaultSlice = -1 );

    void removeFilter( RimCellFilter* filter );

    bool isEmpty() const;
    bool isActive() const;
    void setActive( bool bActive );

    void compoundCellRangeFilter( cvf::CellRangeFilter* cellRangeFilter, size_t gridIndex ) const;

    std::vector<RimCellFilter*> filters() const;

    bool hasActiveFilters() const;
    bool hasActiveIncludeFilters() const;

    void updateIconState();
    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void connectToFilterUpdates( RimCellFilter* filter );

    void setCase( RimCase* theCase );

protected:
    // Overridden methods
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 initAfterRead() override;
    void                 onFilterUpdated( const SignalEmitter* emitter );

private:
    void setAutoName( RimCellFilter* pFilter );
    void addFilter( RimCellFilter* pFilter );

    caf::PdmChildArrayField<RimCellFilter*> m_cellFilters;
    caf::PdmField<bool>                     m_isActive;

    caf::PdmChildArrayField<RimCellRangeFilter*> m_rangeFilters_OBSOLETE;
};
