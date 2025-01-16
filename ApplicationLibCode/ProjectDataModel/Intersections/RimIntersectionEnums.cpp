/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimIntersectionEnums.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RimIntersectionFilterEnum>::setUp()
{
    addItem( RimIntersectionFilterEnum::INTERSECT_FILTER_NONE, "INTERSECT_SHOW_ALL", "Disabled" );
    addItem( RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE, "INTERSECT_SHOW_ABOVE", "Above" );
    addItem( RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW, "INTERSECT_SHOW_BELOW", "Below" );
    addItem( RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN, "INTERSECT_SHOW_BELOW", "Between" );
    setDefault( RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN );
}

} // namespace caf
