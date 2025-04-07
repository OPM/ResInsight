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

#include "RigHydrocarbonFlowTools.h"

#include "RiaDefines.h"
#include "RiaResultNames.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigFloodingSettings.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigHydrocarbonFlowTools::residualOilData( RigCaseCellResultsData&             resultData,
                                                              RigHydrocarbonFlowTools::ResultType resultType,
                                                              const RigFloodingSettings&          floodingSettings,
                                                              std::size_t                         nSamples )
{
    std::vector<double> residualOil;

    if ( ( resultType == RigHydrocarbonFlowTools::ResultType::MOBILE_OIL ) ||
         ( resultType == RigHydrocarbonFlowTools::ResultType::MOBILE_HYDROCARBON ) )
    {
        if ( floodingSettings.oilFlooding() == RigFloodingSettings::FloodingType::GAS_FLOODING )
        {
            residualOil =
                resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::sogcr() ), 0 );
        }
        else if ( floodingSettings.oilFlooding() == RigFloodingSettings::FloodingType::WATER_FLOODING )
        {
            residualOil =
                resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::sowcr() ), 0 );
        }
        else
        {
            residualOil.resize( nSamples, floodingSettings.oilUserDefFlooding() );
        }
    }

    if ( residualOil.empty() ) residualOil.resize( nSamples, 0.0 );

    return residualOil;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigHydrocarbonFlowTools::residualGasData( RigCaseCellResultsData&             resultData,
                                                              RigHydrocarbonFlowTools::ResultType resultType,
                                                              const RigFloodingSettings&          floodingSettings,
                                                              std::size_t                         nSamples )
{
    std::vector<double> residualGas;
    if ( ( resultType == RigHydrocarbonFlowTools::ResultType::MOBILE_GAS ) ||
         ( resultType == RigHydrocarbonFlowTools::ResultType::MOBILE_HYDROCARBON ) )
    {
        if ( floodingSettings.gasFlooding() == RigFloodingSettings::FloodingType::GAS_FLOODING )
        {
            residualGas =
                resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::sgcr() ), 0 );
        }
        else if ( floodingSettings.gasFlooding() == RigFloodingSettings::FloodingType::USER_DEFINED )
        {
            residualGas.resize( nSamples, floodingSettings.gasUserDefFlooding() );
        }
    }
    if ( residualGas.empty() ) residualGas.resize( nSamples, 0.0 );

    return residualGas;
}
