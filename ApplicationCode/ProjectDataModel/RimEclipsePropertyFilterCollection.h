/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimPropertyFilterCollection.h"

#include "cafPdmChildArrayField.h"

class RimEclipsePropertyFilter;
class RimEclipseView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipsePropertyFilterCollection : public RimPropertyFilterCollection
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipsePropertyFilterCollection();
    virtual ~RimEclipsePropertyFilterCollection();

    RimEclipseView*         reservoirView();

    // Fields:
    caf::PdmChildArrayField<RimEclipsePropertyFilter*> propertyFilters;

    // Methods
    bool                    hasActiveFilters() const; 
    bool                    hasActiveDynamicFilters() const; 
    bool                    isUsingFormationNames() const;

    void                    loadAndInitializePropertyFilters();

    void                    updateIconState();
    void                    updateFromCurrentTimeStep();

protected:
    // Overridden methods
    virtual void                    initAfterRead();
};
