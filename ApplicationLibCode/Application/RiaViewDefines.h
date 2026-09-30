/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "enum_bitmask.hpp"

namespace RiaDefines
{
enum class View3dContent
{
    NONE              = 0b00000000,
    ECLIPSE_DATA      = 0b00000001,
    GEOMECH_DATA      = 0b00000010,
    FLAT_INTERSECTION = 0b00000100,
    CONTOUR           = 0b00001000,
    SEISMIC           = 0b00010000,
    DATA_OBJECTS      = 0b00100000,
    ALL               = 0b00111111
};

enum class ItemIn3dView
{
    NONE        = 0b00000000,
    SURFACE     = 0b00000001,
    POLYGON     = 0b00000010,
    CONTOUR_MAP = 0b00000100,
    WELL_PATH   = 0b00001000,
    ALL         = 0b00001111
};
}; // namespace RiaDefines

// Activate bit mask operators at global scope
ENABLE_BITMASK_OPERATORS( RiaDefines::View3dContent )
ENABLE_BITMASK_OPERATORS( RiaDefines::ItemIn3dView )
