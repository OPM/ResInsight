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

#include "RimEclipsePropertyFilter.h"

#include "cafPdmChildArrayField.h"

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipsePropertyFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipsePropertyFilterCollection();
    virtual ~RimEclipsePropertyFilterCollection();

    // Fields:
    caf::PdmField<bool> isActive;
    caf::PdmChildArrayField<RimEclipsePropertyFilter*> propertyFilters;

    // Methods
    bool                    hasActiveFilters() const; 
    bool                    hasActiveDynamicFilters() const; 

    RimEclipseView*         reservoirView();

    void                    loadAndInitializePropertyFilters();

    void                    updateDisplayModelNotifyManagedViews();
    void                    updateIconState();

protected:
    // Overridden methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName);
    virtual void                    initAfterRead();
    virtual caf::PdmFieldHandle*    objectToggleField();
};
