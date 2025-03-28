/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include <vector>

class RigCaseCellResultsData;
class RigFloodingSettings;

class RigHydrocarbonFlowTools
{
public:
    enum class ResultType
    {
        NONE,
        MOBILE_OIL,
        MOBILE_GAS,
        MOBILE_HYDROCARBON
    };

    static std::vector<double> residualOilData( RigCaseCellResultsData&             resultData,
                                                RigHydrocarbonFlowTools::ResultType resultType,
                                                const RigFloodingSettings&          floodingSettings,
                                                std::size_t                         nSamples );

    static std::vector<double> residualGasData( RigCaseCellResultsData&             resultData,
                                                RigHydrocarbonFlowTools::ResultType resultType,
                                                const RigFloodingSettings&          floodingSettings,
                                                std::size_t                         nSamples );
};
