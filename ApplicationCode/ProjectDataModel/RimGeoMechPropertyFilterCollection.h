/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
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
#include "cafPdmPointer.h"

class RimGeoMechPropertyFilter;
class RimGeoMechView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechPropertyFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimGeoMechPropertyFilterCollection();
    virtual ~RimGeoMechPropertyFilterCollection();

    void                  setReservoirView(RimGeoMechView* reservoirView);
    RimGeoMechView*       reservoirView();


    // Fields:
    caf::PdmField<bool> active;
    caf::PdmPointersField<RimGeoMechPropertyFilter*> propertyFilters;

    // Methods
    RimGeoMechPropertyFilter*  createAndAppendPropertyFilter();
    void                    remove(RimGeoMechPropertyFilter* propertyFilter);

    bool                    hasActiveFilters() const; 
    bool                    hasActiveDynamicFilters() const; 


    void                    loadAndInitializePropertyFilters();


    // Overridden methods
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();

protected:
    // Overridden methods
    virtual void initAfterRead();

private:
    caf::PdmPointer<RimGeoMechView> m_reservoirView;
};
