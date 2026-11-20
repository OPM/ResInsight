/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Equinor ASA
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

#include "RifEclipseSummaryAddress.h"

#include <string>
#include <vector>

//==================================================================================================
//
// Creates summary addresses from text on the form <VECTOR>[:<PARAM1>[:<PARAM2>]...]
//
//==================================================================================================
namespace RifEclipseSummaryAddressParser
{
// Eclipse text address. Returns an invalid address if the category is not recognized
RifEclipseSummaryAddress fromEclipseTextAddress( const std::string& textAddress );

// Free text header. Format: [<ER|ERR|ERROR>:]<VECTOR>:<CATEGORY_PARAM_NAME1>[:<CATEGORY_PARAM_NAME2>][....]
// Text not recognized as an Eclipse address is returned as an imported address
RifEclipseSummaryAddress fromEclipseTextAddressParseErrorTokens( const std::string& textAddress );
} // namespace RifEclipseSummaryAddressParser
