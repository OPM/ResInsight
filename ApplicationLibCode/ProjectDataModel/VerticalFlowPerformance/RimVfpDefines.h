/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

namespace RimVfpDefines
{
enum class InterpolatedVariableType
{
    BHP,
    BHP_THP_DIFF
};

enum class TableType
{
    INJECTION,
    PRODUCTION
};

enum class ProductionVariableType
{
    FLOW_RATE,
    THP,
    ARTIFICIAL_LIFT_QUANTITY,
    WATER_CUT,
    GAS_LIQUID_RATIO
};

enum class FlowingPhaseType
{
    OIL,
    GAS,
    WATER,
    LIQUID,
    INVALID
};

enum class FlowingWaterFractionType
{
    WOR,
    WCT,
    WGR,
    INVALID
};

enum class FlowingGasFractionType
{
    GOR,
    GLR,
    OGR,
    INVALID
};

enum class CurveMatchingType
{
    EXACT,
    CLOSEST_MATCH_FAMILY,
};

enum class CurveOptionValuesType
{
    MAIN_TABLE,
    UNION_OF_SELECTED_TABLES,
};

}; // namespace RimVfpDefines
