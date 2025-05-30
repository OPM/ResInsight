/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaDefines.h"
#include "cafColorTable.h"
#include <map>

//==================================================================================================
///
///
//==================================================================================================
class RiaColorTables
{
public:
    using WellPathComponentColors = std::map<RiaDefines::WellPathComponentType, cvf::Color3::ColorIdent>;

    static const caf::ColorTable& normalPaletteColors();
    static const caf::ColorTable& normalPaletteOppositeOrderingColors();
    static const caf::ColorTable& blackWhitePaletteColors();
    static const caf::ColorTable& whiteBlackPaletteColors();
    static const caf::ColorTable& pinkWhitePaletteColors();
    static const caf::ColorTable& whitePinkPaletteColors();
    static const caf::ColorTable& blueWhiteRedPaletteColors();
    static const caf::ColorTable& redWhiteBluePaletteColors();
    static const caf::ColorTable& categoryPaletteColors();
    static const caf::ColorTable& contrastCategoryPaletteColors();
    static const caf::ColorTable& tensorWhiteGrayBlackPaletteColors();
    static const caf::ColorTable& tensorOrangeBlueWhitePaletteColors();
    static const caf::ColorTable& tensorsMagentaBrownGrayPaletteColors();
    static const caf::ColorTable& angularPaletteColors();
    static const caf::ColorTable& rainbowPaletteColors();
    static const caf::ColorTable& stimPlanPaletteColors();
    static const caf::ColorTable& faultsPaletteColors();
    static const caf::ColorTable& wellsPaletteColors();
    static const caf::ColorTable& summaryCurveDefaultPaletteColors();
    static const caf::ColorTable& summaryCurveRedPaletteColors();
    static const caf::ColorTable& summaryCurveGreenPaletteColors();
    static const caf::ColorTable& summaryCurveBluePaletteColors();
    static const caf::ColorTable& summaryCurveBrownPaletteColors();
    static const caf::ColorTable& summaryCurveNoneRedGreenBlueBrownPaletteColors();
    static const caf::ColorTable& wellLogPlotPaletteColors();
    static const caf::ColorTable& selectionPaletteColors();
    static const caf::ColorTable& timestepsPaletteColors();
    static const caf::ColorTable& editableWellPathsPaletteColors();
    static const caf::ColorTable& wellPathsPaletteColors();
    static const caf::ColorTable& waterAndRockPaletteColors();
    static const caf::ColorTable& correlationPaletteColors();
    static const caf::ColorTable& heatMapPaletteColors();
    static const caf::ColorTable& curveSetPaletteColors();

    static cvf::Color3f undefinedCellColor();

    static WellPathComponentColors wellPathComponentColors();

    // Colors used in WebViz
    static const caf::ColorTable& structuralUncertaintyColors();

    // Default 3d View colors
    static cvf::Color3f defaultGridLineColor();
    static cvf::Color3f defaultFaultLineColor();
    static cvf::Color3f defaultWellLabelColor();
    static cvf::Color3f defaultViewerBackgroundColor();

    static caf::ColorTable createBrightnessBasedColorTable( cvf::Color3ub baseColor, int brightnessLevelCount );

    static std::map<RiaDefines::PhaseType, caf::ColorTable> phaseColors();
    static cvf::Color3f                                     phaseColor( RiaDefines::PhaseType phase );

    static cvf::Color3f historyCurveContrastColor();

private:
    static std::vector<cvf::Color3ub> categoryColors();
    static std::vector<cvf::Color3ub> contrastCategoryColors();
    static std::vector<cvf::Color3ub> invertedCategoryColors();
};
