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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimCellFilter;
class RimCase;
class RigPolyLinesData;

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

    RimCellFilter* addNewPolylineFilter( RimCase* srcCase );
    RimCellFilter* addNewUserDefinedFilter( RimCase* srcCase );

    bool isEmpty();
    bool isActive();

    // Methods
    // virtual bool hasActiveFilters() const        = 0;
    // virtual bool hasActiveDynamicFilters() const = 0;

    // virtual void loadAndInitializePropertyFilters() = 0;

    // void         updateDisplayModelNotifyManagedViews( RimPropertyFilter* changedFilter ) const;
    void updateIconState();
    // void         onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
    //                             std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    // Overridden methods
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    caf::PdmFieldHandle* objectToggleField() override;

private:
    caf::PdmChildArrayField<RimCellFilter*> m_cellFilters;
    caf::PdmField<bool>                     m_isActive;
};
