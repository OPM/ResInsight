/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RiaSeismicDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::SeismicSectionType>::setUp()
{
    addItem( RiaDefines::SeismicSectionType::SS_INLINE, "SS_INLINE", "Inline" );
    addItem( RiaDefines::SeismicSectionType::SS_XLINE, "SS_XLINE", "Crossline" );
    addItem( RiaDefines::SeismicSectionType::SS_DEPTHSLICE, "SS_DEPTHSLICE", "Depth Slice" );
    addItem( RiaDefines::SeismicSectionType::SS_POLYLINE, "SS_POLYLINE", "Polyline" );
    setDefault( RiaDefines::SeismicSectionType::SS_INLINE );
}
} // namespace caf
