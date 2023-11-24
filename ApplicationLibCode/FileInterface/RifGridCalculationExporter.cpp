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

#include "RifGridCalculationExporter.h"

#include <iostream>
#include <toml++/toml.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RifGridCalculationExporter::writeToFile( const std::vector<RifGridCalculation>& calculations,
                                                                      const std::string&                     filePath )
{
    printf( "FILE PATH: %s\n", filePath.c_str() );
    std::ofstream stream( filePath );

    return writeToStream( calculations, stream );
}

std::pair<bool, std::string> RifGridCalculationExporter::writeToStream( const std::vector<RifGridCalculation>& calculations,
                                                                        std::ostream&                          stream )
{
    auto calculationsVector = toml::array();

    for ( auto calculation : calculations )
    {
        auto variablesVector = toml::array{};
        for ( auto variable : calculation.variables )
        {
            variablesVector.push_back( toml::table{
                { "name", variable.name },
                { "variable", variable.resultVariable },
                { "type", variable.resultType },
            } );
        }

        auto m = toml::table{
            { "description", calculation.description },
            { "expression", calculation.expression },
            { "unit", calculation.unit },
            { "variables", variablesVector },
        };
        calculationsVector.push_back( m );
    }

    auto tbl = toml::table{
        { "grid-calculation", calculationsVector },
    };

    stream << tbl;

    return { stream.good(), "" };
}
