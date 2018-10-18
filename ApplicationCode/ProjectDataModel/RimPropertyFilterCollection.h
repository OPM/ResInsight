/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cafPdmField.h"

//==================================================================================================
///  
///  
//==================================================================================================
class RimPropertyFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimPropertyFilterCollection();
    ~RimPropertyFilterCollection() override;

    // Fields:
    caf::PdmField<bool> isActive;

    // Methods
    virtual bool                    hasActiveFilters() const = 0; 
    virtual bool                    hasActiveDynamicFilters() const = 0; 

    virtual void                    loadAndInitializePropertyFilters() = 0;

    void                            updateDisplayModelNotifyManagedViews() const;
    virtual void                    updateIconState() = 0;

protected:
    // Overridden methods
    void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;
    caf::PdmFieldHandle*    objectToggleField() override;
};

