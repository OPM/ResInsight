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

#include "RimPropertyFilterCollection.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class RimGeoMechPropertyFilter;
class RimGeoMechView;

//==================================================================================================
///
///
//==================================================================================================
class RimGeoMechPropertyFilterCollection : public RimPropertyFilterCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechPropertyFilterCollection();
    ~RimGeoMechPropertyFilterCollection() override;

    RimGeoMechView* reservoirView();

    // Fields:
    caf::PdmChildArrayField<RimGeoMechPropertyFilter*> propertyFilters;

    // Methods
    bool hasActiveFilters() const override;
    bool hasActiveDynamicFilters() const override;
    bool isUsingFormationNames() const;

    void loadAndInitializePropertyFilters() override;
    void updateIconState() override;

protected:
    // Overridden methods
    void initAfterRead() override;
};
