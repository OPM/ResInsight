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

#include "RifEclipseSummaryAddressDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RifEclipseSummaryAddressDefines::StatisticsType>::setUp()
{
    addItem( RifEclipseSummaryAddressDefines::StatisticsType::NONE, "NONE", "None" );
    addItem( RifEclipseSummaryAddressDefines::StatisticsType::P10, "P10", "P10" );
    addItem( RifEclipseSummaryAddressDefines::StatisticsType::P50, "P50", "P50" );
    addItem( RifEclipseSummaryAddressDefines::StatisticsType::P90, "P90", "P90" );
    addItem( RifEclipseSummaryAddressDefines::StatisticsType::MEAN, "MEAN", "Mean" );
    setDefault( RifEclipseSummaryAddressDefines::StatisticsType::NONE );
}

template <>
void caf::AppEnum<RifEclipseSummaryAddressDefines::CurveType>::setUp()
{
    addItem( RifEclipseSummaryAddressDefines::CurveType::RATE, "RATE", "Rate" );
    addItem( RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED, "ACCUMULATED", "Accumulated" );
    setDefault( RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddressDefines::statisticsTypeToString( StatisticsType type )
{
    caf::AppEnum<StatisticsType> enumType( type );
    return enumType.uiText().toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddressDefines::differenceIdentifier()
{
    return "_DIFF";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddressDefines::historyIdentifier()
{
    return "H";
}
