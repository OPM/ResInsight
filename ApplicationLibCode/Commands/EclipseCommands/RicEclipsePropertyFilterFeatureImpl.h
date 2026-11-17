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

#include <cstddef>
#include <vector>

class RimCombinedFilter;
class RimEclipsePropertyFilter;
class RimEclipsePropertyFilterCollection;

namespace caf
{
class PdmObjectHandle;
}

//==================================================================================================
///
//==================================================================================================
class RicEclipsePropertyFilterFeatureImpl
{
public:
    static std::vector<RimEclipsePropertyFilter*>           selectedPropertyFilters();
    static std::vector<RimEclipsePropertyFilterCollection*> selectedPropertyFilterCollections();

    // Looks at the strict selection first, then at any selected RimFilterInViewCollection facade. Returns
    // nullptr if neither is selected. Used by the property-filter "New ..." commands so they fire
    // when invoked from the unified facade tree node.
    static RimEclipsePropertyFilterCollection* resolveTargetPropertyFilterCollection();

    // When formationNames is true, the new filter is a formation filter instead of a filter on the
    // view's current cell result.
    static void addPropertyFilter( RimEclipsePropertyFilterCollection* propertyFilterCollection, bool formationNames = false );
    static RimEclipsePropertyFilter* addPropertyFilterToCombinedFilter( RimCombinedFilter* combined, bool formationNames = false );
    static void                      insertPropertyFilter( RimEclipsePropertyFilterCollection* propertyFilterCollection, size_t index );

    static bool isPropertyFilterCommandAvailable( caf::PdmObjectHandle* object );

private:
    static void setDefaults( RimEclipsePropertyFilter* propertyFilter );
};
