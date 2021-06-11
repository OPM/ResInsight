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

    static void addPropertyFilter( RimEclipsePropertyFilterCollection* propertyFilterCollection );
    static void insertPropertyFilter( RimEclipsePropertyFilterCollection* propertyFilterCollection, size_t index );

    static bool isPropertyFilterCommandAvailable( caf::PdmObjectHandle* object );

private:
    static void setDefaults( RimEclipsePropertyFilter* propertyFilter );
};
