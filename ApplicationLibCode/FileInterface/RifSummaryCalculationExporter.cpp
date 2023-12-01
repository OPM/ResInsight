/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-    Equinor ASA
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

#include "RifSummaryCalculationExporter.h"

#include <fstream>

#include <tomlplusplus/toml.hpp>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifSummaryCalculationExporter::writeToFile( const std::vector<RifSummaryCalculation>& calculations,
                                                                         const std::string&                        filePath )
{
    std::ofstream stream( filePath );
    if ( !stream.good() ) return { false, "Unable to open file: " + filePath };

    return writeToStream( calculations, stream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifSummaryCalculationExporter::writeToStream( const std::vector<RifSummaryCalculation>& calculations,
                                                                           std::ostream&                             stream )
{
    auto calculationsVector = toml::array();

    for ( auto calculation : calculations )
    {
        auto variablesVector = toml::array{};
        for ( auto variable : calculation.variables )
        {
            variablesVector.push_back( toml::table{
                { "name", variable.name },
                { "address", variable.address },
            } );
        }

        calculationsVector.push_back( toml::table{
            { "expression", calculation.expression },
            { "unit", calculation.unit },
            { "distribute-to-other-items", calculation.distributeToOther },
            { "distribute-to-all-cases", calculation.distributeToAllCases },
            { "variables", variablesVector },
        } );
    }

    auto tbl = toml::table{
        { "summary-calculation", calculationsVector },
    };

    stream << tbl;

    return { stream.good(), "" };
}
