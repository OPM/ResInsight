/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include <string>

//==================================================================================================
//
//
//==================================================================================================
namespace RifEclipseSummaryAddressDefines
{
// Based on list in ecl_smspec.c and list of types taken from Eclipse Reference Manual ecl_rm_2011.1.pdf
enum class SummaryCategory
{
    SUMMARY_INVALID,
    SUMMARY_FIELD,
    SUMMARY_AQUIFER,
    SUMMARY_NETWORK,
    SUMMARY_MISC,
    SUMMARY_REGION,
    SUMMARY_REGION_2_REGION,
    SUMMARY_GROUP,
    SUMMARY_WELL,
    SUMMARY_WELL_COMPLETION,
    SUMMARY_WELL_CONNECTION,
    SUMMARY_WELL_LGR,
    SUMMARY_WELL_CONNECTION_LGR,
    SUMMARY_WELL_SEGMENT,
    SUMMARY_BLOCK,
    SUMMARY_BLOCK_LGR,
    SUMMARY_IMPORTED,
    SUMMARY_TIME // Used to represent time as along the x-axis
};

enum class SummaryIdentifierType
{
    INPUT_REGION_NUMBER,
    INPUT_REGION_2_REGION,
    INPUT_WELL_NAME,
    INPUT_WELL_COMPLETION_NUMBER,
    INPUT_GROUP_NAME,
    INPUT_NETWORK_NAME,
    INPUT_CELL_IJK,
    INPUT_LGR_NAME,
    INPUT_SEGMENT_NUMBER,
    INPUT_AQUIFER_NUMBER,
    INPUT_VECTOR_NAME,
    INPUT_ID
};

enum class StatisticsType
{
    NONE,
    P10,
    P50,
    P90,
    MEAN
};

enum class CurveType
{
    ACCUMULATED,
    RATE
};

std::string statisticsTypeToString( StatisticsType type );

std::string differenceIdentifier();
std::string historyIdentifier();

}; // namespace RifEclipseSummaryAddressDefines
