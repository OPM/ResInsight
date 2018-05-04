/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimRegularLegendConfig.h"
#include "RimEnsembleCurveSet.h"

#include <map>

class RimEnsembleCurveSetCollection;


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimEnsembleCurveSetColorManager
{
public:
    static const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray> ENSEMBLE_COLOR_RANGES;

    static const RimRegularLegendConfig::ColorRangesType DEFAULT_ENSEMBLE_COLOR_RANGE;

    static bool isEnsembleColorRange(RimRegularLegendConfig::ColorRangesType colorRange)
    {
        return ENSEMBLE_COLOR_RANGES.find(colorRange) != ENSEMBLE_COLOR_RANGES.end();
    }

    static RimRegularLegendConfig::ColorRangesType nextColorRange(RimEnsembleCurveSet* curveSet);

private:
    static RimRegularLegendConfig::ColorRangesType colorRangeByIndex(int index);

    static std::map<RimEnsembleCurveSetCollection*, int> m_nextColorIndexes;
    static std::map<RimEnsembleCurveSetCollection*, std::map<RimEnsembleCurveSet*, RimRegularLegendConfig::ColorRangesType>> m_colorCache;
};
