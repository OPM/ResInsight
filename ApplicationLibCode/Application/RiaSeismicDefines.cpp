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
    addItem( RiaDefines::SeismicSectionType::CS_INLINE, "CS_INLINE", "Inline" );
    addItem( RiaDefines::SeismicSectionType::CS_XLINE, "CS_XLINE", "Crossline" );
    addItem( RiaDefines::SeismicSectionType::CS_DEPTHSLICE, "CS_DEPTHSLICE", "Depth Slice" );
    addItem( RiaDefines::SeismicSectionType::CS_POLYLINE, "CS_POLYLINE", "Polyline" );
    setDefault( RiaDefines::SeismicSectionType::CS_INLINE );
}
} // namespace caf
