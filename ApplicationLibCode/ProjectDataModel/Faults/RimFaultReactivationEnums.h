/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

namespace RimFaultReactivation
{

enum class ModelParts
{
    HiPart1,
    MidPart1,
    LowPart1,
    HiPart2,
    MidPart2,
    LowPart2
};

enum class GridPart
{
    PART1,
    PART2
};

enum class BorderSurface
{
    UpperSurface,
    FaultSurface,
    LowerSurface
};

enum class Boundary
{
    FarSide,
    Bottom
};

enum class ElementSets
{
    OverBurden,
    UnderBurden,
    Reservoir,
    IntraReservoir
};

} // namespace RimFaultReactivation
