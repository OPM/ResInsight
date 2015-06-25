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

#include "RimCellPropertyFilter.h"

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellPropertyFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCellPropertyFilterCollection();
    virtual ~RimCellPropertyFilterCollection();


    // Fields:
    caf::PdmField<bool> active;
    caf::PdmPointersField<RimEclipsePropertyFilter*> propertyFilters;

    // Methods
    RimEclipsePropertyFilter*  createAndAppendPropertyFilter();
    void                    remove(RimEclipsePropertyFilter* propertyFilter);

    bool                    hasActiveFilters() const; 
    bool                    hasActiveDynamicFilters() const; 

    void                    setReservoirView(RimEclipseView* reservoirView);
    RimEclipseView*       reservoirView();

    void                    loadAndInitializePropertyFilters();


    // Overridden methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();

protected:
    // Overridden methods
    virtual void initAfterRead();

private:
    caf::PdmPointer<RimEclipseView> m_reservoirView;
};
