/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-  Statoil ASA
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

#include "RifEclipseUserDataParserTools.h"

#include <vector>
#include <string>

//==================================================================================================
/// 
//==================================================================================================
class RifEclipseUserDataKeywordTools
{
public:
    static std::vector<size_t>                          requiredItemsPerLineForKeyword(const std::string& identifier);
    
    static std::vector<std::vector<std::string>>        buildColumnHeaderText(const std::vector<std::string>& quantityNames,
                                                                              const std::vector<std::vector<std::string>>& restOfHeaderRows,
                                                                              std::vector<std::string>* errorText = nullptr);

    static bool                                         isDate(const std::string& identifier);

    static  RifEclipseSummaryAddress                    makeAndFillAddress(const std::string quantityName, const std::vector<std::string>& columnHeaderText);

    static bool                                         isStepType(const std::string& identifier);
};

