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

#include "RifGridCalculationImporter.h"

#include <fstream>

#include <tomlplusplus/toml.hpp>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RifGridCalculation>, std::string> RifGridCalculationImporter::readFromFile( const std::string& filePath )
{
    std::ifstream stream( filePath );
    if ( !stream.good() ) return { {}, "Unable to open file: " + filePath };

    return readFromStream( stream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RifGridCalculation>, std::string> RifGridCalculationImporter::readFromStream( std::istream& stream )
{
    toml::table tbl = toml::parse( stream );

    auto calculationsVector = tbl["grid-calculation"];

    std::vector<RifGridCalculation> calculations;

    if ( toml::array* arr = calculationsVector.as_array() )
    {
        for ( auto&& a : *arr )
        {
            RifGridCalculation calculation;
            if ( toml::table* calc = a.as_table() )
            {
                calculation.description = calc->at_path( "description" ).as_string()->value_or( "" );
                calculation.expression  = calc->at_path( "expression" ).as_string()->value_or( "" );
                calculation.unit        = calc->at_path( "unit" ).as_string()->value_or( "" );

                if ( toml::array* vars = calc->at_path( "variables" ).as_array() )
                {
                    std::vector<RifGridCalculationVariable> variables;
                    for ( auto&& v : *vars )
                    {
                        if ( toml::table* var = v.as_table() )
                        {
                            RifGridCalculationVariable variable;
                            variable.name           = var->at_path( "name" ).as_string()->value_or( "" );
                            variable.resultType     = var->at_path( "type" ).as_string()->value_or( "" );
                            variable.resultVariable = var->at_path( "variable" ).as_string()->value_or( "" );
                            variables.push_back( variable );
                        }
                    }
                    calculation.variables = variables;
                }

                calculations.push_back( calculation );
            }
        }
    }

    return { calculations, "" };
}
