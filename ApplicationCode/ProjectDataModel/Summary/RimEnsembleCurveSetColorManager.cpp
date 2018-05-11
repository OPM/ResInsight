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
const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray> RimEnsembleCurveSetColorManager::m_ensembleColorRanges(
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
const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray>& RimEnsembleCurveSetColorManager::EnsembleColorRanges()
{
    return m_ensembleColorRanges;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimRegularLegendConfig::ColorRangesType RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE = RimRegularLegendConfig::GREEN_RED;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig::ColorRangesType RimEnsembleCurveSetColorManager::cycledEnsembleColorRange(int index)
{
    size_t modIndex = index % m_ensembleColorRanges.size();

    auto crIt = m_ensembleColorRanges.begin();
    for (int i = 0; i < modIndex; ++i) ++crIt;

    return crIt->first;
}

std::map<RimEnsembleCurveSetCollection*, int> RimEnsembleCurveSetColorManager::m_nextColorIndexes;

std::map<RimEnsembleCurveSetCollection*, std::map<RimEnsembleCurveSet*, RimRegularLegendConfig::ColorRangesType>> RimEnsembleCurveSetColorManager::m_colorCache;
