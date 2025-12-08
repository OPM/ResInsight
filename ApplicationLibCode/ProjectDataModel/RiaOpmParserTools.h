/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include <map>
#include <set>
#include <string>
#include <vector>

namespace Opm
{
class UnitSystem;
}

class RifVfpInjTable;
class RifVfpProdTable;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace RiaOpmParserTools
{

std::tuple<Opm::UnitSystem, std::vector<RifVfpProdTable>, std::vector<RifVfpInjTable>>
    extractVfpTablesFromDataFile( const std::string& dataDeckFilename );

std::map<std::string, std::vector<std::pair<int, int>>> extractWseglink( const std::string& filename );

using AicdTemplateValues = std::map<std::string, double>;
std::vector<AicdTemplateValues> extractWsegAicd( const std::string& filename );
std::vector<AicdTemplateValues> extractWsegAicdCompletor( const std::string& filename );

std::string aicdTemplateId();

// Phase conversion utilities
std::set<RiaDefines::PhaseType> phasesFromInteheadValue( int phaseIndicator );

}; // namespace RiaOpmParserTools
