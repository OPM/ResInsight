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
#include "RimSummaryCaseCollection.h"

#include "cafPdmPointer.h"

#include <map>

class RimEnsembleCurveSet;
class RimEnsembleCurveSetCollection;
class RimCustomObjectiveFunction;
class RimObjectiveFunction;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimEnsembleCurveSetColorManager
{
public:
    enum class ColorMode
    {
        SINGLE_COLOR,
        BY_ENSEMBLE_PARAM,
        BY_OBJECTIVE_FUNCTION,
        BY_CUSTOM_OBJECTIVE_FUNCTION
    };
    using ColorModeEnum = caf::AppEnum<ColorMode>;

public:
    static const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray>& EnsembleColorRanges();

    static const RimRegularLegendConfig::ColorRangesType DEFAULT_ENSEMBLE_COLOR_RANGE;
    static RimRegularLegendConfig::ColorRangesType       cycledEnsembleColorRange( int index );

    static bool isEnsembleColorRange( RimRegularLegendConfig::ColorRangesType colorRange )
    {
        return m_ensembleColorRanges.find( colorRange ) != m_ensembleColorRanges.end();
    }

    static void initializeLegendConfig( RimRegularLegendConfig* legendConfig, const EnsembleParameter& parameter );
    static void initializeLegendConfig( RimRegularLegendConfig*               legendConfig,
                                        std::shared_ptr<RimObjectiveFunction> objectiveFunction,
                                        std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses );

    static void initializeLegendConfig( RimRegularLegendConfig*                     legendConfig,
                                        caf::PdmPointer<RimCustomObjectiveFunction> customObjectiveFunction );

    static cvf::Color3f caseColor( const RimRegularLegendConfig* legendConfig,
                                   const RimSummaryCase*         summaryCase,
                                   const EnsembleParameter&      parameter );

    static cvf::Color3f caseColor( const RimRegularLegendConfig*         legendConfig,
                                   RimSummaryCase*                       summaryCase,
                                   std::shared_ptr<RimObjectiveFunction> objectiveFunction,
                                   std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses );

    static cvf::Color3f caseColor( const RimRegularLegendConfig*               legendConfig,
                                   RimSummaryCase*                             summaryCase,
                                   caf::PdmPointer<RimCustomObjectiveFunction> customObjectiveFunction );

private:
    static const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray> m_ensembleColorRanges;

    static std::map<RimEnsembleCurveSetCollection*, int> m_nextColorIndexes;
    static std::map<RimEnsembleCurveSetCollection*, std::map<RimEnsembleCurveSet*, RimRegularLegendConfig::ColorRangesType>> m_colorCache;
};
