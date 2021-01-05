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

#include "RiuQwtPlotCurveDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiuQwtPlotCurveDefines::CurveInterpolationEnum>::setUp()
{
    addItem( RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT,
             "INTERPOLATION_POINT_TO_POINT",
             "Point to Point" );
    addItem( RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT, "INTERPOLATION_STEP_LEFT", "Step Left" );

    setDefault( RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT );
}

template <>
void caf::AppEnum<RiuQwtPlotCurveDefines::LineStyleEnum>::setUp()
{
    addItem( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE, "STYLE_NONE", "None" );
    addItem( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID, "STYLE_SOLID", "Solid" );
    addItem( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH, "STYLE_DASH", "Dashes" );
    addItem( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DOT, "STYLE_DOT", "Dots" );
    addItem( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH_DOT, "STYLE_DASH_DOT", "Dashes and Dots" );

    setDefault( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotCurveDefines::zDepthForIndex( ZIndex index )
{
    switch ( index )
    {
        case RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_CURVE:
            return 100;
            break;
        case RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_STAT_CURVE:
            return 200;
            break;
        case RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_NON_OBSERVED:
            return 300;
            break;
        case RiuQwtPlotCurveDefines::ZIndex::Z_ERROR_BARS:
            return 400;
            break;
        case RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_OBSERVED:
            return 500;
            break;
        default:
            break;
    }

    return 0;
}
