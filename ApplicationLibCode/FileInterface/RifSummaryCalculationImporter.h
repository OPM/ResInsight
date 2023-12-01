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

#pragma once

#include "RifSummaryCalculation.h"

#include <string>
#include <vector>

//==================================================================================================
//
//==================================================================================================
class RifSummaryCalculationImporter
{
public:
    static std::pair<std::vector<RifSummaryCalculation>, std::string> readFromFile( const std::string& filePath );
    static std::pair<std::vector<RifSummaryCalculation>, std::string> readFromStream( std::istream& stream );
};
