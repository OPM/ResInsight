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
#include <utility>

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
    try
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
                    calculation.description = calc->at_path( "description" ).value_or<std::string>( "" );
                    if ( calculation.description.empty() ) throw std::runtime_error( "Missing description." );

                    calculation.expression = calc->at_path( "expression" ).value_or<std::string>( "" );
                    if ( calculation.expression.empty() ) throw std::runtime_error( "Missing expression." );

                    calculation.unit = calc->at_path( "unit" ).value_or<std::string>( "" );

                    if ( toml::array* vars = calc->at_path( "variables" ).as_array() )
                    {
                        std::vector<RifGridCalculationVariable> variables;
                        for ( auto&& v : *vars )
                        {
                            if ( toml::table* var = v.as_table() )
                            {
                                RifGridCalculationVariable variable;
                                variable.name           = var->at_path( "name" ).value_or<std::string>( "" );
                                variable.resultType     = var->at_path( "type" ).value_or<std::string>( "" );
                                variable.resultVariable = var->at_path( "variable" ).value_or<std::string>( "" );
                                if ( variable.name.empty() || variable.resultType.empty() || variable.resultVariable.empty() )
                                    throw std::runtime_error( "Incomplete variable: Missing either name, result type or result variable." );
                                variables.push_back( variable );
                            }
                        }
                        calculation.variables = variables;
                    }

                    calculations.push_back( calculation );
                }
            }
        }

        if ( calculations.empty() )
        {
            return { calculations, "No calculations imported." };
        }

        return { calculations, "" };
    }
    catch ( const std::runtime_error& error )
    {
        return { {}, error.what() };
    }
}
