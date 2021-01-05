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

namespace RiuQwtPlotCurveDefines
{
enum class CurveInterpolationEnum
{
    INTERPOLATION_POINT_TO_POINT,
    INTERPOLATION_STEP_LEFT,
};

enum class LineStyleEnum
{
    STYLE_NONE,
    STYLE_SOLID,
    STYLE_DASH,
    STYLE_DOT,
    STYLE_DASH_DOT
};

// Z index. Higher Z is painted in front
enum class ZIndex
{
    Z_ENSEMBLE_CURVE,
    Z_ENSEMBLE_STAT_CURVE,
    Z_SINGLE_CURVE_NON_OBSERVED,
    Z_ERROR_BARS,
    Z_SINGLE_CURVE_OBSERVED
};

int zDepthForIndex( ZIndex index );

}; // namespace RiuQwtPlotCurveDefines
