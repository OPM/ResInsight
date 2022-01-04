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
void caf::AppEnum<RimIntersectionDepthCutEnum>::setUp()
{
    addItem( RimIntersectionDepthCutEnum::INTERSECT_SHOW_ALL, "INTERSECT_SHOW_ALL", "Show All" );
    addItem( RimIntersectionDepthCutEnum::INTERSECT_SHOW_ABOVE, "INTERSECT_SHOW_ABOVE", "Show Above Threshold" );
    addItem( RimIntersectionDepthCutEnum::INTERSECT_SHOW_BELOW, "INTERSECT_SHOW_BELOW", "Show Below Threshold" );
    setDefault( RimIntersectionDepthCutEnum::INTERSECT_SHOW_ALL );
}

} // namespace caf
