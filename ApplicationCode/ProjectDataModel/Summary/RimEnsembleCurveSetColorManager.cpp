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

#include "RimEnsembleCurveSetColorManager.h"
#include "RimEnsembleCurveSetCollection.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray> RimEnsembleCurveSetColorManager::ENSEMBLE_COLOR_RANGES(
    {
        { RimRegularLegendConfig::GREEN_RED,        cvf::Color3ubArray({ cvf::Color3ub(0x00, 0xff, 0x00), cvf::Color3ub(0xff, 0x00, 0x00) }) },
        { RimRegularLegendConfig::BLUE_MAGENTA,     cvf::Color3ubArray({ cvf::Color3ub(0x00, 0x00, 0xff), cvf::Color3ub(0xff, 0x00, 0xff) }) },
        { RimRegularLegendConfig::RED_LIGHT_DARK,   cvf::Color3ubArray({ cvf::Color3ub(0xff, 0xcc, 0xcc), cvf::Color3ub(0x99, 0x00, 0x00) }) },
        { RimRegularLegendConfig::GREEN_LIGHT_DARK, cvf::Color3ubArray({ cvf::Color3ub(0xcc, 0xff, 0xcc), cvf::Color3ub(0x00, 0x99, 0x00) }) },
        { RimRegularLegendConfig::BLUE_LIGHT_DARK,  cvf::Color3ubArray({ cvf::Color3ub(0xcc, 0xcc, 0xff), cvf::Color3ub(0x00, 0x00, 0x99) }) }
    });

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimRegularLegendConfig::ColorRangesType RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE = RimRegularLegendConfig::GREEN_RED;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig::ColorRangesType RimEnsembleCurveSetColorManager::nextColorRange(RimEnsembleCurveSet* curveSet)
{
    CVF_ASSERT(curveSet);

    RimEnsembleCurveSetCollection* coll;
    curveSet->firstAncestorOrThisOfType(coll);

    if (coll)
    {
        if (m_colorCache.find(coll) != m_colorCache.end())
        {
            if (m_colorCache[coll].find(curveSet) != m_colorCache[coll].end())
            {
                // CurveSet found in cache, use same color range as last time
                return m_colorCache[coll][curveSet];
            }
        }
        else
        {
            m_colorCache.insert(std::make_pair(coll, std::map<RimEnsembleCurveSet*, RimRegularLegendConfig::ColorRangesType>()));
            m_nextColorIndexes.insert(std::make_pair(coll, 0));
        }

        int currColorIndex = m_nextColorIndexes[coll];
        RimRegularLegendConfig::ColorRangesType resultColorRange = colorRangeByIndex(currColorIndex);
        m_nextColorIndexes[coll] = (currColorIndex < (int)ENSEMBLE_COLOR_RANGES.size() - 1) ? currColorIndex + 1 : 0;
        m_colorCache[coll][curveSet] = resultColorRange;
        return resultColorRange;
    }
    return DEFAULT_ENSEMBLE_COLOR_RANGE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig::ColorRangesType RimEnsembleCurveSetColorManager::colorRangeByIndex(int index)
{
    int i = 0;
    for (auto item : ENSEMBLE_COLOR_RANGES)
    {
        if (i++ == index) return item.first;
    }
    return DEFAULT_ENSEMBLE_COLOR_RANGE;
}

std::map<RimEnsembleCurveSetCollection*, int> RimEnsembleCurveSetColorManager::m_nextColorIndexes;

std::map<RimEnsembleCurveSetCollection*, std::map<RimEnsembleCurveSet*, RimRegularLegendConfig::ColorRangesType>> RimEnsembleCurveSetColorManager::m_colorCache;
